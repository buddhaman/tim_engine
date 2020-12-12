void
InitWorld(MemoryArena *arena, World *world, int maxParticles, int maxConstraints, int maxBodies)
{
    world->time = 0;

    world->width = 100.0;
    world->height = 100.0;

    world->gravity = vec3(0,0,-0.01);

    world->maxParticles = maxParticles;
    world->nParticles = 0;
    world->particles = PushArray(arena, Verlet, maxParticles);

    world->maxConstraints = maxConstraints;
    world->nConstraints = 0;
    world->constraints = PushArray(arena, Constraint, maxConstraints);

    world->maxBodies = maxBodies;
    world->nBodies = 0;
    world->bodies = PushArray(arena, Body, maxBodies);
    
    world->nGuys = 0;
    world->maxGuys = maxBodies;
    world->guys = PushArray(arena, Guy, world->maxGuys);

    world->nSwords = 0;
    world->maxSwords = maxBodies;
    world->swords = PushArray(arena, Sword, world->maxSwords);
}

Verlet *
AddParticle(World *world, Body *body, Vec3 pos)
{
    Verlet *particle = world->particles + world->nParticles++;
    particle->pos = pos;
    particle->oldPos = pos;
    body->nParticles++;
    return particle;
}

internal inline void
AddImpulse(Verlet *particle, Vec3 impulse)
{
    particle->oldPos = v3_sub(particle->oldPos, impulse);
}

internal inline void
BodyCheckZCollision(Body *body, r32 z, r32 contactFriction)
{
    for(int particleIdx = 0;
            particleIdx < body->nParticles;
            particleIdx++)
    {
        Verlet *particle = body->particles+particleIdx;
        if(particle->pos.z < z)
        {
            particle->pos.z = z;
            particle->oldPos.x+=(particle->pos.x-particle->oldPos.x)*contactFriction;
            particle->oldPos.y+=(particle->pos.y-particle->oldPos.y)*contactFriction;
        }
    }
}

internal inline Constraint *
Connect(World *world, Body *body, int idx0, int idx1)
{
    Constraint *c = world->constraints + world->nConstraints++;
    c->a = body->particles + idx0;
    c->b = body->particles + idx1;
    Vec3 diff = v3_sub(c->b->pos, c->a->pos);
    c->r = v3_length(diff);
    body->nConstraints++;
    return c;
}

void
InitPerson(World *world, Body *body, Vec3 pos, r32 unit)
{
    AddParticle(world, body, v3_add(pos, vec3(0,0,0)));                     // 0  butt 
    AddParticle(world, body, v3_add(pos, vec3(0,0,2*unit))); // 1  neck 
    AddParticle(world, body, v3_add(pos, vec3(0,0,2.5*unit)));              // 2  head 
    AddParticle(world, body, v3_add(pos, vec3(-unit,0,2*unit)));            // 3  left elbow 
    AddParticle(world, body, v3_add(pos, vec3(-unit*2,0,2*unit)));          // 4  left hand 
    AddParticle(world, body, v3_add(pos, vec3(unit,0,2*unit)));             // 5  right elbow
    AddParticle(world, body, v3_add(pos, vec3(unit*2,0,2*unit)));           // 6  righ hand
    AddParticle(world, body, v3_add(pos, vec3(-unit,0, 0)));                // 7  left knee
    AddParticle(world, body, v3_add(pos, vec3(-unit*2,0,0)));               // 8  left foot
    AddParticle(world, body, v3_add(pos, vec3(unit,0,0)));                  // 9  right knee
    AddParticle(world, body, v3_add(pos, vec3(unit*2,0,0)));                // 10 right foot

    // Not visible
    Connect(world, body, 1, 2);
    Connect(world, body, 0, 1);

    Connect(world, body, 1, 3);
    Connect(world, body, 3, 4);
    Connect(world, body, 1, 5);
    Connect(world, body, 5, 6);

    Connect(world, body, 0, 7);
    Connect(world, body, 7, 8);
    Connect(world, body, 0, 9);
    Connect(world, body, 9, 10);
}

void
InitSword(World *world, Body *body, Vec3 pos, r32 unit)
{
    AddParticle(world, body, v3_add(pos, vec3(0,0,0)));
    AddParticle(world, body, v3_add(pos, vec3(unit,0,0)));
    Connect(world, body, 0, 1);
}

// Don't call init of next body before creating entire body
void 
InitBody(World *world, Body *body)
{
    body->nParticles = 0;
    body->nConstraints = 0;
    body->particles = world->particles + world->nParticles;
    body->constraints = world->constraints + world->nConstraints;
}

Body *
CreateBodyAsSword(World *world, Vec3 pos, r32 unit)
{
    Body *body = world->bodies+world->nBodies++;
    InitBody(world, body);
    InitSword(world, body, pos, unit);
    return body;
}

Body *
CreateBodyAsPerson(World *world, Vec3 pos, r32 unit)
{
    Body *body = world->bodies + world->nBodies++;
    InitBody(world, body);
    InitPerson(world, body, pos, unit);
    return body;
}

void
BodyAddImpulse(Body *body, Vec3 impulse)
{
    for(int particleIdx = 0;
            particleIdx < body->nParticles;
            particleIdx++)
    {
        Verlet *particle = body->particles+particleIdx;
        AddImpulse(particle, impulse);
    }
}

void
DoPhysicsStep(World *world)
{
    world->time+=1.0/60;
    for(int particleIdx = 0; 
            particleIdx < world->nParticles;
            particleIdx++)
    {
        Verlet *particle = world->particles+particleIdx;
        Vec3 tmp = particle->pos;
        particle->pos = v3_sub(v3_muls(particle->pos, 2), particle->oldPos); 
        particle->oldPos = tmp;
    }
    for(int constraintIdx = 0;
            constraintIdx < world->nConstraints;
            constraintIdx++)
    {
        Constraint *c = world->constraints+constraintIdx;
        Vec3 diff = v3_sub(c->b->pos, c->a->pos);
        r32 l = v3_length(diff);
        r32 fact = (l-c->r)/l;
        diff=v3_muls(diff, fact*0.5);
        c->a->pos = v3_add(c->a->pos, diff);
        c->b->pos = v3_sub(c->b->pos, diff);
    }
}

