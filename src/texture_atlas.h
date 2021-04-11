
typedef struct
{
    char *name;
    int width;
    int height;
    int x;
    int y;
    Vec2 pos;
    Vec2 size;
} AtlasRegion;

typedef struct
{
    // Optional
    ui32 width;
    ui32 height;
    ui32 *image;

    // Not optional
    ui32 textureHandle;
    int maxRegions;
    int nRegions;
    AtlasRegion *regions;
} TextureAtlas;

