
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

void
DrawGrid(SpriteBatch *batch, 
        Camera2D *camera, 
        r32 gridResolution, 
        r32 gridLineWidth,
        AtlasRegion *texture)
{
    r32 minX = camera->pos.x-camera->size.x/2;
    r32 minY = camera->pos.y-camera->size.y/2;
    r32 maxX = camera->pos.x+camera->size.x;
    r32 maxY = camera->pos.y+camera->size.y;
    ui32 nXLines = camera->size.x/gridResolution+2;
    ui32 nYLines = camera->size.y/gridResolution+2;
    r32 xStart = floorf(minX/gridResolution)*gridResolution;
    r32 yStart = floorf(minY/gridResolution)*gridResolution;
    ui32 maxLines = 100;
    ui32 fadeAfter = 50;
    r32 alpha = 0.2;
    ui32 mostLines = Max(nXLines, nYLines);
    if(mostLines < maxLines)
    {
        r32 fade = (maxLines-mostLines)/((r32)(maxLines-fadeAfter));
        if(fade > 1.0) fade = 1.0;
        batch->colorState = vec4(1.0, 1.0, 1.0, fade*alpha);
        for(ui32 xIdx = 0; xIdx < nXLines; xIdx++)
        {
            r32 x = xStart+xIdx*gridResolution;
            PushRect2(batch, 
                    vec2(x-gridLineWidth/2.0, minY), 
                    vec2(gridLineWidth, maxY-minY), 
                    texture->pos, 
                    texture->size);
        }
        for(ui32 yIdx = 0; yIdx < nYLines; yIdx++)
        {
            r32 y = yStart+yIdx*gridResolution;
            PushRect2(batch, 
                    vec2(minX, y-gridLineWidth/2.0),
                    vec2(maxX-minX, gridLineWidth), 
                    texture->pos, 
                    texture->size);
        }
    }
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

    DrawGrid(batch, camera, 10.0, 1.0, texture);
    DrawGrid(batch, camera, 50.0, 2.0, texture);

    Vec4 creatureColor = vec4(1.0, 0.8, 0.8, 1.0);
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

            batch->colorState = vec4(shade*creatureColor.x, 
                    shade*creatureColor.y, 
                    shade*creatureColor.z, 
                    alpha);
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

BodyPartDefinition *
DefineBodyPart(CreatureDefinition *def, 
        Vec2 pos,
        Vec2 dims,
        r32 angle)
{
    BodyPartDefinition *body = def->bodyParts+def->nBodyParts++;
    body->pos = pos;
    body->width = dims.x;
    body->height = dims.y;
    body->angle = angle;
    return body;
}

RotaryMuscleDefinition *
DefineRotaryMuscle(CreatureDefinition *def,
        ui32 bodyPartIdx0,
        ui32 bodyPartIdx1,
        Vec2 pivotPoint,
        r32 minAngle, 
        r32 maxAngle)
{
    RotaryMuscleDefinition *muscle =def->rotaryMuscles + def->nRotaryMuscles++;
    muscle->bodyPartIdx0 = bodyPartIdx0;
    muscle->bodyPartIdx1 = bodyPartIdx1;
    muscle->pivotPoint = pivotPoint;
    muscle->minAngle = minAngle;
    muscle->maxAngle = maxAngle;
    return muscle;
}

void
InitFakeWorld(FakeWorld *world, MemoryArena *persistentMemory, MemoryArena *transientMemory)
{
    world->persistentMemory = persistentMemory;
    world->transientMemory = transientMemory;

    // Create creature definition
#if 0
    r32 partWidth = 10;
    r32 partHeight = 40;
    ui32 bodyParts = 16;
    for(int bodyPartIdx = 0; 
            bodyPartIdx < bodyParts;
            bodyPartIdx++)
    {
        DefineBodyPart(&world->def, 
            vec2(0, bodyPartIdx*partHeight),
            vec2(partWidth, partHeight),
            0);
    }
    for(int bodyPartIdx = 0; 
            bodyPartIdx < bodyParts-1;
            bodyPartIdx++)
    {
        //RotaryMuscleDefinition *muscle = world->def.rotaryMuscles + world->def.nRotaryMuscles++;
        BodyPartDefinition *a = world->def.bodyParts+bodyPartIdx;
        BodyPartDefinition *b = world->def.bodyParts+bodyPartIdx+1;
        Vec2 pivotPoint = v2_muls(v2_add(a->pos, b->pos), 0.5);

        DefineRotaryMuscle(&world->def,bodyPartIdx, bodyPartIdx+1, pivotPoint, -1, 1);
    }
#endif

    // Handmade creature
    DefineBodyPart(&world->def, vec2(0, 0), vec2(40, 40), 0);

    DefineBodyPart(&world->def, vec2(40, 0), vec2(40, 10), 0);
    DefineBodyPart(&world->def, vec2(80, 0), vec2(40, 10), 0);
    DefineBodyPart(&world->def, vec2(120, 0), vec2(40, 10), 0);

    DefineBodyPart(&world->def, vec2(-40, 0), vec2(40, 10), 0);
    DefineBodyPart(&world->def, vec2(-80, 0), vec2(40, 10), 0);
    DefineBodyPart(&world->def, vec2(-120, 0), vec2(40, 10), 0);

    DefineBodyPart(&world->def, vec2(0, -40), vec2(10, 40), 0);
    DefineBodyPart(&world->def, vec2(0, -80), vec2(10, 40), 0);
    DefineBodyPart(&world->def, vec2(0, -120), vec2(10, 40), 0);

    DefineBodyPart(&world->def, vec2(0, 40), vec2(10, 40), 0);
    DefineBodyPart(&world->def, vec2(0, 80), vec2(10, 40), 0);
    DefineBodyPart(&world->def, vec2(0, 120), vec2(10, 40), 0);

    DefineRotaryMuscle(&world->def, 0, 1, vec2(20, 0), -1, 1);
    DefineRotaryMuscle(&world->def, 1, 2, vec2(60, 0), -1, 1);
    DefineRotaryMuscle(&world->def, 2, 3, vec2(100, 0), -1, 1);

    DefineRotaryMuscle(&world->def, 0, 4, vec2(-20, 0), -1, 1);
    DefineRotaryMuscle(&world->def, 4, 5, vec2(-60, 0), -1, 1);
    DefineRotaryMuscle(&world->def, 5, 6, vec2(-100, 0), -1, 1);

    DefineRotaryMuscle(&world->def, 0, 7, vec2(0, -20), -1, 1);
    DefineRotaryMuscle(&world->def, 7, 8, vec2(0, -60), -1, 1);
    DefineRotaryMuscle(&world->def, 8, 9, vec2(0, -100), -1, 1);

    DefineRotaryMuscle(&world->def, 0, 10, vec2(0, 20), -1, 1);
    DefineRotaryMuscle(&world->def, 10, 11, vec2(0, 60), -1, 1);
    DefineRotaryMuscle(&world->def, 11, 12, vec2(0, 100), -1, 1);

    world->inputSize = 1;
    world->outputSize = world->def.nBodyParts+world->def.nRotaryMuscles;
    world->hiddenSize = 1;
    world->geneSize = GetMinimalGatedUnitGeneSize(world->inputSize, world->outputSize, world->hiddenSize);
    world->nGenes = 2;

    // Create es from definition
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


