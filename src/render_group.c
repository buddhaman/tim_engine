
void
FlushRenderGroup(RenderGroup *renderGroup)
{
    renderGroup->nCommands = 0;
}

void 
ExecuteRenderGroup(RenderGroup *renderGroup, Assets *assets, ShaderInstance *shaderInstance)
{

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    BeginShaderInstance(shaderInstance);

    Mesh2D *batch = assets->batch;
    BeginMesh2D(batch);
    U32 currentTextureHandle = 0;

    for(U32 commandIdx = 0;
            commandIdx < renderGroup->nCommands;
            commandIdx++)
    {
        RenderCommand *command = renderGroup->commands+commandIdx;

        // TODO: Remove this code by sorting and inserting state changes.
        U32 handle = command->textureHandle;
        if(currentTextureHandle!=handle)
        {
            EndMesh2D(batch);
            BeginMesh2D(batch);
            glBindTexture(GL_TEXTURE_2D, handle);
            currentTextureHandle = handle;
        }

        switch(command->type)
        {

        case RENDER_2D_RECT:
        {
            batch->colorState = command->color;
            PushRect2(batch,
                    command->pos,
                    command->dims,
                    command->uvPos,
                    command->uvDims);
        } break;

        case RENDER_2D_ORIENTED_RECT:
        {
            batch->colorState = command->color;
            AtlasRegion region;
            region.pos = command->uvPos;
            region.size = command->uvDims;
            PushOrientedRectangle2(batch,
                    command->pos,
                    command->dims.x,
                    command->dims.y,
                    command->angle,
                    &region);
        } break;

        case RENDER_2D_LINE:
        {
            batch->colorState = command->color;
            PushLine2(batch,
                    command->from,
                    command->to,
                    command->lineWidth,
                    command->uvPos,
                    command->uvDims);
        } break;

        case RENDER_2D_SEMICIRCLE:
        {
            batch->colorState = command->color;
            AtlasRegion region;
            region.pos = command->uvPos;
            region.size = command->uvDims;
            PushSemiCircle2(batch,
                    command->pos,
                    command->radius,
                    command->range.x,
                    command->range.y,
                    command->nPoints,
                    &region);
        } break;

        case RENDER_2D_LINE_CIRCLE:
        {
            batch->colorState = command->color;
            AtlasRegion region;
            region.pos = command->uvPos;
            region.size = command->uvDims;
            PushLineCircle2(batch,
                    command->pos,
                    command->radius,
                    command->lineWidth,
                    command->nPoints,
                    &region);
        } break;

        default:
        {
            DebugOut("RENDERCOMMAND NOT IMPLEMENTED YET");
        } break;

        }
    }
    EndMesh2D(batch);
}

void 
ExecuteAndFlushRenderGroup(RenderGroup *renderGroup, Assets *assets, ShaderInstance *shaderInstance)
{
    ExecuteRenderGroup(renderGroup, assets, shaderInstance);
    FlushRenderGroup(renderGroup);
}

internal inline RenderCommand*
PushRenderCommand(RenderGroup *renderGroup, RenderCommandType type)
{
    Assert(renderGroup->nCommands+1 < renderGroup->maxCommands);
    RenderCommand *command = renderGroup->commands+renderGroup->nCommands++;
    command->type = type;
    return command;
}

internal inline void
Push2DTexRectColored(RenderGroup* renderGroup,
        Vec2 pos,
        Vec2 dims,
        U32 textureHandle,
        Vec2 uvPos,
        Vec2 uvDims,
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_RECT);
    command->pos = pos;
    command->dims = dims;
    command->textureHandle = textureHandle,
    command->uvPos = uvPos;
    command->uvDims = uvDims;
    command->color = color;
}

internal inline void
Push2DRectColored(RenderGroup* renderGroup,
        Vec2 pos,
        Vec2 dims,
        AtlasRegion *texture,
        Vec4 color)
{
    Push2DTexRectColored(renderGroup,
            pos,
            dims, 
            texture->atlas->textureHandle,
            texture->pos,
            texture->size, 
            color);
}

internal inline void
Push2DCircleColored(RenderGroup* renderGroup,
        Vec2 center,
        R32 radius,
        AtlasRegion *texture,
        Vec4 color)
{
    Push2DTexRectColored(renderGroup,
            V2(center.x-radius, center.y-radius),
            V2(radius*2, radius*2),
            texture->atlas->textureHandle,
            texture->pos,
            texture->size, 
            color);
}

internal inline void
Push2DTexLineColored(RenderGroup *renderGroup,
        Vec2 from,
        Vec2 to,
        R32 lineWidth,
        U32 textureHandle,
        Vec2 uvPos,
        Vec2 uvDims, 
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_LINE);
    command->from = from;
    command->to = to;
    command->lineWidth = lineWidth;
    command->textureHandle = textureHandle;
    command->uvPos = uvPos;
    command->uvDims = uvDims;
    command->color = color;
}

