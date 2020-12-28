// Behavior

BTNODE_UPDATE_FUNCTION(updateLeafNodeStub)
{
    DebugOut("Update function stub");
    return BT_SUCCESS;
}

BTNode *
BTCreateNode()
{
    BTNode *node = malloc(sizeof(BTNode));

    *node = (BTNode){};
    node->update = updateLeafNodeStub;
    node->type = BT_LEAF;

    return node;
}

// Guy

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
    guy->lHand = guy->body->particles+4;
    guy->rHand = guy->body->particles+6;
    guy->behavior = BTCreateNode();
    return guy;
}

Sword *
AddSword(World *world, Vec3 pos)
{
    Sword *sword = world->swords+world->nSwords++;
    *sword = (Sword){};
    sword->unit = 3.0;
    sword->body = CreateBodyAsSword(world, pos, sword->unit);
    sword->handle = sword->body->particles;
    sword->tip = sword->body->particles+1;
    return sword;
}

Guy *
IntersectGuys(World *world, Vec3 selectAt, r32 radius)
{
    Guy *selected = NULL;
    for(int guyIdx = 0;
            guyIdx < world->nGuys; 
            guyIdx++)
    {
        Guy *guy = world->guys + guyIdx;
        Vec3 diff = v3_sub(guy->pos, selectAt);
        if(v3_length(diff) < radius)
        {
            return guy;
        }
    }
    return selected;
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

        if(guy->sword)
        {
            guy->sword->handle->pos = guy->lHand->pos;
        }

        guy->behavior->update(NULL);
    }
}

void
UpdateSwords(World *world)
{
    for(int swordIdx = 0;
            swordIdx < world->nSwords;
            swordIdx++)
    {
        Sword *sword = world->swords + swordIdx;
        Body *body = sword->body;
        BodyAddImpulse(body, world->gravity);
        BodyCheckZCollision(body, 0.1, 0.05);
    }
}

void
CollideGuySword(World *world)
{
    r32 radius = 2;
    for(int guyIdx = 0;
            guyIdx < world->nGuys;
            guyIdx++)
    {
        Guy *guy = world->guys+guyIdx;
        if(guy->sword)
        {
            continue;
        }
        else
        {
            for(int swordIdx = 0;
                    swordIdx < world->nSwords;
                    swordIdx++)
            {
                Sword *sword = world->swords+swordIdx;
                if(sword->guy)
                {
                    continue;
                }
                else
                {
                    Vec3 swordPos = sword->handle->pos;
                    Vec3 guyPos = guy->lHand->pos;
                    Vec3 diff = v3_sub(swordPos, guyPos);
                    if(v3_length(diff) < radius)
                    {
                        guy->sword = sword;
                        sword->guy = guy;
                    }
                }
            }
        }
    }
}

