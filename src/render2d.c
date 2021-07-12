
void
InitFontRenderer(MemoryArena *arena, 
        FontRenderer *fontRenderer, 
        const char *pathToFont, 
        r32 fontSize)
{
    ui32 w = 512;
    ui32 h = 512;
    ui32 rangeSize = 95;
    ui32 firstGlyph = 32;

    fontRenderer->atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, fontRenderer->atlas, rangeSize, w, h);

    stbtt_pack_context packContext;
    stbtt_pack_range range = {};
    ui8 *ttfBuffer = malloc(1<<20);
    ui8 *tmpBitmap = malloc(sizeof(ui8)*w*h);
    ui8 *fontTextureImage = malloc(sizeof(ui32)*w*h);

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
    ui32 imageSize = w*h;
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
CreateFrameBuffer(MemoryArena *arena, ui32 width, ui32 height)
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
InitSpriteBatch(SpriteBatch *batch, ui32 maxVertices, MemoryArena *arena)
{
    batch->nVertices = 0;
    batch->maxVertices = maxVertices;
    batch->stride = 8;  //pos(2), tex(2), col(4)
    batch->vertexBuffer = PushArray(arena, r32, batch->maxVertices*batch->stride);
    batch->indexBuffer = PushArray(arena, ui16, batch->maxVertices);

    glGenVertexArrays(1, &batch->vao);
    glGenBuffers(1, &batch->vbo);
    glGenBuffers(1, &batch->ebo);

    glBindVertexArray(batch->vao);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);

    size_t memOffset = 0;

    // Position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, batch->stride*sizeof(r32), (void *)(memOffset));
    memOffset+=2*sizeof(r32);

    // Texture coords
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, batch->stride*sizeof(r32), (void *)(memOffset));
    memOffset+=4*sizeof(r32);

    // Texture coords
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, batch->stride*sizeof(r32), (void *)(memOffset));
    memOffset+=2*sizeof(r32);
    CheckOpenglError();
}

internal inline void
BeginSpritebatch(SpriteBatch *batch)
{
    Assert(!batch->isDrawing);
    batch->isDrawing = 1;
    batch->nVertices = 0;
    batch->nIndices = 0;
}

internal inline void
EndSpritebatch(SpriteBatch *batch)
{
    Assert(batch->isDrawing);
    batch->isDrawing = 0;
    glBindVertexArray(batch->vao);
    glBindBuffer(GL_ARRAY_BUFFER, batch->vbo);
    glBufferData(GL_ARRAY_BUFFER, batch->nVertices*batch->stride*sizeof(r32), 
            batch->vertexBuffer, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, batch->ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, batch->nIndices*sizeof(ui16), 
            batch->indexBuffer, GL_DYNAMIC_DRAW);

    glDrawElements(GL_TRIANGLES, batch->nIndices, GL_UNSIGNED_SHORT, 0);
}

internal inline void
PushVertex2(SpriteBatch *batch, Vec2 pos, Vec2 texCoord, Vec4 color)
{
    ui32 bufferIdx = batch->nVertices*batch->stride;

    batch->vertexBuffer[bufferIdx] = pos.x;
    batch->vertexBuffer[bufferIdx+1] = pos.y;
    batch->vertexBuffer[bufferIdx+2] = color.x;
    batch->vertexBuffer[bufferIdx+3] = color.y;
    batch->vertexBuffer[bufferIdx+4] = color.z;
    batch->vertexBuffer[bufferIdx+5] = color.w;
    batch->vertexBuffer[bufferIdx+6] = texCoord.x;
    batch->vertexBuffer[bufferIdx+7] = texCoord.y;

    batch->nVertices++;
    Assert(batch->nIndices < batch->maxVertices);
}

internal inline void
PushIndex(SpriteBatch *batch, ui16 index)
{
    batch->indexBuffer[batch->nIndices++] = index;
}

