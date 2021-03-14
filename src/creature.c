
BodyPart *
CreatureAddBodyPart(World *world, 
        Creature *creature, 
        Vec2 pos, 
        Vec2 size)
{
    BodyPart *part = creature->bodyParts + creature->nBodyParts++;
    RigidBody *body = AddDynamicRectangle(world, pos, size.x, size.y, 0, creature->physicsGroup);

    part->body = body;
    part->color = vec4(RandomR32(0, 1), RandomR32(0,1), RandomR32(0,1),  0.5);

    return part;
}

RotaryMuscle *
CreatureAddRotaryMuscle(World *world, 
        Creature *creature, 
        BodyPart *bodyA, 
        BodyPart *bodyB, 
        Vec2 pivotPoint,
        r32 minAngle, 
        r32 maxAngle)
{
    RotaryMuscle *muscle = creature->rotaryMuscles+creature->nRotaryMuscles++;
    *muscle = (RotaryMuscle){};

    //RotaryLimitJoint(world, bodyA->body, bodyB->body, pivotPoint, minAngle, maxAngle);
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
AddCreature(World *world, Vec2 pos)
{
    Creature *creature = world->creatures+world->nCreatures++;
    *creature = (Creature){};

    creature->maxBodyParts = world->maxBodypartsPerCreature;
    creature->nBodyParts = 0;
    creature->bodyParts = AllocateElement(world->bodyPartPool);

    creature->maxRotaryMuscles = world->maxRotaryMusclesPerCreature;
    creature->nRotaryMuscles = 0;
    creature->rotaryMuscles = AllocateElement(world->bodyPartPool);

    creature->physicsGroup = world->physicsGroupCounter++;

    r32 limbWidth = 10;
    r32 limbHeight = 40;
    r32 angle = 1.0;

    // Build body
    ui32 chains = 12;
    BodyPart *last = NULL;
    for(int i = 0; i < chains; i++)
    {
        BodyPart *part = CreatureAddBodyPart(world, creature, vec2(pos.x, pos.y+i*limbHeight), vec2(limbWidth, limbHeight));
        if(last)
        {
            Vec2 pivotPoint = vec2(pos.x, pos.y+limbHeight/2.0+(i-1)*limbHeight);
            CreatureAddRotaryMuscle(world, creature, last, part, pivotPoint, -angle, angle);
        }
        last = part;
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
UpdateCreature(World *world, Creature *creature)
{
    creature->internalClock+=1.0/60.0;
    r32 drag = 0.1;

    int nRotaryMuscles = creature->nRotaryMuscles;
    int nHalfPhases = 4;
    r32 secondsPerStrokeCycle = 2;
    r32 secondsPerDragCycle = 2;
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
}

