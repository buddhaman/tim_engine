
void
InitAtlas(MemoryArena *arena, TextureAtlas *atlas, int maxRegions)
{
    *atlas = (TextureAtlas){};
    atlas->maxRegions = maxRegions;
    atlas->regions = PushArray(arena, AtlasRegion, maxRegions);

    glGenTextures(1, &atlas->textureHandle);
    glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
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
        DebugOut("%d %d %d %d -> %f %f %f %f", 
                region->x,
                region->y,
                region->width,
                region->height,
                region->pos.x,
                region->pos.y,
                region->size.x,
                region->size.y);
    }
}

TextureAtlas *
MakeDefaultTexture(MemoryArena *arena, int circleRadius)
{
    TextureAtlas *atlas = PushStruct(arena, TextureAtlas);
    InitAtlas(arena, atlas, 2);

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