void
PushQuad2(SpriteBatch *batch, Vec2 p0, Vec2 p1, Vec2 p2, Vec2 p3, Vec2 texOrig, Vec2 texSize)
{
    ui16 lastIdx = batch->nVertices;
    PushVertex2(batch, p0, vec2(texOrig.x, texOrig.y+texSize.y), batch->colorState);
    PushVertex2(batch, p1, vec2(texOrig.x+texSize.x, texOrig.y+texSize.y), batch->colorState);
    PushVertex2(batch, p2, vec2(texOrig.x+texSize.x, texOrig.y), batch->colorState);
    PushVertex2(batch, p3, texOrig, batch->colorState);
    PushIndex(batch, lastIdx);
    PushIndex(batch, lastIdx+1);
    PushIndex(batch, lastIdx+2);
    PushIndex(batch, lastIdx+2);
    PushIndex(batch, lastIdx+3);
    PushIndex(batch, lastIdx);
    Assert(batch->nIndices < batch->maxVertices);
}

void
PushSemiCircle2(SpriteBatch *batch, 
        Vec2 pos, 
        r32 radius, 
        r32 minAngle, 
        r32 maxAngle, 
        ui32 nPoints, 
        AtlasRegion *tex)
{
    ui16 lastIdx = batch->nVertices;
    PushVertex2(batch, pos, tex->pos, batch->colorState);
    r32 angDiff = (maxAngle-minAngle)/(nPoints-1);
    for(ui32 atPoint = 0;
            atPoint < nPoints;
            atPoint++)
    {
        r32 angle = minAngle+angDiff*atPoint;
        Vec2 point = v2_add(pos, v2_polar(angle, radius));
        PushVertex2(batch, point, tex->pos, batch->colorState); 
    }
    for(ui32 atPoint = 0;
            atPoint < nPoints-1;
            atPoint++)
    {
        PushIndex(batch, lastIdx);
        PushIndex(batch, lastIdx+atPoint+1);
        PushIndex(batch, lastIdx+atPoint+2);
    }
}

void
PushLineCircle2(SpriteBatch *batch,
        Vec2 center,
        r32 radius,
        r32 lineWidth,
        int nPoints,
        AtlasRegion *tex)
{
    ui16 lastIdx = batch->nVertices;
    r32 innerRadius = radius-lineWidth/2.0;
    r32 outerRadius = radius+lineWidth/2.0;
    r32 dAngle = 2*M_PI/nPoints;

    for(int atPoint = 0;
            atPoint < nPoints;
            atPoint++)
    {
        r32 angle = atPoint * dAngle;
        r32 c = cosf(angle);
        r32 s = sinf(angle);
        PushVertex2(batch, 
                vec2(center.x+c*innerRadius, center.y+s*innerRadius),
                tex->pos, 
                batch->colorState);
        PushVertex2(batch, 
                vec2(center.x+c*outerRadius, center.y+s*outerRadius),
                tex->pos, 
                batch->colorState);
    }
    for(int atPoint = 0;
            atPoint < nPoints-1;
            atPoint++)
    {
        PushIndex(batch, lastIdx+atPoint*2);
        PushIndex(batch, lastIdx+atPoint*2+1);
        PushIndex(batch, lastIdx+atPoint*2+3);
        PushIndex(batch, lastIdx+atPoint*2+3);
        PushIndex(batch, lastIdx+atPoint*2+2);
        PushIndex(batch, lastIdx+atPoint*2);
    }
    PushIndex(batch, lastIdx+nPoints*2-2);
    PushIndex(batch, lastIdx+nPoints*2-1);
    PushIndex(batch, lastIdx+1);
    PushIndex(batch, lastIdx+1);
    PushIndex(batch, lastIdx);
    PushIndex(batch, lastIdx+nPoints*2-2);
}

void
PushRect2(SpriteBatch *batch, Vec2 orig, Vec2 size, Vec2 texOrig, Vec2 texSize)
{
    PushQuad2(batch, 
            orig, 
            vec2(orig.x+size.x, orig.y), 
            vec2(orig.x+size.x, orig.y+size.y),
            vec2(orig.x, orig.y+size.y),
            texOrig, 
            texSize);
}

void
PushLineRect2(SpriteBatch *batch, Vec2 origin, Vec2 size, Vec2 texOrig, Vec2 texSize, r32 lineWidth)
{
    PushRect2(batch, origin, vec2(size.x, lineWidth), texOrig, texSize);
    PushRect2(batch, origin, vec2(lineWidth, size.y), texOrig, texSize);
    PushRect2(batch, vec2(origin.x+size.x-lineWidth, origin.y), vec2(lineWidth, size.y), texOrig, texSize);
    PushRect2(batch, vec2(origin.x, origin.y+size.y-lineWidth), vec2(size.x, lineWidth), texOrig, texSize);
}

