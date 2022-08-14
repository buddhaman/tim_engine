
enum {
    GROUND_MASK = 1<<0,
    DYNAMIC_MASK = 1<<1
};

RigidBody *
AddDynamicRectangle(FakeWorld *world, Vec2 pos, r32 width, r32 height, r32 angle, ui32 group)
{
    RigidBody *body = world->rigidBodies+world->nRigidBodies++;
    cpSpace *space = world->space;
    body->pos = pos;
    body->width = width;
    body->height = height;
    body->drag = 0.0;

    // Create cpBody
    cpFloat density = 0.001;
    cpFloat mass = width*height*density;
    cpFloat moment = cpMomentForBox(mass, width, height);
    body->body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    body->shape = cpSpaceAddShape(space, cpBoxShapeNew(body->body, width, height, 0.01));
    cpShapeSetFilter(body->shape, cpShapeFilterNew(group, DYNAMIC_MASK, DYNAMIC_MASK | GROUND_MASK));

    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);

    cpShapeSetFriction(body->shape, 0.8);
    cpShapeSetElasticity(body->shape, 0.0);

    return body;
}

internal inline RigidBody *
AddStaticRectangle(FakeWorld *world, Vec2 pos, r32 width, r32 height, r32 angle)
{
    RigidBody *body = world->rigidBodies+world->nRigidBodies++;
    cpSpace *space = world->space;
    body->pos = pos;
    body->width = width;
    body->height = height;

    // Create cpBody
    body->body = cpSpaceAddBody(space, cpBodyNewStatic());
    body->shape = cpBoxShapeNew(body->body, width, height, 0.01);
    cpSpaceAddShape(space, body->shape);
    cpShapeSetFilter(body->shape, cpShapeFilterNew(0, GROUND_MASK, DYNAMIC_MASK));
    
    cpShapeSetFriction(body->shape, 1.0);
    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);
    cpSpaceReindexShapesForBody(space, body->body);

    world->staticBodies[world->nStaticBodies++] = body;

    return body;
}

void
DestroyRigidBody(RigidBody *body)
{
    cpBodyFree(body->body);
    cpShapeFree(body->shape);
}

internal inline Vec2
GetBodyPos(RigidBody *body)
{
    cpVect pos = cpBodyGetPosition(body->body);
    return V2(pos.x, pos.y);
}

internal inline r32
GetBodyAngle(RigidBody *body)
{
    return (r32)cpBodyGetAngle(body->body);
}

internal inline OrientedBox
GetRigidBodyBox(RigidBody *body)
{
    cpVect pos = cpBodyGetPosition(body->body);
    r32 angle = GetBodyAngle(body);
    return (OrientedBox){V2(pos.x, pos.y), V2(body->width, body->height), angle};
}

internal r32
FitnessStrictlyMoveRight(FakeWorld *world, Creature *creature)
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

internal r32
FitnessDistanceTarget(FakeWorld *world, Creature *creature)
{
    // Position of main bodypart
    Vec2 creaturePos = GetBodyPos(creature->bodyParts->body);
    r32 dist = V2Dist(creaturePos, world->target);
    return -dist;
}

internal r32
FitnessWalkRight(FakeWorld *world, Creature *creature)
{
    BodyPart *part = creature->bodyParts;
    Vec2 pos = GetBodyPos(part->body);
    return pos.x;
}

internal r32
CreatureGetFitness(FakeWorld *world, Creature *creature)
{
    switch(world->trainingType)
    {

    case TRAIN_DISTANCE_TARGET:
    {
        return FitnessDistanceTarget(world, creature);
    } break;

    case TRAIN_DISTANCE_X:
    {
        return FitnessStrictlyMoveRight(world, creature);
    } break;

    case TRAIN_WALK_RIGHT:
    {
        return FitnessWalkRight(world, creature);
    } break;

    default:
    {
        DebugOut("ERROR: Fitness strategy not implemented");
        return -1.0;
    }

    }
}

