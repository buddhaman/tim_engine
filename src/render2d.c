
void
InitFontRenderer(MemoryArena *arena, 
        FontRenderer *fontRenderer, 
        const char *pathToFont, 
        R32 fontSize)
{
    U32 w = 512;
    U32 h = 512;
    U32 rangeSize = 95;
    U32 firstGlyph = 32;

    fontRenderer->atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, fontRenderer->atlas, rangeSize, w, h);

    stbtt_pack_context packContext;
    stbtt_pack_range range = {};
    U8 *ttfBuffer = malloc(1<<20);
    U8 *tmpBitmap = malloc(sizeof(U8)*w*h);
    U8 *fontTextureImage = malloc(sizeof(U32)*w*h);

    FILE *handle = fopen(pathToFont, "rb");
    if(handle)
    {
        fread(ttfBuffer, 1, 1<<20, handle);
        fclose(handle);
    }
    else
    {
        DebugOut("ERROR: Cant open font file: %s.", pathToFont);
    }

    range.font_size = fontSize;
    range.first_unicode_codepoint_in_range = firstGlyph;
    range.num_chars = rangeSize;
    range.chardata_for_range = fontRenderer->charData;

    stbtt_PackBegin(&packContext, tmpBitmap, w, h, 0, 1, NULL);
    stbtt_PackSetOversampling(&packContext, 1, 1);
    stbtt_PackFontRanges(&packContext, ttfBuffer, 0, &range, 1);
    stbtt_PackEnd(&packContext);

    // Turn bitmap into rgba image
    U32 imageSize = w*h;
    for(int idx = 0; idx < imageSize; idx++)
    {
        fontTextureImage[idx*4+0] = 255;
        fontTextureImage[idx*4+1] = 255;
        fontTextureImage[idx*4+2] = 255;
        fontTextureImage[idx*4+3] = tmpBitmap[idx];
    }

    glGenTextures(1, &fontRenderer->atlas->textureHandle);
    glBindTexture(GL_TEXTURE_2D, fontRenderer->atlas->textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 
            w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, fontTextureImage);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    free(ttfBuffer);
    free(tmpBitmap);
    free(fontTextureImage);
}

FrameBuffer *
CreateFrameBuffer(MemoryArena *arena, U32 width, U32 height)
{
    FrameBuffer *frameBuffer = PushStruct(arena, FrameBuffer);
    frameBuffer->width = width;
    frameBuffer->height = height;
    glGenFramebuffers(1, &frameBuffer->fbo);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer->fbo);

    glGenTextures(1, &frameBuffer->colorTexture);
    glBindTexture(GL_TEXTURE_2D, frameBuffer->colorTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glFramebufferTexture2D(GL_FRAMEBUFFER, 
            GL_COLOR_ATTACHMENT0, 
            GL_TEXTURE_2D, 
            frameBuffer->colorTexture, 
            0);

    if(glCheckFramebufferStatus(GL_FRAMEBUFFER)==GL_FRAMEBUFFER_COMPLETE)
    {
        DebugOut("Framebuffer succesfully created!!!");
    }
    else
    {
        DebugOut("Framebuffer creation failed.");
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return frameBuffer;
}

void
InitMesh2D(Mesh2D *mesh, U32 maxVertices, MemoryArena *arena)
{
    mesh->nVertices = 0;
    mesh->maxVertices = maxVertices;
    mesh->stride = 5;  //pos(2), tex(2), col(1)
    mesh->vertexBuffer = PushArray(arena, R32, mesh->maxVertices*mesh->stride);
    mesh->indexBuffer = PushArray(arena, U16, mesh->maxVertices);

    glGenVertexArrays(1, &mesh->vao);
    glGenBuffers(1, &mesh->vbo);
    glGenBuffers(1, &mesh->ebo);

    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);

    size_t memOffset = 0;

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, mesh->stride*sizeof(R32), (void *)(memOffset));
    memOffset+=2*sizeof(R32);

    // Color
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_UNSIGNED_BYTE, GL_TRUE, mesh->stride*sizeof(R32), (void *)(memOffset));
    memOffset+=1*sizeof(R32);

    // Texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, mesh->stride*sizeof(R32), (void *)(memOffset));
    memOffset+=2*sizeof(R32);
    //CheckOpenglError();
}

