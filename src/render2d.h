
typedef struct
{
    TextureAtlas *atlas;
    stbtt_packedchar charData[128];
    ui32 firstChar;
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
    b32 isDrawing;
} SpriteBatch;

typedef struct
{
    ui32 width;
    ui32 height;

    ui32 colorTexture;
    ui32 fbo;
} FrameBuffer;

