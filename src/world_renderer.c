
void
DrawGuys(Mesh *mesh, World *world)
{
    for(int guyIdx = 0;
            guyIdx < world->nGuys;
            guyIdx++)
    {
        Guy *guy = world->guys + guyIdx;
        Body *body = guy->body;

        for(int cIdx = 0;
                cIdx < body->nConstraints;
                cIdx++)
        {
            Constraint *c = body->constraints+cIdx;
            Vec3 from = c->a->pos;
            Vec3 to = c->b->pos;
            mesh->colorState = vec3(0,0,0);
            PushLine(mesh, from, to, 0.2, vec3(0,1,0));
        }
    }
}
