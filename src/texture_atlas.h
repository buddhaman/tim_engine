
typedef struct TextureAtlas TextureAtlas;
typedef struct AtlasRegion AtlasRegion;

struct AtlasRegion
{
    TextureAtlas *atlas;
    char *name;
    int width;
    int height;
    int x;
    int y;
    Vec2 pos;
    Vec2 size;
};

struct TextureAtlas
{
    // Optional
    U32 width;
    U32 height;
    U32 *image;

    // Not optional
    U32 textureHandle;
    int maxRegions;
    int nRegions;
    AtlasRegion *regions;
};

