
BodyPart *
CreatureAddBodyPart(FakeWorld *world, 
        Creature *creature, 
        Vec2 pos, 
        Vec2 size)
{
    BodyPart *part = world->bodyParts + world->nBodyParts++;
    creature->nBodyParts++;
    RigidBody *body = AddDynamicRectangle(world, pos, size.x, size.y, 0, creature->physicsGroup);

    part->body = body;
    part->color = vec4(RandomR32(0, 1), RandomR32(0,1), RandomR32(0,1),  0.5);

    return part;
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

    RotaryLimitJoint(world, bodyA->body, bodyB->body, pivotPoint, minAngle, maxAngle);

    muscle->bodyA = bodyA;
    muscle->bodyB = bodyB;
    muscle->minAngle = minAngle;
    muscle->maxAngle = maxAngle;
    muscle->motor = cpSimpleMotorNew(bodyA->body->body, bodyB->body->body, 5);
    cpSpaceAddConstraint(world->space, muscle->motor);
    cpConstraintSetMaxForce(muscle->motor, 0);

    return muscle;
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

    creature->physicsGroup = world->physicsGroupCounter++;

    // Build body
    for(int bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts; 
            bodyPartIdx++)
    {
        BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
        CreatureAddBodyPart(world, 
                creature, 
                v2_add(pos, partDef->pos),
                vec2(partDef->width, partDef->height));
    }

    for(int muscleIdx = 0;
            muscleIdx < def->nRotaryMuscles;
            muscleIdx++)
    {
        RotaryMuscleDefinition *muscleDef = def->rotaryMuscles+muscleIdx;
        BodyPart *a = creature->bodyParts+muscleDef->bodyPartIdx0;
        BodyPart *b = creature->bodyParts+muscleDef->bodyPartIdx1;
        CreatureAddRotaryMuscle(world, creature, a, b, 
                v2_add(pos, muscleDef->pivotPoint), muscleDef->minAngle, muscleDef->maxAngle);
    }

    return creature;
}

void
SetMuscleActivation(RotaryMuscle *muscle, r32 activation)
{
    r32 absActivation = fabs(activation);
    cpSimpleMotorSetRate(muscle->motor, activation < 0.0 ? -5 : 5);
    r32 maxActivation = 500000.0;
    cpConstraintSetMaxForce(muscle->motor, absActivation*maxActivation);
}

void
UpdateCreature(FakeWorld *world, Creature *creature)
{
    MinimalGatedUnit *brain = creature->brain;
    creature->internalClock+=1.0/60.0;
    r32 drag = 0.2;
    r32 input = sinf(creature->internalClock);
    brain->x.v[0] = input;
    UpdateMinimalGatedUnit(brain);

    for(ui32 muscleIdx = 0;
            muscleIdx < creature->nRotaryMuscles;
            muscleIdx++)
    {
        RotaryMuscle *muscle = creature->rotaryMuscles+muscleIdx;
        r32 activation = brain->h.v[brain->stateSize-brain->outputSize+muscleIdx];
        SetMuscleActivation(muscle, activation);
    }
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        r32 activation = brain->h.v[brain->stateSize-brain->outputSize+creature->nRotaryMuscles+bodyPartIdx];
        part->body->drag = 0.05+activation*drag + drag;
    }

#if 0
    int nRotaryMuscles = creature->nRotaryMuscles;
    int nHalfPhases = 2;
    r32 secondsPerStrokeCycle = 4;
    r32 secondsPerDragCycle = 4;
    r32 offsetPerMuscle = nHalfPhases*M_PI/nRotaryMuscles;
    r32 offsetPerBodyPart = nHalfPhases*M_PI/(nRotaryMuscles+1);

    for(ui32 muscleIdx = 0;
            muscleIdx < creature->nRotaryMuscles;
            muscleIdx++)
    {
        RotaryMuscle *muscle = creature->rotaryMuscles+muscleIdx;
        r32 activation = sinf(creature->internalClock/secondsPerStrokeCycle*M_PI*2+offsetPerMuscle*muscleIdx);
        SetMuscleActivation(muscle, activation);
    }
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        r32 activation = sinf(creature->internalClock/secondsPerDragCycle*M_PI*2+offsetPerBodyPart*bodyPartIdx);
        part->body->drag = 0.05+activation*drag + drag;
    }
#endif
}

