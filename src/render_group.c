
void 
ExecuteRenderGroup(RenderGroup *renderGroup, Assets *assets, Camera2D *camera, Shader *shader)
{
    glUseProgram(shader->program);

    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // TODO: Better way to implement uniform location lookup with shaders.
    int matLocation = glGetUniformLocation(shader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);

    SpriteBatch *batch = assets->batch;
    BeginSpritebatch(batch);
    ui32 currentTextureHandle = 0;

    for(ui32 commandIdx = 0;
            commandIdx < renderGroup->nCommands;
            commandIdx++)
    {
        RenderCommand *command = renderGroup->commands+commandIdx;

        // TODO: Remove this code by sorting and inserting state changes.
        ui32 handle = command->textureHandle;
        if(currentTextureHandle!=handle)
        {
            EndSpritebatch(batch);
            BeginSpritebatch(batch);
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


        default:
        {
            DebugOut("RENDERCOMMAND NOT IMPLEMENTED YET");
        } break;

        }
    }
    EndSpritebatch(batch);
    renderGroup->nCommands = 0;
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
        ui32 textureHandle,
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
        Vec4 color,
        AtlasRegion *texture)
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
        r32 radius,
        Vec4 color,
        AtlasRegion *texture)
{
    Push2DTexRectColored(renderGroup,
            vec2(center.x-radius, center.y-radius),
            vec2(radius*2, radius*2),
            texture->atlas->textureHandle,
            texture->pos,
            texture->size, 
            color);
}

internal inline void
Push2DTexLineColored(RenderGroup *renderGroup,
        Vec2 from,
        Vec2 to,
        r32 lineWidth,
        ui32 textureHandle,
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
        r32 lineWidth,
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
        r32 angle,
        ui32 textureHandle,
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
        r32 angle,
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
        r32 angle, 
        r32 lineWidth,
        AtlasRegion *texture,
        Vec4 color)
{
    r32 c = cosf(angle);
    r32 s = sinf(angle);
    Vec2 axis0 = vec2(c*dims.x/2.0, s*dims.x/2.0);
    Vec2 axis1 = vec2(s*dims.y/2.0, -c*dims.y/2.0);
    Vec2 p00 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, -1)));
    Vec2 p10 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, -1)));
    Vec2 p11 = v2_add(pos, v2_add(v2_muls(axis0, 1), v2_muls(axis1, 1)));
    Vec2 p01 = v2_add(pos, v2_add(v2_muls(axis0, -1), v2_muls(axis1, 1)));
    Push2DLineColored(renderGroup, p00, p10, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p10, p11, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p11, p01, lineWidth, texture, color);
    Push2DLineColored(renderGroup, p01, p00, lineWidth, texture, color);
}


internal inline void
Push2DTextColored(RenderGroup *renderGroup, 
        FontRenderer *fontRenderer,
        Vec2 pos, 
        Vec4 color,
        char *sequence)
{
    stbtt_aligned_quad q;
    r32 x = pos.x;
    r32 y = pos.y;
    ui32 w = fontRenderer->atlas->width;
    ui32 h = fontRenderer->atlas->height;
    ui32 textureHandle = fontRenderer->atlas->textureHandle;

    while(*sequence)
    {
        stbtt_GetPackedQuad(fontRenderer->charData, w, h, *sequence-32, &x, &y, &q, 0);
        Push2DTexRectColored(renderGroup, 
                vec2(q.x0, q.y0), 
                vec2(q.x1-q.x0, q.y1-q.y0), 
                textureHandle, 
                vec2(q.s0, q.t1), 
                vec2(q.s1-q.s0, q.t0-q.t1), 
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
            vec4(1,1,1,1),
            sequence);
}

internal inline void
Push2DSemiCircleColored(RenderGroup *renderGroup, 
        Vec2 pos,
        r32 radius,
        r32 fromAngle,
        r32 toAngle, 
        ui32 nPoints,
        AtlasRegion *texture,
        Vec4 color)
{
    RenderCommand *command = PushRenderCommand(renderGroup, RENDER_2D_SEMICIRCLE);
    command->pos = pos;
    command->radius = radius;
    command->range = vec2(fromAngle, toAngle);
    command->nPoints = nPoints;
    command->textureHandle = texture->atlas->textureHandle;
    command->uvPos = texture->pos;
    command->uvDims = texture->size;
    command->color = color;
}

void
InitRenderGroup(MemoryArena *arena, RenderGroup *renderGroup, ui32 maxCommands)
{
    renderGroup->nCommands = 0;
    renderGroup->maxCommands = maxCommands;
    renderGroup->commands = PushAndZeroArray(arena, RenderCommand, renderGroup->maxCommands);
}