internal inline void
UpdateFakeWorld(FakeWorld *world)
{
    for(ui32 creatureIdx = 0;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        Creature *creature = world->creatures+creatureIdx;
        UpdateCreature(world, creature);
    }
    cpSpaceStep(world->space, 1.0/60.0);
    for(ui32 rigidBodyIdx = 0;
            rigidBodyIdx < world->nRigidBodies;
            rigidBodyIdx++)
    {
        RigidBody *body = world->rigidBodies+rigidBodyIdx;
        if(!cpBodyIsSleeping(body->body))
        {
            // Apply friction
            cpVect vel = cpBodyGetVelocity(body->body);
            vel = cpvmult(vel, 1.0-body->drag);
            cpBodySetVelocity(body->body, vel);
            cpFloat angVel = cpBodyGetAngularVelocity(body->body);
            // Angular friction is 20% of normal friction.
            angVel = (1.0-body->drag*0.2)*angVel;
            cpBodySetAngularVelocity(body->body, angVel);
        }
    }
}

void
RestartFakeWorld(FakeWorld *world)
{
    CreatureDefinition *def = &world->def;
    world->space = cpSpaceNew();

    world->physicsGroupCounter = 1U;
    world->nStaticBodies = 0;

#define DefineFixedWorldArray(type, counterName, maxName, maxValue, arrayName) \
    world->counterName = 0;\
    world->maxName = maxValue;\
    world->arrayName = PushArray(world->transientMemory, type, maxValue)

    DefineFixedWorldArray(RigidBody, nRigidBodies, maxRigidBodies, 512, rigidBodies);
    DefineFixedWorldArray(Creature, nCreatures, maxCreatures, 128, creatures);
    DefineFixedWorldArray(BodyPart, nBodyParts, maxBodyParts, 512, bodyParts);
    DefineFixedWorldArray(RotaryMuscle, nRotaryMuscles, maxRotaryMuscles, 512, rotaryMuscles);
#undef DefineFixedWorldArray

    ui32 transientStateSize = GetMinimalGatedUnitStateSize(def->nInputs, 
            def->nOutputs,
            def->nHidden);

    // Set new target
    if(world->trainingType==TRAIN_DISTANCE_TARGET)
    {
        r32 randomAngle = RandomR32(-M_PI, M_PI);
        world->target = V2Polar(randomAngle, 500.0);
    }
    else if(world->trainingType==TRAIN_WALK_RIGHT)
    {
        AddStaticRectangle(world, V2(0,-150.0), 3200.0, 40.0, 0.0);
        cpSpaceSetGravity(world->space, cpv(0, -1200.0));
    }

    r32 startXDev = 0.0;
    for(int creatureIdx = 0; 
            creatureIdx < world->nGenes;
            creatureIdx++)
    {
        MinimalGatedUnit *brain = PushStruct(world->transientMemory, MinimalGatedUnit);
        VecR32 *gene = world->strategies->genes+creatureIdx;
        r32 *state = PushAndZeroArray(world->transientMemory, r32, transientStateSize);
        InitMinimalGatedUnit(brain, def->nInputs, def->nOutputs, def->nHidden, gene, state);
        AddCreature(world, V2(RandomR32(-startXDev, startXDev), 0), &world->def, brain);
    }
    //AddStaticRectangle(world, V2(0,0), 1600.0, 40, 0.0);
}

void
InitFakeWorld(FakeWorld *world, 
        MemoryArena *persistentMemory, 
        MemoryArena *transientMemory, 
        CreatureDefinition *creatureDefinition, 
        ui32 nGenes,
        r32 learningRate,
        r32 dev)
{
    world->persistentMemory = persistentMemory;
    world->transientMemory = transientMemory;
    world->def = *creatureDefinition;
    world->nGenes = nGenes;

    // Set trainging scenario.
    world->trainingType = TRAIN_WALK_RIGHT;

    DebugOut("Gene size = %u" ,world->def.geneSize);

    // Create es from definition
    world->strategies = ESCreate(persistentMemory, world->def.geneSize, world->nGenes, dev, learningRate);
    ESGenerateGenes(world->strategies);
    RestartFakeWorld(world);
}

void
DestroyFakeWorld(FakeWorld *world)
{
    cpSpaceFree(world->space);
    for(ui32 creatureIdx = 0;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        DestroyCreature(world->creatures+creatureIdx);
    }
    ClearArena(world->transientMemory);
}