internal inline void
BeginMesh2D(Mesh2D *mesh)
{
    Assert(!mesh->isDrawing);
    mesh->isDrawing = 1;
    mesh->nVertices = 0;
    mesh->nIndices = 0;
}

internal inline void
EndMesh2D(Mesh2D *mesh)
{
    Assert(mesh->isDrawing);
    mesh->isDrawing = 0;
    glBindVertexArray(mesh->vao);
    glBindBuffer(GL_ARRAY_BUFFER, mesh->vbo);
    glBufferData(GL_ARRAY_BUFFER, mesh->nVertices*mesh->stride*sizeof(R32), 
            mesh->vertexBuffer, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->nIndices*sizeof(U16), 
            mesh->indexBuffer, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, mesh->nIndices, GL_UNSIGNED_SHORT, 0);
}

internal inline void
PushVertex2(Mesh2D *mesh, Vec2 pos, Vec2 texCoord, Vec4 color)
{
    U32 bufferIdx = mesh->nVertices*mesh->stride;

    mesh->vertexBuffer[bufferIdx] = pos.x;
    mesh->vertexBuffer[bufferIdx+1] = pos.y;
    U32 colorBits = Vec4ToRGBA(color);
    R32 colorBitsFloat = *((R32 *)(&colorBits));
    mesh->vertexBuffer[bufferIdx+2] = colorBitsFloat;
    mesh->vertexBuffer[bufferIdx+3] = texCoord.x;
    mesh->vertexBuffer[bufferIdx+4] = texCoord.y;

    mesh->nVertices++;
    Assert(mesh->nIndices < mesh->maxVertices);
}

internal inline void
PushIndex(Mesh2D *mesh, U16 index)
{
    mesh->indexBuffer[mesh->nIndices++] = index;
}

void
PushQuad2(Mesh2D *mesh, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 texOrig, Vec2 texSize)
{
    U16 lastIdx = mesh->nVertices;
    PushVertex2(mesh, p0, V2(texOrig.x, texOrig.y+texSize.y), mesh->colorState);
    PushVertex2(mesh, p1, V2(texOrig.x+texSize.x, texOrig.y+texSize.y), mesh->colorState);
    PushVertex2(mesh, p2, V2(texOrig.x+texSize.x, texOrig.y), mesh->colorState);
    PushVertex2(mesh, p3, texOrig, mesh->colorState);
    PushIndex(mesh, lastIdx);
    PushIndex(mesh, lastIdx+1);
    PushIndex(mesh, lastIdx+2);
    PushIndex(mesh, lastIdx+2);
    PushIndex(mesh, lastIdx+3);
    PushIndex(mesh, lastIdx);
    Assert(mesh->nIndices < mesh->maxVertices);
}

void
PushSemiCircle2(Mesh2D *mesh, 
        Vec2 pos, 
        R32 radius, 
        R32 minAngle, 
        R32 maxAngle, 
        U32 nPoints, 
        AtlasRegion *tex)
{
    U16 lastIdx = mesh->nVertices;
    PushVertex2(mesh, pos, tex->pos, mesh->colorState);
    R32 angDiff = (maxAngle-minAngle)/(nPoints-1);
    for(U32 atPoint = 0;
            atPoint < nPoints;
            atPoint++)
    {
        R32 angle = minAngle+angDiff*atPoint;
        Vec2 point = V2Add(pos, V2Polar(angle, radius));
        PushVertex2(mesh, point, tex->pos, mesh->colorState); 
    }
    for(U32 atPoint = 0;
            atPoint < nPoints-1;
            atPoint++)
    {
        PushIndex(mesh, lastIdx);
        PushIndex(mesh, lastIdx+atPoint+1);
        PushIndex(mesh, lastIdx+atPoint+2);
    }
}

