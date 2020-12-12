
Guy *
AddGuy(World *world, Vec3 pos)
{
    Guy *guy = world->guys + world->nGuys++;
    *guy = (Guy){};
    guy->unit = 1.0;
    guy->pos = pos;
    guy->body = CreateBodyAsPerson(world, pos, guy->unit);

    guy->head = guy->body->particles+2;
    guy->lFoot = guy->body->particles+8;
    guy->rFoot = guy->body->particles+10;

    return guy;
}

void
UpdateGuys(World *world)
{
    // Physics step is done automatically in world
    for(int guyIdx = 0;
            guyIdx < world->nGuys;
            guyIdx++)
    {
        Guy *guy = world->guys + guyIdx;

        guy->orientation += RandomFloat(-0.1, 0.1);
        r32 c = cosf(guy->orientation);
        r32 s = sinf(guy->orientation);
        r32 speed = 0.08;
        guy->pos.x+=c*speed;
        guy->pos.y+=s*speed;

        // put feet on ground and apply impulse to head. Pretend ground is at 0
        Vec3 lFootPos = v3_add(guy->pos, vec3(-s,c,0));
        Vec3 rFootPos = v3_add(guy->pos, vec3(s,-c,0));
        guy->lFoot->pos = lFootPos;
        guy->rFoot->pos = rFootPos;
        AddImpulse(guy->head, vec3(0,0,0.1));
    }
}