internal inline void
Push2DLineColored(RenderGroup *renderGroup,
        Vec2 from,
        Vec2 to,
        R32 lineWidth,
        AtlasRegion *texture,
        Vec4 color)
{
    Push2DTexLineColored(renderGroup,
            from,
            to,
            lineWidth,
            texture->atlas->textureHandle,
            texture->pos,
            texture->size,
            color);
}

internal inline void
Push2DTexOrientedRectangleColored(RenderGroup *renderGroup,
        Vec2 pos,
        Vec2 dims,
        R32 angle,
        U32 textureHandle,
        Vec2 uvPos,
        Vec2 uvDims,
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_ORIENTED_RECT);
    command->pos = pos;
    command->dims = dims;
    command->angle = angle;
    command->textureHandle = textureHandle;
    command->uvPos = uvPos;
    command->uvDims = uvDims;
    command->color = color;
}

internal inline void
Push2DOrientedRectangleColored(RenderGroup *renderGroup,
        Vec2 pos,
        Vec2 dims,
        R32 angle,
        AtlasRegion *texture,
        Vec4 color)
{
    Push2DTexOrientedRectangleColored(renderGroup,
            pos,
            dims, 
            angle,
            texture->atlas->textureHandle,
            texture->pos,
            texture->size,
            color);
}

internal inline void 
Push2DOrientedLineRectangleColored(RenderGroup *renderGroup,
        Vec2 pos, 
        Vec2 dims, 
        R32 angle, 
        R32 lineWidth,
        AtlasRegion *texture,
        Vec4 color)
{
    R32 c = cosf(angle);
    R32 s = sinf(angle);
    Vec2 axis0 = V2(c*dims.x/2.0, s*dims.x/2.0);
    Vec2 axis1 = V2(s*dims.y/2.0, -c*dims.y/2.0);
    Vec2 p00 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, -1)));
    Vec2 p10 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, -1)));
    Vec2 p11 = V2Add(pos, V2Add(V2MulS(axis0, 1), V2MulS(axis1, 1)));
    Vec2 p01 = V2Add(pos, V2Add(V2MulS(axis0, -1), V2MulS(axis1, 1)));
    Push2DLineColored(renderGroup, p00, p10, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p10, p11, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p11, p01, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p01, p00, lineWidth, texture, color);
}


internal inline void
Push2DTextColored(RenderGroup *renderGroup, 
        FontRenderer *fontRenderer,
        Vec2 pos, 
        char *sequence,
        Vec4 color)
{
    stbtt_aligned_quad q;
    R32 x = pos.x;
    R32 y = pos.y;
    U32 w = fontRenderer->atlas->width;
    U32 h = fontRenderer->atlas->height;
    U32 textureHandle = fontRenderer->atlas->textureHandle;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData, w, h, *sequence-32, &x, &y, &q, 0);
        Push2DTexRectColored(renderGroup, 
                V2(q.x0, q.y0), 
                V2(q.x1-q.x0, q.y1-q.y0), 
                textureHandle, 
                V2(q.s0, q.t1), 
                V2(q.s1-q.s0, q.t0-q.t1), 
                color);
        sequence++;
    }
}

internal inline void
Push2DText(RenderGroup *renderGroup, 
        FontRenderer *fontRenderer,
        Vec2 pos, 
        char *sequence)
{
    Push2DTextColored(renderGroup,
            fontRenderer,
            pos, 
            sequence,
            V4(1,1,1,1));
}

internal inline void
Push2DSemiCircleColored(RenderGroup *renderGroup, 
        Vec2 pos,
        R32 radius,
        R32 fromAngle,
        R32 toAngle, 
        U32 nPoints,
        AtlasRegion *texture,
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_SEMICIRCLE);
    command->pos = pos;
    command->radius = radius;
    command->range = V2(fromAngle, toAngle);
    command->nPoints = nPoints;
    command->textureHandle = texture->atlas->textureHandle;
    command->uvPos = texture->pos;
    command->uvDims = texture->size;
    command->color = color;
}

internal inline void
Push2DLineCircleColored(RenderGroup *renderGroup, 
        Vec2 pos,
        R32 radius,
        U32 nPoints,
        R32 lineWidth,
        AtlasRegion *texture,
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_LINE_CIRCLE);
    command->pos = pos;
    command->radius = radius;
    command->nPoints = nPoints;
    command->lineWidth = lineWidth;
    command->textureHandle = texture->atlas->textureHandle;
    command->uvPos = texture->pos;
    command->uvDims = texture->size;
    command->color = color;
}

void
InitRenderGroup(MemoryArena *arena, RenderGroup *renderGroup, U32 maxCommands)
{
    renderGroup->nCommands = 0;
    renderGroup->maxCommands = maxCommands;
    renderGroup->commands = PushAndZeroArray(arena, RenderCommand, renderGroup->maxCommands);
}

