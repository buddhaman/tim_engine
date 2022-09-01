
typedef struct
{
    TextureAtlas *atlas;
    stbtt_packedchar charData[128];
    U32 firstChar;
} FontRenderer;

typedef struct
{
    U32 vao;
    U32 vbo;
    U32 ebo;

    U32 nVertices;
    U32 maxVertices;
    U32 stride;
    R32 *vertexBuffer;

    U32 nIndices;
    U16 *indexBuffer;

    Vec4 colorState;
    B32 isDrawing;
} Mesh2D;

typedef struct
{
    U32 width;
    U32 height;

    U32 colorTexture;
    U32 fbo;
} FrameBuffer;

