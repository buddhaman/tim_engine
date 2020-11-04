
Guy *
AddGuy(World *world, Vec3 pos)
{
    Guy *guy = world->guys + world->nGuys++;
    *guy = (Guy){};
    guy->pos = pos;
    guy->body = CreateBodyAsPerson(world, pos);

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
        // put feet on ground and apply impulse to head
        guy->lFoot->pos = guy->pos;
        guy->rFoot->pos = guy->pos;
        AddImpulse(guy->head, vec3(0,0,0.2));
        guy->pos.x+=0.05;
        guy->pos.y+=0.05;
    }
}
