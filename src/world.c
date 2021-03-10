
void
InitWorld(World *world, MemoryArena *arena)
{
    world->space = cpSpaceNew();
    cpSpaceSetGravity(world->space, cpv(0, -800));

    world->nRigidBodies = 0;
    world->maxRigidBodies = 128;
    world->rigidBodies = PushArray(arena, RigidBody, world->maxRigidBodies);
}

RigidBody *
AddDynamicRectangle(World *world, Vec2 pos, r32 width, r32 height, r32 angle)
{
    RigidBody *body = world->rigidBodies+world->nRigidBodies++;
    cpSpace *space = world->space;
    body->pos = pos;
    body->width = width;
    body->height = height;

    // Create cpBody
    cpFloat mass = 1;
    cpFloat moment = cpMomentForBox(mass, width, height);
    body->body = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    cpShape *shape = cpSpaceAddShape(space, cpBoxShapeNew(body->body, width, height, 0.01));

    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);

    cpShapeSetFriction(shape, 0.8);

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
    
    cpShapeSetFriction(shape, 1.0);
    cpBodySetPosition(body->body, cpv(pos.x, pos.y));
    cpBodySetAngle(body->body, angle);
    cpSpaceReindexShapesForBody(space, body->body);

    return body;
}

void
UpdateWorld(World *world)
{
    cpSpaceStep(world->space, 1.0/60.0);
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
    for(int rigidBodyIdx = 0;
            rigidBodyIdx < world->nRigidBodies;
            rigidBodyIdx++)
    {
        RigidBody *body = world->rigidBodies+rigidBodyIdx;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);

        PushOrientedLineRectangle2(batch, 
                pos,
                body->width,
                body->height,
                angle,
                2.0,
                texture);

    }
}

