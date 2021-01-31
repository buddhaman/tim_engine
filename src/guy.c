// Behavior

BTNODE_UPDATE_FUNCTION(updateLeafNodeStub)
{
    DebugOut("Updating leaf node stub");

    return BT_SUCCESS;
}

BTNODE_UPDATE_FUNCTION(updateWalkTo)
{
    r32 xDiff = guy->walkTo.x - guy->pos.x;
    r32 yDiff = guy->walkTo.y - guy->pos.y;
    r32 angDiff = guy->orientation - atan2f(yDiff, xDiff);
    r32 rotationSpeed = 0.01;
    if(angDiff < 0)
    {
        guy->orientation+=rotationSpeed;
    }
    else
    {
        guy->orientation-=rotationSpeed;
    }
    return BT_SUCCESS;
}

BTNode *
BTCreateNode(BTNodeType type)
{
    BTNode *node = malloc(sizeof(BTNode));

    *node = (BTNode){};
    node->update = updateLeafNodeStub;
    node->type = type;

    return node;
}

BTNode *
BTCreateLeaf(btnode_update *update)
{
    BTNode *leaf = BTCreateNode(BT_LEAF);
    leaf->update = update;
    return leaf;
}

void
AddChildToNode(BTNode *parent, BTNode *child)
{
    parent->children[parent->nChildren++] = child;
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
    guy->behavior = BTCreateLeaf(updateWalkTo);
    guy->walkTo = vec3(20, 20, 0);
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

BTNodeResult
BTUpdateNode(BTNode *node, World *world, Guy *guy)
{
    BTNodeResult result = BT_FAIL;
    switch(guy->behavior->type)
    {

    case BT_SEQUENCE:
    {
        b32 hasFailed = 0;
        for(int childIdx = 0;
                childIdx < node->nChildren;
                childIdx++)
        {
            BTNode *child = node->children[childIdx];
            BTNodeResult childResult = BTUpdateNode(child, world, guy);
            if(childResult!=BT_SUCCESS)
            {
                result = childResult;
                hasFailed = 1;
                break;
            }
        }
        if(!hasFailed)
        {
            result = BT_SUCCESS;
        }
    } break;

    case BT_SELECTOR:
    {
        b32 hasFailed = 1;
        for(int childIdx = 0;
                childIdx < node->nChildren;
                childIdx++)
        {
            BTNode *child = node->children[childIdx];
            BTNodeResult childResult = BTUpdateNode(child, world, guy);
            if(childResult!=BT_FAIL)
            {
                result = childResult;
                hasFailed = 0;
                break;
            }
        }
        if(hasFailed)
        {
            result = BT_FAIL;
        }
    } break;

    case BT_LEAF:
    {
        result = node->update(node, world, guy);
    } break;

    }
    return result;
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

        r32 c = cosf(guy->orientation);
        r32 s = sinf(guy->orientation);
        r32 speed = 0.4;
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

        BTUpdateNode(guy->behavior, world, guy);
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


