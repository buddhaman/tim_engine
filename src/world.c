
void
InitWorld(World *world, MemoryArena *arena)
{
    world->arena = arena;
    world->space = cpSpaceNew();
    //cpSpaceSetGravity(world->space, cpv(0, -5000));
    cpSpaceSetGravity(world->space, cpv(0, 0));

    world->physicsGroupCounter = 1U;
    world->nRigidBodies = 0;
    world->maxRigidBodies = 512;
    world->rigidBodies = PushArray(arena, RigidBody, world->maxRigidBodies);

    world->nCreatures = 0;
    world->maxCreatures = 128;
    world->creatures = PushArray(arena, Creature, world->maxCreatures);

    world->pointerArraySize = 32;
    world->pointerArrayPool = CreateMemoryPool(arena, 
            sizeof(void*)*world->pointerArraySize, 3);

    world->maxBodypartsPerCreature = 32;
    world->bodyPartPool = CreateMemoryPool(arena, 
            sizeof(BodyPart)*world->maxBodypartsPerCreature, 3);

    world->maxRotaryMusclesPerCreature = 32;
    world->bodyPartPool = CreateMemoryPool(arena, 
            sizeof(RotaryMuscle)*world->maxRotaryMusclesPerCreature, 3);

}

enum {
    GROUND_MASK = 1<<0,
    DYNAMIC_MASK = 1<<1
};

RigidBody *
AddDynamicRectangle(World *world, Vec2 pos, r32 width, r32 height, r32 angle, ui32 group)
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
    cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body->body, width, height, 0.01));
    cpShapeSetFilter(shape, cpShapeFilterNew(group, DYNAMIC_MASK, DYNAMIC_MASK | GROUND_MASK));

    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);

    cpShapeSetFriction(shape, 0.8);
    cpShapeSetElasticity(shape, 0.0);

    return body;
}

RigidBody *
AddStaticRectangle(World *world, Vec2 pos, r32 width, r32 height, r32 angle)
{
    RigidBody *body = world->rigidBodies+world->nRigidBodies++;
    cpSpace *space = world->space;
    body->pos = pos;
    body->width = width;
    body->height = height;

    // Create cpBody
    body->body = cpSpaceAddBody(space, cpBodyNewStatic());
    cpShape *shape = cpBoxShapeNew(body->body, width, height, 0.01);
    cpSpaceAddShape(space, shape);
    cpShapeSetFilter(shape, cpShapeFilterNew(0, GROUND_MASK, DYNAMIC_MASK));
    
    cpShapeSetFriction(shape, 1.0);
    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);
    cpSpaceReindexShapesForBody(space, body->body);

    return body;
}

void
RotaryLimitJoint(World *world, RigidBody *bodyA, RigidBody *bodyB, Vec2 pivotPoint, r32 minAngle, r32 maxAngle)
{
    cpConstraint *pivotConstraint = cpPivotJointNew(bodyA->body, bodyB->body, cpv(pivotPoint.x, pivotPoint.y));
    cpSpaceAddConstraint(world->space, pivotConstraint);

    cpConstraint *rotaryLimitConstraint = cpRotaryLimitJointNew(bodyA->body, bodyB->body, minAngle, maxAngle);
    cpSpaceAddConstraint(world->space, rotaryLimitConstraint);
}

void
UpdateWorld(World *world)
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
DrawWorld(World *world, SpriteBatch *batch, Camera2D *camera, AtlasRegion *texture)
{
    r32 lineWidth = 2;
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

            r32 shade = 1.0-part->body->drag;
            batch->colorState = vec4(shade, shade, shade, 1.0);
            PushOrientedRectangle2(batch, 
                    pos,
                    body->width,
                    body->height,
                    angle,
                    texture);
        }
    }
}

