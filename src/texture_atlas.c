
void
InitAtlas(MemoryArena *arena, TextureAtlas *atlas, int maxRegions)
{
    *atlas = (TextureAtlas){};
    atlas->maxRegions = maxRegions;
    atlas->regions = PushArray(arena, AtlasRegion, maxRegions);
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
};

