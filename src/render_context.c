
void
InitRenderContext(RenderContext *renderContext, MemoryArena *arena)
{
    renderContext->batch = PushStruct(arena, SpriteBatch);
    InitSpriteBatch(renderContext->batch, 100000, arena);

    // Init simple textureatlas
    renderContext->defaultAtlas = MakeDefaultTexture(arena, 256);

    renderContext->spriteShader = PushStruct(arena, Shader);
    InitShader(renderContext->spriteShader, "shaders/sprite.vert", "shaders/sprite.frag");
    LoadShader(renderContext->spriteShader);

    renderContext->fontRenderer = PushStruct(arena, FontRenderer);
    InitFontRenderer(renderContext->fontRenderer, "DejaVuSansMono.ttf");
}

void
DrawGrid(SpriteBatch *batch, 
        Camera2D *camera, 
        r32 gridResolution, 
        r32 gridLineWidth,
        AtlasRegion *texture)
{
    r32 minX = camera->pos.x-camera->size.x/2;
    r32 minY = camera->pos.y-camera->size.y/2;
    r32 maxX = camera->pos.x+camera->size.x;
    r32 maxY = camera->pos.y+camera->size.y;
    ui32 nXLines = camera->size.x/gridResolution+2;
    ui32 nYLines = camera->size.y/gridResolution+2;
    r32 xStart = floorf(minX/gridResolution)*gridResolution;
    r32 yStart = floorf(minY/gridResolution)*gridResolution;
    ui32 maxLines = 100;
    ui32 fadeAfter = 50;
    r32 alpha = 0.2;
    ui32 mostLines = Max(nXLines, nYLines);
    if(mostLines < maxLines)
    {
        r32 fade = (maxLines-mostLines)/((r32)(maxLines-fadeAfter));
        if(fade > 1.0) fade = 1.0;
        batch->colorState = vec4(1.0, 1.0, 1.0, fade*alpha);
        for(ui32 xIdx = 0; xIdx < nXLines; xIdx++)
        {
            r32 x = xStart+xIdx*gridResolution;
            PushRect2(batch, 
                    vec2(x-gridLineWidth/2.0, minY), 
                    vec2(gridLineWidth, maxY-minY), 
                    texture->pos, 
                    texture->size);
        }
        for(ui32 yIdx = 0; yIdx < nYLines; yIdx++)
        {
            r32 y = yStart+yIdx*gridResolution;
            PushRect2(batch, 
                    vec2(minX, y-gridLineWidth/2.0),
                    vec2(maxX-minX, gridLineWidth), 
                    texture->pos, 
                    texture->size);
        }
    }
}
