
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
    cpFloat mass = 1;
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
        }
    }
}

Vec2
GetBodyPos(RigidBody *body)
{
    cpVect pos = cpBodyGetPosition(body->body);
    return vec2(pos.x, pos.y);
}

r32
GetBodyAngle(RigidBody *body)
{
    return (r32)cpBodyGetAngle(body->body);
}

// For now assumes batch has already begun.
void
DrawFakeWorld(FakeWorld *world, SpriteBatch *batch, Camera2D *camera, AtlasRegion *texture)
{
    r32 lineWidth = 2;
#if 0
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < world->nRigidBodies;
            bodyPartIdx++)
    {
        RigidBody *body = world->rigidBodies+bodyPartIdx;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);

        batch->colorState = vec4(0.0, 0.0, 0.0, 1.0);
        PushOrientedLineRectangle2(batch, 
                pos,
                body->width+lineWidth,
                body->height+lineWidth,
                angle,
                2,
                texture);
    }
#endif

    for(ui32 creatureIdx = 0;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        Creature *creature = world->creatures+creatureIdx;
        for(ui32 bodyPartIdx = 0;
                bodyPartIdx < creature->nBodyParts;
                bodyPartIdx++)
        {
            BodyPart *part = creature->bodyParts+bodyPartIdx;
            RigidBody *body = part->body;
            Vec2 pos = GetBodyPos(body);
            r32 angle = GetBodyAngle(body);
            r32 alpha = creatureIdx==(world->nCreatures-1) ? 1.0 : 0.2;
            r32 shade = 1.0-part->body->drag;

            if(creatureIdx==(world->nCreatures-1))
            {
            batch->colorState = vec4(0,0,0,1);
            PushOrientedRectangle2(batch, 
                    pos,
                    body->width+lineWidth,
                    body->height+lineWidth,
                    angle,
                    texture);
            }
            batch->colorState = vec4(shade, shade, shade, alpha);
            PushOrientedRectangle2(batch, 
                    pos,
                    body->width,
                    body->height,
                    angle,
                    texture);
        }
    }
}

void
RestartFakeWorld(FakeWorld *world)
{
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

    ui32 transientStateSize = GetMinimalGatedUnitStateSize(world->inputSize, 
            world->outputSize, 
            world->hiddenSize);
    for(int creatureIdx = 0; 
            creatureIdx < world->nGenes;
            creatureIdx++)
    {
        MinimalGatedUnit *brain = PushStruct(world->transientMemory, MinimalGatedUnit);
        VecR32 *gene = world->strategies->genes+creatureIdx;
        r32 *state = PushAndZeroArray(world->transientMemory, r32, transientStateSize);
        InitMinimalGatedUnit(brain, world->inputSize, world->outputSize, world->hiddenSize, gene, state);
        AddCreature(world, vec2(0,0), &world->def, brain);
    }
    //AddStaticRectangle(world, vec2(0,0), 1600.0, 40, 0.0);
}

void
InitFakeWorld(FakeWorld *world, MemoryArena *persistentMemory, MemoryArena *transientMemory)
{
    world->persistentMemory = persistentMemory;
    world->transientMemory = transientMemory;

    // Create creature definition
    r32 partWidth = 10;
    r32 partHeight = 40;
    ui32 bodyParts = 16;
    for(int bodyPartIdx = 0; 
            bodyPartIdx < bodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *body = world->def.bodyParts+world->def.nBodyParts++;
        body->pos = vec2(0, bodyPartIdx*partHeight);
        body->width = partWidth;
        body->height = partHeight;
        body->angle = 0;
    }
    for(int bodyPartIdx = 0; 
            bodyPartIdx < bodyParts-1;
            bodyPartIdx++)
    {
        RotaryMuscleDefinition *muscle = world->def.rotaryMuscles + world->def.nRotaryMuscles++;
        BodyPartDefinition *a = world->def.bodyParts+bodyPartIdx;
        BodyPartDefinition *b = world->def.bodyParts+bodyPartIdx+1;
        Vec2 pivotPoint = v2_muls(v2_add(a->pos, b->pos), 0.5);

        muscle->bodyPartIdx0 = bodyPartIdx;
        muscle->bodyPartIdx1 = bodyPartIdx+1;
        muscle->pivotPoint = pivotPoint;
        muscle->minAngle = -1;
        muscle->maxAngle = 1;
    }

    world->inputSize = 1;
    world->outputSize = world->def.nBodyParts+world->def.nRotaryMuscles;
    world->hiddenSize = 1;
    world->geneSize = GetMinimalGatedUnitGeneSize(world->inputSize, world->outputSize, world->hiddenSize);
    world->nGenes = 4;

    // Create ee from definition
    world->strategies = ESCreate(persistentMemory, world->geneSize, world->nGenes, 0.01, 0.008);
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


