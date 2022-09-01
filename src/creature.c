
BodyPart *
CreatureAddBodyPart(FakeWorld *world, 
        Creature *creature, 
        Vec2 pos, 
        Vec2 size,
        R32 angle)
{
    BodyPart *part = world->bodyParts + world->nBodyParts++;
    creature->nBodyParts++;
    RigidBody *body = AddDynamicRectangle(world, pos, size.x, size.y, angle, creature->physicsGroup);

    part->body = body;
    part->color = V4(RandomR32(0, 1), RandomR32(0,1), RandomR32(0,1),  0.5);

    return part;
}

internal inline B32
BodyPartPoint2Intersect(BodyPart *part, Vec2 point)
{
    OrientedBox bodyBox = GetRigidBodyBox(part->body);
    return OrientedBoxPoint2Intersect(bodyBox.pos, bodyBox.dims, bodyBox.angle, point);
}

BodyPart *
GetCreatureBodyPartAt(Creature *creature, Vec2 point)
{
    BodyPart *intersect = NULL;
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        if(BodyPartPoint2Intersect(part, point))
        {
            intersect = part;
        }
    }
    return intersect;
}

internal inline Vec2
GetBodyPartPos(BodyPart *part)
{
    return GetBodyPos(part->body);
}

RotaryMuscle *
CreatureAddRotaryMuscle(FakeWorld *world, 
        Creature *creature, 
        BodyPart *bodyA, 
        BodyPart *bodyB, 
        Vec2 pivotPoint,
        R32 minAngle, 
        R32 maxAngle)
{
    RotaryMuscle *muscle = world->rotaryMuscles+world->nRotaryMuscles++;
    creature->nRotaryMuscles++;
    *muscle = (RotaryMuscle){};

    muscle->pivotConstraint = cpPivotJointNew(bodyA->body->body, 
            bodyB->body->body, 
            cpv(pivotPoint.x, pivotPoint.y));
    cpSpaceAddConstraint(world->space, muscle->pivotConstraint);

    muscle->rotaryLimitConstraint = cpRotaryLimitJointNew(bodyA->body->body, 
            bodyB->body->body, 
            minAngle, 
            maxAngle);
    cpSpaceAddConstraint(world->space, muscle->rotaryLimitConstraint);

    muscle->bodyA = bodyA;
    muscle->bodyB = bodyB;
    muscle->minAngle = minAngle;
    muscle->maxAngle = maxAngle;
    muscle->motor = cpSimpleMotorNew(bodyA->body->body, bodyB->body->body, 5);
    cpSpaceAddConstraint(world->space, muscle->motor);
    cpConstraintSetMaxForce(muscle->motor, 0);

    return muscle;
}

void
DestroyRotaryMuscle(FakeWorld *world, RotaryMuscle *muscle)
{
    cpSpaceRemoveConstraint(world->space, muscle->motor);
    cpConstraintDestroy(muscle->motor);
    cpConstraintFree(muscle->motor);

    cpSpaceRemoveConstraint(world->space, muscle->rotaryLimitConstraint);
    cpConstraintDestroy(muscle->rotaryLimitConstraint);
    cpConstraintFree(muscle->rotaryLimitConstraint);

    cpSpaceRemoveConstraint(world->space, muscle->pivotConstraint);
    cpConstraintDestroy(muscle->pivotConstraint);
    cpConstraintFree(muscle->pivotConstraint);

}

void
BuildCreature(FakeWorld *world, 
        Creature *creature, 
        Vec2 pos,
        CreatureDefinition *def, 
        BodyPart *parentPart,
        BodyPartDefinition *parentPartDef,
        BodyPartDefinition *partDef)
{
    BodyPart *part = CreatureAddBodyPart(world, 
            creature, 
            V2Add(pos, partDef->pos),
            V2(partDef->width, partDef->height),
            partDef->angle);
    part->def = partDef;
    U32 subDefs[def->nBodyParts];
    U32 nConnections = GetSubNodeBodyPartsById(def, partDef, subDefs);
    for(U32 connectionIdx = 0;
            connectionIdx < nConnections;
            connectionIdx++)
    {
        BodyPartDefinition *subPartDef = GetBodyPartById(def, subDefs[connectionIdx]);
        BuildCreature(world, creature, pos, def, part, partDef, subPartDef);
    }
    // Connect to parent
    if(partDef->connectionId)
    {
        BodyPart *a = parentPart;
        BodyPart *b = part;
        R32 angleA = parentPartDef->angle;
        //R32 angleB = partDef->angle;
        R32 angleOffset = -angleA;
        R32 edgeAngle = GetAbsoluteEdgeAngle(parentPartDef, partDef->xEdge, partDef->yEdge);
        RotaryMuscle *muscle = CreatureAddRotaryMuscle(world, creature, a, b, 
                V2Add(pos, partDef->pivotPoint), 
                edgeAngle+partDef->minAngle+angleOffset,
                edgeAngle+partDef->maxAngle+angleOffset);
        part->rotaryMuscle = muscle;
    }
}

// No sine
R32
GetInternalClockValue(Creature *creature, U32 clockIdx)
{
    return creature->phases[clockIdx] + 
        creature->frequencies[clockIdx]*creature->internalClock*M_PI*2;
}

