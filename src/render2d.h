
typedef struct
{
    stbtt_packedchar charData12[128];
    stbtt_packedchar charData32[128];
    ui32 font12Texture;
    int textureWidth;
    int textureHeight;
} FontRenderer;

typedef struct
{
    ui32 vao;
    ui32 vbo;
    ui32 ebo;

    ui32 nVertices;
    ui32 maxVertices;
    ui32 stride;
    r32 *vertexBuffer;

    ui32 nIndices;
    ui16 *indexBuffer;

    Vec4 colorState;
} SpriteBatch;

