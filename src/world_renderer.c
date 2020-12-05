
void
DrawGuys(Mesh *mesh, World *world, Vec2 texCoord, Vec2 texSize,
        Vec2 circleCoord, Vec2 circleTexSize)
{
    for(int guyIdx = 0;
            guyIdx < world->nGuys;
            guyIdx++)
    {
        Guy *guy = world->guys + guyIdx;
        Body *body = guy->body;
        Vec3 headPos = guy->head->pos;

        r32 r = guy->orientation;
        mesh->colorState = vec4(1, 0, 0, 1);
        PushRect3(mesh, headPos, vec2(1,1), vec3(cosf(r),sinf(r), 0), vec3(0,0,1), circleCoord, circleTexSize);

        for(int cIdx = 0;
                cIdx < body->nConstraints;
                cIdx++)
        {
            Constraint *c = body->constraints+cIdx;
            Vec3 from = c->a->pos;
            Vec3 to = c->b->pos;
            PushLine(mesh, from, to, 0.2, vec3(0,1,0), texCoord, texSize);
        }

        // Draw circle.
        r32 radius = 1.5+0.3*sinf(world->time*8);
        Vec3 dirVec = vec3(radius*cosf(guy->orientation), radius*sinf(guy->orientation), 0.0);
        Vec3 circlePos = guy->pos;
        circlePos.z+=0.01;

        mesh->colorState=vec4(0.0, 0.0, 1.0, 1);
        PushRect3(mesh, circlePos, vec2(radius*2, radius*2), 
                vec3(0,0,1), vec3(0,1,0), circleCoord, circleTexSize);

        circlePos.z+=0.01;
        mesh->colorState=vec4(1.0, 0.0, 1.0, 1.0);
        PushLineArrow(mesh, circlePos, v3_add(circlePos, dirVec), vec3(0,0,1), texCoord, texSize, 0.2);
    }

}
