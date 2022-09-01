
enum {
    GROUND_MASK = 1<<0,
    DYNAMIC_MASK = 1<<1
};

RigidBody *
AddDynamicRectangle(FakeWorld *world, Vec2 pos, R32 width, R32 height, R32 angle, U32 group)
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
AddStaticRectangle(FakeWorld *world, Vec2 pos, R32 width, R32 height, R32 angle)
{
    RigidBody *body = world->staticBodies+world->nStaticBodies++;
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

Grass *
AddGrass(FakeWorld *world, 
        Vec2 pos, 
        R32 width, 
        R32 topHeight, 
        R32 ovalHeight)
{
    Assert(world->nGrass < world->maxGrass);
    Grass *grass = world->grass+world->nGrass++;
    *grass = (Grass){};
    grass->pos = pos;
    grass->width = width;
    grass->topHeight = topHeight;
    grass->ovalHeight = ovalHeight;
    return grass;
}

StaticPlatform *
AddStaticPlatform(FakeWorld *world, Vec2 pos, Vec2 dims)
{
    StaticPlatform *platform = world->staticPlatforms+world->nStaticPlatforms++;
    *platform = (StaticPlatform){};

    platform->bounds.pos  = pos;
    platform->bounds.dims = dims;
    
    platform->body = AddStaticRectangle(world, pos, dims.x, dims.y, 0.0f);

    R32 avgWidth     = 60.0f;
    R32 widthSpread  = 5.0f;
    R32 avgHeight    = 20.0f;
    R32 heightSpread = 3.0f;
    R32 overhang     = 3.0f;

    int nGrass = (int)(dims.x/avgWidth)+1;
    R32 atX = pos.x-dims.x/2.0f;
    R32 toX = pos.x+dims.x/2.0f;
    for(int grassIdx = 0;
            grassIdx < nGrass*2; // Just a random limit to prevent infinite loop.
            grassIdx++)
    {
        B32 last = 0;
        R32 width;
        if(toX-atX < avgWidth*2)
        {
            last = 1;
            width = toX-atX;
        }
        else
        { 
            width = RandomR32(avgWidth-widthSpread, avgWidth+widthSpread);
        }
        R32 topHeight = RandomR32(avgHeight-heightSpread, avgHeight+heightSpread);
        R32 ovalHeight = Min(width, topHeight*1.8f);
        AddGrass(world, V2(atX-overhang, pos.y+dims.y/2.0f-topHeight), 
                width + overhang*2.0f,
                topHeight+overhang, 
                ovalHeight);

        atX+=width;
        if(last) break;
    }

    atX = pos.x-dims.x/2.0f;
    R32 atY = pos.y+dims.y/2.0f;
    R32 r = 30.0f;
    R32 spread = 10.0f;
    R32 angleSpread = M_PI/3.0f;
    while(atX < toX)
    {
        atX+=RandomR32(100, 200);
        if(atX > toX) break;
        Assert(world->nTallGrass < world->maxTallGrass);
        TallGrass *grass = world->tallGrass+world->nTallGrass++;
        grass->from = V2(atX, atY);
        
        int N = RandomUI32(4, 6);
        for(int i = 0; i < N; i++)
        {
            R32 theta = M_PI/2.0-angleSpread+angleSpread*2*i/((R32)(N-1));
            grass->to[grass->nBlades++] = 
                V2Add(grass->from, V2Polar(theta, RandomR32(r-spread, r+spread)));
        }
    }

    atX = pos.x-dims.x/2.0f;
    r = 200.0f;
    while(atX < toX)
    {
        atX+=RandomR32(200, 300);
        if(atX > toX) break;
        Assert(world->nBushes < world->maxBushes);
        Bush *bush = world->bushes+world->nBushes++;

        Vec2 pos = V2(atX, atY);
        bush->leafs[bush->nLeafs] = pos;
        bush->r[bush->nLeafs] = 50.0f;
        bush->nLeafs++;
        r = 60.0f;
        int N = 6;
        for(int i = 0; i < N; i++)
        {
            R32 theta = M_PI/2.0-angleSpread+angleSpread*2*i/((R32)(N-1));
            bush->leafs[bush->nLeafs] = V2Add(pos, V2Polar(theta, RandomR32(r-spread, r+spread)));
            bush->r[bush->nLeafs] = 40.0f;
            bush->nLeafs++;
        }
        r = 90.0f;
        N = 10;
        for(int i = 0; i < 8; i++)
        {
            R32 theta = M_PI/2.0-angleSpread+angleSpread*2*i/((R32)(N-1));
            bush->leafs[bush->nLeafs] = V2Add(pos, V2Polar(theta, RandomR32(r-spread, r+spread)));
            bush->r[bush->nLeafs] = 25.0f;
            bush->nLeafs++;
        }
    }

    return platform;
}

void
DestroyRigidBody(FakeWorld *world, RigidBody *body)
{
    cpSpaceRemoveShape(world->space, body->shape);
    cpShapeDestroy(body->shape);
    cpShapeFree(body->shape);

    cpSpaceRemoveBody(world->space, body->body);
    cpBodyDestroy(body->body);
    cpBodyFree(body->body);
}

internal inline Vec2
GetBodyPos(RigidBody *body)
{
    cpVect pos = cpBodyGetPosition(body->body);
    return V2(pos.x, pos.y);
}

internal inline R32
GetBodyAngle(RigidBody *body)
{
    return (R32)cpBodyGetAngle(body->body);
}

internal inline OrientedBox
GetRigidBodyBox(RigidBody *body)
{
    cpVect pos = cpBodyGetPosition(body->body);
    R32 angle = GetBodyAngle(body);
    return (OrientedBox){V2(pos.x, pos.y), V2(body->width, body->height), angle};
}

internal R32
FitnessStrictlyMoveRight(FakeWorld *world, Creature *creature)
{
    R32 minX = 10000.0;
    R32 maxY = -10000.0;
    for(U32 bodyPartIdx = 0;
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

internal R32
FitnessDistanceTarget(FakeWorld *world, Creature *creature)
{
    // Position of main bodypart
    Vec2 creaturePos = GetBodyPos(creature->bodyParts->body);
    R32 dist = V2Dist(creaturePos, world->target);
    return -dist;
}

internal R32
FitnessWalkRight(FakeWorld *world, Creature *creature)
{
    BodyPart *part = creature->bodyParts;
    Vec2 pos = GetBodyPos(part->body);
    return pos.x;
}

internal R32
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
    for(U32 creatureIdx = 0;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        Creature *creature = world->creatures+creatureIdx;
        UpdateCreature(world, creature);
    }
    cpSpaceStep(world->space, 1.0/60.0);
    for(U32 rigidBodyIdx = 0;
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

    U32 transientStateSize = GetMinimalGatedUnitStateSize(def->nInputs, 
            def->nOutputs,
            def->nHidden);

    // Set new target
    if(world->trainingType==TRAIN_DISTANCE_TARGET)
    {
        R32 randomAngle = RandomR32(-M_PI, M_PI);
        world->target = V2Polar(randomAngle, 500.0);
    }

    R32 startXDev = 0.0;
    for(int creatureIdx = 0; 
            creatureIdx < world->nGenes;
            creatureIdx++)
    {
        MinimalGatedUnit *brain = PushStruct(world->transientMemory, MinimalGatedUnit);
        VecR32 *gene = world->strategies->genes+creatureIdx;
        R32 *state = PushAndZeroArray(world->transientMemory, R32, transientStateSize);
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
        U32 nGenes,
        R32 learningRate,
        R32 dev)
{
    world->persistentMemory = persistentMemory;
    world->transientMemory = transientMemory;
    world->def = *creatureDefinition;
    world->nGenes = nGenes;

    // Set trainging scenario.
    world->trainingType = TRAIN_WALK_RIGHT;
    world->size = V2(16000.0f, 3000.0f);
    world->origin = V2MulS(world->size, -0.5f);

    world->maxGrass = 2048;
    world->nGrass = 0;
    world->grass = PushAndZeroArray(persistentMemory, Grass, world->maxGrass);

    world->maxTallGrass = 256;
    world->nTallGrass = 0;
    world->tallGrass = PushAndZeroArray(persistentMemory, TallGrass, world->maxTallGrass);

    world->maxBushes = 128;
    world->nBushes = 0;
    world->bushes = PushAndZeroArray(persistentMemory, Bush, world->maxBushes);

    world->maxStaticPlatforms = 64;
    world->nStaticPlatforms = 0;
    world->staticPlatforms = PushAndZeroArray(persistentMemory, StaticPlatform, world->maxStaticPlatforms);

    world->maxStaticBodies = 64;
    world->nStaticBodies = 0;
    world->staticBodies = PushAndZeroArray(persistentMemory, RigidBody, world->maxStaticBodies);

    DebugOut("Gene size = %u" ,world->def.geneSize);

    world->space = cpSpaceNew();

    if(world->trainingType==TRAIN_WALK_RIGHT)
    {
        AddStaticPlatform(world, V2(800.0f,-500.0), V2(3600.0, 800.0));
        cpSpaceSetGravity(world->space, cpv(0, -1200.0));
    }

    // Create es from definition
    world->strategies = ESCreate(persistentMemory, world->def.geneSize, world->nGenes, dev, learningRate);
    ESGenerateGenes(world->strategies);
    RestartFakeWorld(world);
}

void
DestroyFakeWorld(FakeWorld *world)
{
    //cpSpaceFree(world->space);
    //cpSpaceDestroy(world->space);
    for(U32 creatureIdx = 0;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        DestroyCreature(world, world->creatures+creatureIdx);
    }
    ClearArena(world->transientMemory);
}


