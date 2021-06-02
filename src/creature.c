
BodyPart *
CreatureAddBodyPart(FakeWorld *world, 
        Creature *creature, 
        Vec2 pos, 
        Vec2 size,
        r32 angle)
{
    BodyPart *part = world->bodyParts + world->nBodyParts++;
    creature->nBodyParts++;
    RigidBody *body = AddDynamicRectangle(world, pos, size.x, size.y, angle, creature->physicsGroup);

    part->body = body;
    part->color = vec4(RandomR32(0, 1), RandomR32(0,1), RandomR32(0,1),  0.5);

    return part;
}

internal inline b32
BodyPartPoint2Intersect(BodyPart *part, Vec2 point)
{
    OrientedBox bodyBox = GetRigidBodyBox(part->body);
    return OrientedBoxPoint2Intersect(bodyBox.pos, bodyBox.dims, bodyBox.angle, point);
}

BodyPart *
GetCreatureBodyPartAt(Creature *creature, Vec2 point)
{
    BodyPart *intersect = NULL;
    for(ui32 bodyPartIdx = 0;
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
        r32 minAngle, 
        r32 maxAngle)
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
DestroyRotaryMuscle(RotaryMuscle *muscle)
{
    cpConstraintFree(muscle->motor);
    cpConstraintFree(muscle->rotaryLimitConstraint);
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
            v2_add(pos, partDef->pos),
            vec2(partDef->width, partDef->height),
            partDef->angle);
    part->def = partDef;
    ui32 subDefs[def->nBodyParts];
    ui32 nConnections = GetSubNodeBodyPartsById(def, partDef, subDefs);
    for(ui32 connectionIdx = 0;
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
        r32 angleA = parentPartDef->angle;
        //r32 angleB = partDef->angle;
        r32 angleOffset = -angleA;
        r32 edgeAngle = GetAbsoluteEdgeAngle(parentPartDef, partDef->xEdge, partDef->yEdge);
        RotaryMuscle *muscle = CreatureAddRotaryMuscle(world, creature, a, b, 
                v2_add(pos, partDef->pivotPoint), 
                edgeAngle+partDef->minAngle+angleOffset,
                edgeAngle+partDef->maxAngle+angleOffset);
        part->rotaryMuscle = muscle;
    }
}

// No sine
r32
GetInternalClockValue(Creature *creature, ui32 clockIdx)
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

    creature->solidColor = vec3(def->solidColor.x, def->solidColor.y, def->solidColor.z);
    creature->drawSolidColor = def->drawSolidColor;

    creature->nInternalClocks = def->nInternalClocks;
    creature->internalClock = 0;
    for(ui32 clockIdx = 0;
            clockIdx < creature->nInternalClocks;
            clockIdx++)
    {
        creature->phases[clockIdx] = 0;
        creature->frequencies[clockIdx] = 1.0/(clockIdx+1);
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
                v2_add(pos, partDef->pos),
                vec2(partDef->width, partDef->height),
                partDef->angle);

        RotaryMuscleDefinition *muscleDef = def->rotaryMuscles+muscleIdx;
        BodyPart *a = creature->bodyParts+muscleDef->bodyPartIdx0;
        BodyPart *b = creature->bodyParts+muscleDef->bodyPartIdx1;
        r32 angleA = cpBodyGetAngle(a->body->body);
        r32 angleB = cpBodyGetAngle(b->body->body);
        CreatureAddRotaryMuscle(world, creature, a, b, 
                v2_add(pos, muscleDef->pivotPoint), 
                muscleDef->minAngle+angleB-angleA, 
                muscleDef->maxAngle+angleB-angleA);
    }
#endif

    return creature;
}

void
DestroyCreature(Creature *creature)
{
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        DestroyRigidBody(creature->bodyParts[bodyPartIdx].body);
    }
    for(ui32 rotaryMuscleIdx = 0;
            rotaryMuscleIdx < creature->nRotaryMuscles;
            rotaryMuscleIdx++)
    {
        DestroyRotaryMuscle(creature->rotaryMuscles + rotaryMuscleIdx);
    }
}

Vec2
GetBodyPartCenter(BodyPart *part)
{
    cpVect bodyPos= cpBodyGetPosition(part->body->body);
    return vec2(bodyPos.x, bodyPos.y);
}

void
SetMuscleActivation(RotaryMuscle *muscle, r32 activation)
{
    r32 absActivation = fabsf(activation);
    cpSimpleMotorSetRate(muscle->motor, activation < 0.0 ? -5 : 5);
    r32 maxActivation = 800000.0;
    cpConstraintSetMaxForce(muscle->motor, absActivation*maxActivation);
}

r32
FitnessStrictlyMoveForward(Creature *creature)
{
    r32 maxX = -10000.0;
    r32 minY = 10000.0;
    for(ui32 bodyPartIdx = 0;
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

r32
FitnessStrictlyMoveRight(Creature *creature)
{
    r32 minX = 10000.0;
    r32 maxY = -10000.0;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        cpVect pos = cpBodyGetPosition(part->body->body);
        minX = Min(minX, pos.x);
        maxY = Max(maxY, fabs(pos.y));
    }
    return minX - maxY;
}

r32
CreatureGetFitness(Creature *creature)
{
    return FitnessStrictlyMoveRight(creature);
}

void
UpdateCreature(FakeWorld *world, Creature *creature)
{
    MinimalGatedUnit *brain = creature->brain;
    r32 drag = 0.2;

    creature->internalClock+=1.0/60.0;
    //r32 input0 = sinf(4*creature->internalClock);
    //r32 input1 = sinf(8*creature->internalClock);
    //r32 input0 = sinf(GetInternalClockValue(creature, 0));
    //r32 input1 = sinf(GetInternalClockValue(creature, 1));

    for(ui32 internalClockIdx = 0;
            internalClockIdx < creature->nInternalClocks;
            internalClockIdx++)
    {
        brain->x.v[internalClockIdx] = sinf(GetInternalClockValue(creature, internalClockIdx));
    }

    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        Vec2 pos = GetBodyPartCenter(part);
        if(part->def->hasAbsoluteXPositionInput)
        {
            r32 activation = pos.x/100;
            brain->x.v[part->def->absoluteXPositionInputIdx] = activation;
        }
        if(part->def->hasAbsoluteYPositionInput)
        {
            r32 activation = pos.y/100;
            brain->x.v[part->def->absoluteYPositionInputIdx] = activation;
        }
    }

    UpdateMinimalGatedUnit(brain);

    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        if(part->def->hasDragOutput)
        {
            r32 activation = brain->h.v[brain->stateSize-brain->outputSize+part->def->dragOutputIdx];

            part->body->drag = 0.03+activation*drag + drag;
        }
        if(part->def->hasRotaryMuscleOutput)
        {
            r32 activation = brain->h.v[brain->stateSize-brain->outputSize+part->def->rotaryMuscleOutputIdx];

            RotaryMuscle *muscle = part->rotaryMuscle;
            Assert(muscle);
            SetMuscleActivation(muscle, activation);
        }
    }
}

