
void
InitAtlas(MemoryArena *arena, TextureAtlas *atlas, int maxRegions)
{
    *atlas = (TextureAtlas){};
    atlas->maxRegions = maxRegions;
    atlas->regions = PushArray(arena, AtlasRegion, maxRegions);

    glGenTextures(1, &atlas->textureHandle);
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);

}

AtlasRegion *
AddAtlasRegion(TextureAtlas *atlas, int x, int y, int width, int height)
{
    AtlasRegion *region = atlas->regions+atlas->nRegions++;
    region->width = width;
    region->height = height;
    region->x = x;
    region->y = y;
    return region;
}

void
NormalizePositions(TextureAtlas *atlas, int totalWidth, int totalHeight)
{
    for(int atlasIdx = 0;
            atlasIdx < atlas->nRegions;
            atlasIdx++)
    {
        AtlasRegion *region = atlas->regions+atlasIdx;
        region->pos = vec2(region->x/((r32)totalWidth), region->y/((r32)totalHeight));
        region->size = vec2(region->width/((r32)totalWidth), region->height/((r32)totalHeight));
    }
}

// Assume square texture
void
DrawCircleOnTexture(TextureAtlas *atlas, Vec2 rangePos, Vec2 rangeSize, Vec2 uvCenter, r32 radius, ui32 color)
{
    r32 pxWidth = 1.0/atlas->width;
    r32 pxHeight = 1.0/atlas->height;
    r32 minTexX = Max(rangePos.x-pxWidth, uvCenter.x-radius);
    r32 minTexY = Max(rangePos.y-pxHeight, uvCenter.y-radius);
    r32 maxTexX = Min(rangePos.x+rangeSize.x+pxWidth, uvCenter.x+radius);
    r32 maxTexY = Min(rangePos.y+rangeSize.y+pxHeight, uvCenter.y+radius);
    int startX = round(minTexX*atlas->width);
    int startY = round(minTexY*atlas->height);
    startX = Clamp(0, startX, atlas->width);
    startY = Clamp(0, startY, atlas->height);
    int endX = round(maxTexX*atlas->width);
    int endY = round(maxTexY*atlas->height);
    endX = Clamp(0, endX, atlas->width);
    endY = Clamp(0, endY, atlas->height);
    int w = endX-startX;
    int h = endY-startY;

    if(w<=0 || h<=0) return;    // STOP EARLY

    ui32 subImage[w*h];
    memset(subImage, 0, sizeof(subImage));

    for(ui32 y = startY; y < endY; y++)
    for(ui32 x = startX; x < endX; x++)
    {
        r32 pxXDiff = pxWidth*(x+0.5) - uvCenter.x;
        r32 pxYDiff = pxHeight*(y+0.5) - uvCenter.y;
        r32 l2 = pxXDiff*pxXDiff + pxYDiff*pxYDiff;
        if(l2 <= radius*radius)
        {
            subImage[x-startX + (y-startY)*w] = atlas->image[x+atlas->width*y] = color;
        }
        else
        {
            subImage[x-startX + (y-startY)*w] = atlas->image[x+atlas->width*y];
        }
    }
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexSubImage2D(GL_TEXTURE_2D, 0, startX, startY, w, h, GL_RGBA, GL_UNSIGNED_BYTE, subImage);
}

TextureAtlas *
MakeRandomTextureAtlas(MemoryArena *arena)
{
    TextureAtlas *atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, atlas, 16);    // Grid of 4x4. 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ui32 width = 2048;
    ui32 height = 2048;
    ui32 *image = PushAndZeroArray(arena, ui32, width*height);
    atlas->image = image;
    atlas->width = width;
    atlas->height = height;
    for(int y = 0; y < width; y++)
    for(int x = 0; x < height; x++)
    {
#if 1
        image[x+y*width] = 0;
#else
        ui8 r = RandomUI32(128, 255);
        ui8 g = RandomUI32(128, 255);
        ui8 b = RandomUI32(128, 255);

        image[x+y*width] = r + (g << 8) + (b << 16) +(255U << 24);
#endif
    }
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    AddAtlasRegion(atlas, 0, 0, width, height);
    NormalizePositions(atlas, width, height);
    return atlas;
}

TextureAtlas *
MakeDefaultTexture(MemoryArena *arena, int circleRadius)
{
    TextureAtlas *atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, atlas, 2);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    int width = circleRadius*2 + 2;
    int height = circleRadius*2 + 2;

    // Not stored. Doesnt matter
    ui32 *image = malloc(sizeof(ui32)*width*height);
    r32 circleX = circleRadius;
    r32 circleY = circleRadius;
    r32 R2 = circleRadius*circleRadius;
    for(int y = 0; y < width; y++)
    for(int x = 0; x < height; x++)
    {
        r32 cx = x+0.5;
        r32 cy = y+0.5;
        r32 dx = cx-circleX;
        r32 dy = cy-circleY;
        r32 d2 = dx*dx + dy*dy;
        image[x+y*width] = d2 < R2 ? 0xFFFFFFFF : 0x0;
    }

    // Add white region
    image[width-1+(height-1)*width] = 0xFFFFFFFF;

    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    AddAtlasRegion(atlas, 0, 0, circleRadius*2, circleRadius*2);
    AddAtlasRegion(atlas, width-1, height-1, 1, 1);
    NormalizePositions(atlas, width, height);

    free(image);
    return atlas;
}

