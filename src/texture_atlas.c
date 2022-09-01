
void
InitAtlas(MemoryArena *arena, TextureAtlas *atlas, U32 maxRegions, U32 width, U32 height)
{
    *atlas = (TextureAtlas){};
    atlas->maxRegions = maxRegions;
    atlas->regions = PushArray(arena, AtlasRegion, maxRegions);
    atlas->width = width;
    atlas->height = height;

    glGenTextures(1, &atlas->textureHandle);
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
}

AtlasRegion *
AddAtlasRegion(TextureAtlas *atlas, int x, int y, int width, int height)
{
    AtlasRegion *region = atlas->regions+atlas->nRegions++;
    region->atlas = atlas;
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
        region->pos = V2(region->x/((R32)totalWidth), region->y/((R32)totalHeight));
        region->size = V2(region->width/((R32)totalWidth), region->height/((R32)totalHeight));
    }
}

void
SetTextureAtlasImageData(TextureAtlas *atlas, unsigned char *data)
{
    memcpy(atlas->image, data, sizeof(unsigned char)*4*atlas->width*atlas->height);
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, atlas->width, atlas->height, 
            0, GL_RGBA, GL_UNSIGNED_BYTE, atlas->image);
}

void
DrawCircleOnTexture(TextureAtlas *atlas, Vec2 rangePos, Vec2 rangeSize, Vec2 uvCenter, R32 radius, U32 color)
{
    R32 pxWidth = 1.0/atlas->width;
    R32 pxHeight = 1.0/atlas->height;
    R32 minTexX = Max(rangePos.x-pxWidth, uvCenter.x-radius);
    R32 minTexY = Max(rangePos.y-pxHeight, uvCenter.y-radius);
    R32 maxTexX = Min(rangePos.x+rangeSize.x+pxWidth, uvCenter.x+radius);
    R32 maxTexY = Min(rangePos.y+rangeSize.y+pxHeight, uvCenter.y+radius);
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

    U32 subImage[w*h];
    memset(subImage, 0, sizeof(subImage));

    for(U32 y = startY; y < endY; y++)
    for(U32 x = startX; x < endX; x++)
    {
        R32 pxXDiff = pxWidth*(x+0.5) - uvCenter.x;
        R32 pxYDiff = pxHeight*(y+0.5) - uvCenter.y;
        R32 l2 = pxXDiff*pxXDiff + pxYDiff*pxYDiff;
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
    U32 width = 2048;
    U32 height = 2048;
    TextureAtlas *atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, atlas, 16, width, height);    // Grid of 4x4. 

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    U32 *image = PushAndZeroArray(arena, U32, width*height);
    atlas->image = image;
    atlas->width = width;
    atlas->height = height;
    for(int y = 0; y < width; y++)
    for(int x = 0; x < height; x++)
    {
#if 1
        image[x+y*width] = 0;
#else
        U8 r = RandomUI32(128, 255);
        U8 g = RandomUI32(128, 255);
        U8 b = RandomUI32(128, 255);

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
    int width = circleRadius*2 + 2;
    int height = circleRadius*2 + 2;
    InitAtlas(arena, atlas, 2, width, height);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Not stored. Doesnt matter
    U32 *image = malloc(sizeof(U32)*width*height);
    R32 circleX = circleRadius;
    R32 circleY = circleRadius;
    R32 R2 = circleRadius*circleRadius;
    for(int y = 0; y < width; y++)
    for(int x = 0; x < height; x++)
    {
        R32 cx = x+0.5;
        R32 cy = y+0.5;
        R32 dx = cx-circleX;
        R32 dy = cy-circleY;
        R32 d2 = dx*dx + dy*dy;
        image[x+y*width] = d2 < R2 ? 0xFFFFFFFF : 0x00FFFFFF;
    }

    image[width-1+(height-1)*width] = 0xFFFFFFFF;

    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);

    AddAtlasRegion(atlas, 0, 0, circleRadius*2, circleRadius*2);
    AddAtlasRegion(atlas, width-1, height-1, 0, 0);
    NormalizePositions(atlas, width, height);

    // Total hack, adding half a pixel to the coordinate so white region is accurate.
    R32 xx = 1.0/width;
    R32 yy = 1.0/height;
    atlas->regions[1].pos.x+=xx*0.5f;
    atlas->regions[1].pos.y+=yy*0.5f;

    free(image);
    return atlas;
}

