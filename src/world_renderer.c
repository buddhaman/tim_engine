
internal inline void
DrawBodyPartWithTexture(RenderGroup *renderGroup, 
        BodyPartDefinition *part, 
        Vec2 pos, 
        r32 angle, 
        r32 textureOverhang,
        ui32 textureHandle,
        Vec4 color)
{
    Push2DTexOrientedRectangleColored(renderGroup,
            pos,
            vec2(part->width+textureOverhang*2, part->height+textureOverhang*2),
            angle,
            textureHandle,
            part->uvPos,
            part->uvDims,
            color);
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
    ui32 maxLines = 60;
    ui32 fadeAfter = 40;
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
