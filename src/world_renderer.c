
void
DrawGuys(Mesh *mesh, World *world, Vec2 texCoord, Vec2 texSize,
        Vec2 circleCoord, Vec2 circleTexSize)
{
    r32 time = world->time*32;
    for(int guyIdx = 0;
            guyIdx < world->nGuys;
            guyIdx++)
    {
        Guy *guy = world->guys + guyIdx;
        Body *body = guy->body;
        Vec3 headPos = guy->head->pos;

        r32 orientation = guy->orientation;
        r32 headSize = 0.5;
        r32 eyeSize = 0.25;
        r32 pupilSize = 0.15;
        r32 eyeDist = 0.3;
        Vec3 headNormal = vec3(cosf(orientation), sinf(orientation), 0.0);
        Vec3 headPerp = v3_cross(headNormal, vec3(0,0,1));
        Vec3 headCenterZ0 = v3_add(headPos, v3_muls(headNormal, 0.01));

        // Draw circle.
        r32 radius = 1.5+0.3*sinf(world->time*8);
        Vec3 dirVec = vec3(radius*cosf(guy->orientation), radius*sinf(guy->orientation), 0.0);
        Vec3 circlePos = guy->pos;
        circlePos.z+=0.001*((guyIdx%100)+1);

        r32 circleAlpha = 0.4;
        mesh->colorState=vec4(0.0, 0.0, 1.0, circleAlpha);
        PushRect3(mesh, circlePos, vec2(radius*2, radius*2), 
                vec3(0,0,1), vec3(0,1,0), circleCoord, circleTexSize);

        circlePos.z+=0.01;
        mesh->colorState=vec4(1.0, 0.0, 1.0, circleAlpha);
        PushLineArrow(mesh, circlePos, v3_add(circlePos, dirVec), vec3(0,0,1), texCoord, texSize, 0.2);

        mesh->colorState = vec4(1, 0, 0, 1);
        PushRect3(mesh, headPos, 
                vec2(headSize*2, headSize*2), headNormal, vec3(0,0,1), circleCoord, circleTexSize);
        mesh->colorState = vec4(1,1,1,1);
        Vec3 lEyePos = v3_add(
                headCenterZ0,
                v3_muls(headPerp, -eyeDist)
                );
        Vec3 rEyePos = v3_add(
                headCenterZ0,
                v3_muls(headPerp, eyeDist)
                );

        r32 eyeOpen = 0.5*(sinf(time)+1);
        PushRect3(mesh, lEyePos, 
                vec2(2*eyeSize, 2*eyeSize*eyeOpen), 
                headNormal, vec3(0,0,1), circleCoord, circleTexSize);
        mesh->colorState=vec4(0,0,0,1);
        lEyePos = v3_add(lEyePos, v3_muls(headNormal, 0.01));
        PushRect3(mesh, lEyePos, 
                vec2(2*pupilSize, 2*pupilSize*eyeOpen), headNormal, vec3(0,0,1), circleCoord, circleTexSize);

        mesh->colorState=vec4(1,1,1,1);
        PushRect3(mesh, rEyePos, 
                vec2(2*eyeSize, 2*eyeSize*eyeOpen), headNormal, vec3(0,0,1), circleCoord, circleTexSize);
        mesh->colorState=vec4(0,0,0,1);
        rEyePos = v3_add(rEyePos, v3_muls(headNormal, 0.01));
        PushRect3(mesh, rEyePos, 
                vec2(2*pupilSize, 2*pupilSize*eyeOpen), headNormal, vec3(0,0,1), circleCoord, circleTexSize);

        mesh->colorState = vec4(1, 0, 0, 1);
        for(int cIdx = 1; // Dont draw neck
                cIdx < body->nConstraints;
                cIdx++)
        {
            Constraint *c = body->constraints+cIdx;
            Vec3 from = c->a->pos;
            Vec3 to = c->b->pos;
            PushLine(mesh, from, to, 0.3, dirVec, texCoord, texSize);
        }
    }
}

internal void
DrawSwords(Mesh *mesh, World *world, Vec2 texCoord, Vec2 texSize,
        Vec2 circleCoord, Vec2 circleTexSize)
{
    for(int swordIdx = 0;
            swordIdx < world->nSwords;
            swordIdx++)
    {
        Sword *sword = world->swords+swordIdx;
        r32 lineWidth = 0.2;
        Vec3 from = sword->handle->pos;
        Vec3 to = sword->tip->pos;
        Vec3 diff = v3_norm(v3_sub(to, from));
        Vec3 perp = v3_norm(v3_cross(diff, vec3(0,0,1)));
        Vec3 handlePos = Lerp3(from, to, 0.2);
        Vec3 lEdge = v3_add(handlePos, v3_muls(perp, 0.4));
        Vec3 rEdge = v3_add(handlePos, v3_muls(perp, -0.4));

        mesh->colorState = vec4(0,0,0,1);
        PushLine(mesh, from, to, lineWidth, vec3(0,0,1), texCoord, texSize);
        PushLine(mesh, handlePos, lEdge, lineWidth, vec3(0,0,1), texCoord, texSize);
        PushLine(mesh, handlePos, rEdge, lineWidth, vec3(0,0,1), texCoord, texSize);
    }
}

