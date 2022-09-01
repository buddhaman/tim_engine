
internal inline void
DrawBodyPartWithTexture(RenderGroup *renderGroup, 
        BodyPartDefinition *part, 
        Vec2 pos, 
        R32 angle, 
        R32 textureOverhang,
        U32 textureHandle,
        Vec4 color)
{
    Push2DTexOrientedRectangleColored(renderGroup,
            pos,
            V2(part->width+textureOverhang*2, part->height+textureOverhang*2),
            angle,
            textureHandle,
            part->uvPos,
            part->uvDims,
            color);
}

void
DrawGrid(Mesh2D *mesh, 
        Camera2D *camera, 
        R32 gridResolution, 
        R32 gridLineWidth,
        AtlasRegion *texture)
{
    R32 minX = camera->pos.x-camera->size.x/2;
    R32 minY = camera->pos.y-camera->size.y/2;
    R32 maxX = camera->pos.x+camera->size.x;
    R32 maxY = camera->pos.y+camera->size.y;

    U32 nXLines = camera->size.x/gridResolution+2;
    U32 nYLines = camera->size.y/gridResolution+2;

    R32 xStart = floorf(minX/gridResolution)*gridResolution;
    R32 yStart = floorf(minY/gridResolution)*gridResolution;

    U32 maxLines = 60;
    U32 fadeAfter = 40;
    R32 alpha = 0.2;
    U32 mostLines = Max(nXLines, nYLines);

    if(mostLines < maxLines)
    {
        R32 fade = (maxLines-mostLines)/((R32)(maxLines-fadeAfter));
        if(fade > 1.0) fade = 1.0;
        mesh->colorState = V4(1.0, 1.0, 1.0, fade*alpha);
        for(U32 xIdx = 0; xIdx < nXLines; xIdx++)
        {
            R32 x = xStart+xIdx*gridResolution;
            PushRect2(mesh, 
                    V2(x-gridLineWidth/2.0, minY), 
                    V2(gridLineWidth, maxY-minY), 
                    texture->pos, 
                    texture->size);
        }
        for(U32 yIdx = 0; yIdx < nYLines; yIdx++)
        {
            R32 y = yStart+yIdx*gridResolution;
            PushRect2(mesh, 
                    V2(minX, y-gridLineWidth/2.0),
                    V2(maxX-minX, gridLineWidth), 
                    texture->pos, 
                    texture->size);
        }
    }
}
