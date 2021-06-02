
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
    return vec2(pos.x, pos.y);
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
    return (OrientedBox){vec2(pos.x, pos.y), vec2(body->width, body->height), angle};
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
    cpSpaceSetGravity(world->space, cpv(0, 0));

    world->physicsGroupCounter = 1U;

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

    r32 startXDev = 0.0;
    for(int creatureIdx = 0; 
            creatureIdx < world->nGenes;
            creatureIdx++)
    {
        MinimalGatedUnit *brain = PushStruct(world->transientMemory, MinimalGatedUnit);
        VecR32 *gene = world->strategies->genes+creatureIdx;
        r32 *state = PushAndZeroArray(world->transientMemory, r32, transientStateSize);
        InitMinimalGatedUnit(brain, def->nInputs, def->nOutputs, def->nHidden, gene, state);
        AddCreature(world, vec2(RandomR32(-startXDev, startXDev), 0), &world->def, brain);
    }
    //AddStaticRectangle(world, vec2(0,0), 1600.0, 40, 0.0);
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