void
PushLineCircle2(Mesh2D *mesh,
        Vec2 center,
        R32 radius,
        R32 lineWidth,
        int nPoints,
        AtlasRegion *tex)
{
    U16 lastIdx = mesh->nVertices;
    R32 innerRadius = radius-lineWidth/2.0;
    R32 outerRadius = radius+lineWidth/2.0;
    R32 dAngle = 2*M_PI/nPoints;

    for(int atPoint = 0;
            atPoint < nPoints;
            atPoint++)
    {
        R32 angle = atPoint * dAngle;
        R32 c = cosf(angle);
        R32 s = sinf(angle);
        PushVertex2(mesh, 
                V2(center.x+c*innerRadius, center.y+s*innerRadius),
                tex->pos, 
                mesh->colorState);
        PushVertex2(mesh, 
                V2(center.x+c*outerRadius, center.y+s*outerRadius),
                tex->pos, 
                mesh->colorState);
    }
    for(int atPoint = 0;
            atPoint < nPoints-1;
            atPoint++)
    {
        PushIndex(mesh, lastIdx+atPoint*2);
        PushIndex(mesh, lastIdx+atPoint*2+1);
        PushIndex(mesh, lastIdx+atPoint*2+3);
        PushIndex(mesh, lastIdx+atPoint*2+3);
        PushIndex(mesh, lastIdx+atPoint*2+2);
        PushIndex(mesh, lastIdx+atPoint*2);
    }
    PushIndex(mesh, lastIdx+nPoints*2-2);
    PushIndex(mesh, lastIdx+nPoints*2-1);
    PushIndex(mesh, lastIdx+1);
    PushIndex(mesh, lastIdx+1);
    PushIndex(mesh, lastIdx);
    PushIndex(mesh, lastIdx+nPoints*2-2);
}

void
PushRect2(Mesh2D *mesh, Vec2 orig, Vec2 size, Vec2 texOrig, Vec2 texSize)
{
    PushQuad2(mesh, 
            orig, 
            V2(orig.x+size.x, orig.y), 
            V2(orig.x+size.x, orig.y+size.y),
            V2(orig.x, orig.y+size.y),
            texOrig, 
            texSize);
}

void
PushLineRect2(Mesh2D *mesh, Vec2 origin, Vec2 size, Vec2 texOrig, Vec2 texSize, R32 lineWidth)
{
    PushRect2(mesh, origin, V2(size.x, lineWidth), texOrig, texSize);
    PushRect2(mesh, origin, V2(lineWidth, size.y), texOrig, texSize);
    PushRect2(mesh, V2(origin.x+size.x-lineWidth, origin.y), V2(lineWidth, size.y), texOrig, texSize);
    PushRect2(mesh, V2(origin.x, origin.y+size.y-lineWidth), V2(size.x, lineWidth), texOrig, texSize);
}

void
PushLine2(Mesh2D *mesh, Vec2 from, Vec2 to, R32 lineWidth, Vec2 texOrig, Vec2 texSize)
{
    Vec2 diff = V2Sub(to, from);
    R32 invl = 1.0/V2Len(diff);
    Vec2 perp = V2(diff.y*invl*lineWidth/2.0, -diff.x*invl*lineWidth/2.0);
    PushQuad2(mesh,
            V2Add(from, perp), V2Add(to, perp),
            V2Sub(to, perp), V2Sub(from, perp),
            texOrig,
            texSize);
}

