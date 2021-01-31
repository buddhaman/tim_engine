
internal inline void
PushVertex2(Mesh *mesh, Vec2 vert, Vec2 texCoord)
{
    mesh->vertices2[mesh->nVertices] = vert;
    mesh->texCoords[mesh->nVertices] = texCoord;
    mesh->colors[mesh->nVertices] = mesh->colorState;
    mesh->nVertices++;
    Assert(mesh->nVertices <= mesh->maxVertices);
}

void
PushQuad2(Mesh *mesh, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 texOrig, Vec2 texSize)
{
    ui32 nVertices = mesh->nVertices;
    PushVertex2(mesh, p0, vec2(texOrig.x, texOrig.y+texSize.y));
    PushVertex2(mesh, p1, vec2(texOrig.x+texSize.x, texOrig.y+texSize.y));
    PushVertex2(mesh, p2, vec2(texOrig.x+texSize.x, texOrig.y));
    PushVertex2(mesh, p3, texOrig);
    PushIndex(mesh, nVertices);
    PushIndex(mesh, nVertices+1);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+2);
    PushIndex(mesh, nVertices+3);
    PushIndex(mesh, nVertices);
}

void
PushRect2(Mesh *mesh, Vec2 orig, Vec2 size, Vec2 texOrig, Vec2 texSize)
{
    PushQuad2(mesh, 
            orig, 
            vec2(orig.x+size.x, orig.y), 
            vec2(orig.x+size.x, orig.y+size.y),
            vec2(orig.x, orig.y+size.y),
            texOrig, 
            texSize);
}

void
PushLineRect2(Mesh *mesh, Vec2 origin, Vec2 size, Vec2 texOrig, Vec2 texSize, r32 lineWidth)
{
    PushRect2(mesh, origin, vec2(size.x, lineWidth), texOrig, texSize);
    PushRect2(mesh, origin, vec2(lineWidth, size.y), texOrig, texSize);
    PushRect2(mesh, vec2(origin.x+size.x-lineWidth, origin.y), vec2(lineWidth, size.y), texOrig, texSize);
    PushRect2(mesh, vec2(origin.x, origin.y+size.y-lineWidth), vec2(size.x, lineWidth), texOrig, texSize);
}

void
PushLine2(Mesh *mesh, Vec2 from, Vec2 to, r32 lineWidth, Vec2 texOrig, Vec2 texSize)
{
    Vec2 diff = v2_sub(to, from);
    r32 invl = 1.0/v2_length(diff);
    Vec2 perp = vec2(diff.y*invl*lineWidth/2.0, -diff.x*invl*lineWidth/2.0);
    PushQuad2(mesh,
            v2_add(from, perp), v2_add(to, perp),
            v2_sub(to, perp), v2_sub(from, perp),
            texOrig,
            texSize);
}

void
PushCircle2(Mesh *mesh, Vec2 center, r32 size, Vec2 texOrig, Vec2 texSize)
{
    PushRect2(mesh, vec2(center.x-size, center.y-size), vec2(size*2, size*2), texOrig, texSize);
}

internal inline Vec2
Lerp2(Vec2 from, Vec2 to, r32 lambda)
{
    return v2_add(v2_muls(from, 1.0-lambda), v2_muls(to, lambda));
}

Vec2
GetStringSize(FontRenderer *fontRenderer, char *sequence)
{
    stbtt_aligned_quad q;
    r32 x = 0.0;
    r32 y = 0.0;
    r32 minY = 10000.0;
    r32 maxY = -10000.0;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData12, 512, 512, *sequence-32, &x, &y, &q, 0);
        minY = Min(minY, q.y0);
        maxY = Max(maxY, q.y1);
        sequence++;
    }
    return vec2(x, maxY-minY);
}

void
DrawString2D(Mesh *mesh, FontRenderer *fontRenderer, Vec2 pos, char *sequence)
{
    stbtt_aligned_quad q;
    r32 x = pos.x;
    r32 y = pos.y;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData12, 512, 512, *sequence-32, &x, &y, &q, 0);
        PushRect2(mesh, vec2(q.x0, q.y0), vec2(q.x1-q.x0, q.y1-q.y0), 
                vec2(q.s0, q.t1), vec2(q.s1-q.s0, q.t0-q.t1));
        sequence++;
    }
}