void
PushLine2(SpriteBatch *batch, Vec2 from, Vec2 to, r32 lineWidth, Vec2 texOrig, Vec2 texSize)
{
    Vec2 diff = v2_sub(to, from);
    r32 invl = 1.0/v2_length(diff);
    Vec2 perp = vec2(diff.y*invl*lineWidth/2.0, -diff.x*invl*lineWidth/2.0);
    PushQuad2(batch,
            v2_add(from, perp), v2_add(to, perp),
            v2_sub(to, perp), v2_sub(from, perp),
            texOrig,
            texSize);
}

void
PushOrientedRectangle2(SpriteBatch *batch, 
        Vec2 pos, 
        r32 width, 
        r32 height, 
        r32 angle,
        AtlasRegion *texture)
{
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    Vec2 axis0 = vec2(c*width/2.0, s*width/2.0);
    Vec2 axis1 = vec2(s*height/2.0, -c*height/2.0);
    Vec2 p00 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, -1)));
    Vec2 p10 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, -1)));
    Vec2 p11 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, 1)));
    Vec2 p01 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, 1)));

    PushQuad2(batch, p00, p10, p11, p01, texture->pos, texture->size);
}

void
PushOrientedLineRectangle2(SpriteBatch *batch, 
        Vec2 pos, 
        r32 width, 
        r32 height, 
        r32 angle,
        r32 lineWidth,
        AtlasRegion *texture)
{
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    Vec2 axis0 = vec2(c*width/2.0, s*width/2.0);
    Vec2 axis1 = vec2(s*height/2.0, -c*height/2.0);
    Vec2 p00 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, -1)));
    Vec2 p10 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, -1)));
    Vec2 p11 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, 1)));
    Vec2 p01 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, 1)));

    PushLine2(batch, p00, p10, lineWidth, texture->pos, texture->size);
    PushLine2(batch, p10, p11, lineWidth, texture->pos, texture->size);
    PushLine2(batch, p11, p01, lineWidth, texture->pos, texture->size);
    PushLine2(batch, p01, p00, lineWidth, texture->pos, texture->size);
}

void
PushCircle2(SpriteBatch *batch, Vec2 center, r32 size, AtlasRegion *tex)
{
    PushRect2(batch, vec2(center.x-size, center.y-size), vec2(size*2, size*2), tex->pos, tex->size);
}

internal inline Vec2
Lerp2(Vec2 from, Vec2 to, r32 lambda)
{
    return v2_add(v2_muls(from, 1.0-lambda), v2_muls(to, lambda));
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
    r32 x = pos.x;
    r32 y = pos.y;
    r32 minY = 10000.0;
    r32 maxY = -10000.0;
    ui32 w = fontRenderer->atlas->width;
    ui32 h = fontRenderer->atlas->height;

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
DrawString2D(SpriteBatch *batch, FontRenderer *fontRenderer, Vec2 pos, char *sequence)
{
    stbtt_aligned_quad q;
    r32 x = pos.x;
    r32 y = pos.y;
    ui32 w = fontRenderer->atlas->width;
    ui32 h = fontRenderer->atlas->height;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData, w, h, *sequence-32, &x, &y, &q, 0);
        PushRect2(batch, vec2(q.x0, q.y0), vec2(q.x1-q.x0, q.y1-q.y0), 
                vec2(q.s0, q.t1), vec2(q.s1-q.s0, q.t0-q.t1));
        sequence++;
    }
}

internal inline void
DrawDirectRect(SpriteBatch *batch, 
        ShaderInstance *shaderInstance,
        Vec2 pos, 
        Vec2 dims, 
        ui32 textureHandle, 
        Vec2 uvPos, 
        Vec2 uvDims,
        Vec4 color)
{
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BeginSpritebatch(batch);
    BeginShaderInstance(shaderInstance);

    batch->colorState = color;
    glBindTexture(GL_TEXTURE_2D, textureHandle);

    PushRect2(batch,
            pos,
            dims,
            uvPos,
            uvDims);

    EndSpritebatch(batch);

}