void
PushOrientedRectangle2(Mesh2D *mesh, 
        Vec2 pos, 
        R32 width, 
        R32 height, 
        R32 angle,
        AtlasRegion *texture)
{
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    Vec2 axis0 = V2(c*width/2.0, s*width/2.0);
    Vec2 axis1 = V2(s*height/2.0, -c*height/2.0);
    Vec2 p00 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, -1)));
    Vec2 p10 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, -1)));
    Vec2 p11 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, 1)));
    Vec2 p01 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, 1)));

    PushQuad2(mesh, p00, p10, p11, p01, texture->pos, texture->size);
}

void
PushOrientedLineRectangle2(Mesh2D *mesh, 
        Vec2 pos, 
        R32 width, 
        R32 height, 
        R32 angle,
        R32 lineWidth,
        AtlasRegion *texture)
{
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    Vec2 axis0 = V2(c*width/2.0, s*width/2.0);
    Vec2 axis1 = V2(s*height/2.0, -c*height/2.0);
    Vec2 p00 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, -1)));
    Vec2 p10 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, -1)));
    Vec2 p11 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, 1)));
    Vec2 p01 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, 1)));

    PushLine2(mesh, p00, p10, lineWidth, texture->pos, texture->size);
    PushLine2(mesh, p10, p11, lineWidth, texture->pos, texture->size);
    PushLine2(mesh, p11, p01, lineWidth, texture->pos, texture->size);
    PushLine2(mesh, p01, p00, lineWidth, texture->pos, texture->size);
}

void
PushCircle2(Mesh2D *mesh, Vec2 center, R32 size, AtlasRegion *tex)
{
    PushRect2(mesh, V2(center.x-size, center.y-size), V2(size*2, size*2), tex->pos, tex->size);
}

internal inline Vec2
Lerp2(Vec2 from, Vec2 to, R32 lambda)
{
    return V2Add(V2MulS(from, 1.0-lambda), V2MulS(to, lambda));
}

// Stencil Buffer Uitls

// Dont forget to enable stencil testing.
void
BeginStencilShape()
{
    glStencilMask(0xFF);
    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glDepthMask(GL_FALSE);
    glStencilOp(GL_REPLACE, GL_REPLACE, GL_REPLACE);
}

void 
EndStencilShape()
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glStencilMask(0x00);
}

Rect2
GetStringSize(FontRenderer *fontRenderer, char *sequence, Vec2 pos)
{
    stbtt_aligned_quad q;
    R32 x = pos.x;
    R32 y = pos.y;
    R32 minY = 10000.0;
    R32 maxY = -10000.0;
    U32 w = fontRenderer->atlas->width;
    U32 h = fontRenderer->atlas->height;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData, w, h, *sequence-32, &x, &y, &q, 0);
        minY = Min(minY, q.y0);
        maxY = Max(maxY, q.y1);
        sequence++;
    }
    return (Rect2){.x = pos.x, .y = minY, .width = x-pos.x, .height = maxY-minY};
}

void
DrawString2D(Mesh2D *mesh, FontRenderer *fontRenderer, Vec2 pos, char *sequence)
{
    stbtt_aligned_quad q;
    R32 x = pos.x;
    R32 y = pos.y;
    U32 w = fontRenderer->atlas->width;
    U32 h = fontRenderer->atlas->height;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData, w, h, *sequence-32, &x, &y, &q, 0);
        PushRect2(mesh, V2(q.x0, q.y0), V2(q.x1-q.x0, q.y1-q.y0), 
                V2(q.s0, q.t1), V2(q.s1-q.s0, q.t0-q.t1));
        sequence++;
    }
}

internal inline void
DrawDirectRect(Mesh2D *mesh, 
        ShaderInstance *shaderInstance,
        Vec2 pos, 
        Vec2 dims, 
        U32 textureHandle, 
        Vec2 uvPos, 
        Vec2 uvDims,
        Vec4 color)
{
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BeginMesh2D(mesh);
    BeginShaderInstance(shaderInstance);

    mesh->colorState = color;
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    PushRect2(mesh,
            pos,
            dims,
            uvPos,
            uvDims);

    EndMesh2D(mesh);

}