Creature *
AddCreature(FakeWorld *world, Vec2 pos, CreatureDefinition *def, MinimalGatedUnit *brain)
{
    Creature *creature = world->creatures+world->nCreatures++;
    *creature = (Creature){};

    creature->brain = brain;

    creature->nBodyParts = 0;
    creature->bodyParts = world->bodyParts+world->nBodyParts;

    creature->nRotaryMuscles = 0;
    creature->rotaryMuscles = world->rotaryMuscles+world->nRotaryMuscles;

    creature->solidColor = V3(def->solidColor.x, def->solidColor.y, def->solidColor.z);
    creature->drawSolidColor = def->drawSolidColor;

    creature->nInternalClocks = def->nInternalClocks;
    creature->internalClock = 0;
    for(U32 clockIdx = 0;
            clockIdx < creature->nInternalClocks;
            clockIdx++)
    {
        creature->phases[clockIdx] = 0;
        creature->frequencies[clockIdx] = 0.5f/(clockIdx+1);
    }

    //creature->physicsGroup = world->physicsGroupCounter++;
    creature->physicsGroup = 1;

    BuildCreature(world, creature, pos, def, NULL, NULL, def->bodyParts);

#if 0
    // Build body recursively.
    
    for(int bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts; 
            bodyPartIdx++)
    {
        BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
        CreatureAddBodyPart(world, 
                creature, 
                V2Add(pos, partDef->pos),
                V2(partDef->width, partDef->height),
                partDef->angle);

        RotaryMuscleDefinition *muscleDef = def->rotaryMuscles+muscleIdx;
        BodyPart *a = creature->bodyParts+muscleDef->bodyPartIdx0;
        BodyPart *b = creature->bodyParts+muscleDef->bodyPartIdx1;
        R32 angleA = cpBodyGetAngle(a->body->body);
        R32 angleB = cpBodyGetAngle(b->body->body);
        CreatureAddRotaryMuscle(world, creature, a, b, 
                V2Add(pos, muscleDef->pivotPoint), 
                muscleDef->minAngle+angleB-angleA, 
                muscleDef->maxAngle+angleB-angleA);
    }
#endif

    return creature;
}

void
DestroyCreature(FakeWorld *world, Creature *creature)
{
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        DestroyRigidBody(world, creature->bodyParts[bodyPartIdx].body);
    }
    for(U32 rotaryMuscleIdx = 0;
            rotaryMuscleIdx < creature->nRotaryMuscles;
            rotaryMuscleIdx++)
    {
        DestroyRotaryMuscle(world, creature->rotaryMuscles + rotaryMuscleIdx);
    }
}

Vec2
GetBodyPartCenter(BodyPart *part)
{
    cpVect bodyPos= cpBodyGetPosition(part->body->body);
    return V2(bodyPos.x, bodyPos.y);
}

void
SetMuscleActivation(RotaryMuscle *muscle, R32 activation)
{
#if 1
    // Muscle as force
    R32 absActivation = fabsf(activation);
    cpSimpleMotorSetRate(muscle->motor, activation < 0.0 ? -5 : 5);
    R32 maxActivation = 1200000.0;
    cpConstraintSetMaxForce(muscle->motor, absActivation*maxActivation);
#else
    // Muscle as target (relative) orientation to the joint limits

#endif
}

R32
FitnessStrictlyMoveForward(Creature *creature)
{
    R32 maxX = -10000.0;
    R32 minY = 10000.0;
    for(I32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        cpVect pos = cpBodyGetPosition(part->body->body);
        maxX = Max(maxX, fabsf(pos.x));
        minY = Min(minY, pos.y);
    }
    return minY - maxX;
}

void
UpdateCreature(FakeWorld *world, Creature *creature)
{
    MinimalGatedUnit *brain = creature->brain;
    R32 drag = 0.2;

    creature->internalClock += (1.0f/60.0f);

    for(U32 internalClockIdx = 0;
            internalClockIdx < creature->nInternalClocks;
            internalClockIdx++)
    {
        brain->x.v[internalClockIdx] = sinf(GetInternalClockValue(creature, internalClockIdx));
    }

    for(U32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        Vec2 pos = GetBodyPartCenter(part);
        R32 angle = GetBodyAngle(part->body);

        if(part->def->hasAbsoluteXPositionInput)
        {
            R32 activation = pos.x/100;
            brain->x.v[part->def->absoluteXPositionInputIdx] = activation;
        }
        if(part->def->hasAbsoluteYPositionInput)
        {
            R32 activation = pos.y/100;
            brain->x.v[part->def->absoluteYPositionInputIdx] = activation;
        }
        if(part->def->hasAngleTowardsTargetInput)
        {
            Vec2 diff = V2Sub(world->target, pos);
            R32 tAngle = atan2f(diff.y, diff.x);
            R32 activation = GetNormalizedAngDiff(angle, tAngle);
            brain->x.v[part->def->angleTowardsTargetInputIdx] = activation;
        }
        if(part->def->hasAbsoluteAngleInput)
        {
            R32 activation = angle;
            brain->x.v[part->def->absoluteAngleInputIdx] = activation;
        }
    }

    UpdateMinimalGatedUnit(brain);

    for(I32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        if(part->def->hasDragOutput)
        {
            R32 activation = brain->h.v[brain->stateSize-brain->outputSize+part->def->dragOutputIdx];

            part->body->drag = 0.03+activation*drag + drag;
        }
        if(part->def->hasRotaryMuscleOutput)
        {
            R32 activation = brain->h.v[brain->stateSize-brain->outputSize+part->def->rotaryMuscleOutputIdx];

            RotaryMuscle *muscle = part->rotaryMuscle;
            Assert(muscle);
            SetMuscleActivation(muscle, activation);
        }
    }
}

