
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
        local_persist r32 r = 0.0;
        r+=0.01;
        mesh->colorState = vec3(0, 0, 0);
        PushRect3(mesh, headPos, vec2(1,1), vec3(cosf(r),sinf(r), 0), vec3(0,0,1), circleCoord, circleTexSize);

        for(int cIdx = 0;
                cIdx < body->nConstraints;
                cIdx++)
        {
            Constraint *c = body->constraints+cIdx;
            Vec3 from = c->a->pos;
            Vec3 to = c->b->pos;
            mesh->colorState = vec3(0,0,0);
            PushLine(mesh, from, to, 0.2, vec3(0,1,0), texCoord, texSize);
        }
    }
}
