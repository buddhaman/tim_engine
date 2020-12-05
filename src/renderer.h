#define N_ATTRIBUTES 6
typedef enum
{
    ATTR_POS2       = 1 << 0,
    ATTR_POS3       = 1 << 1,
    ATTR_COL3       = 1 << 2,
    ATTR_COL4       = 1 << 3,
    ATTR_TEX        = 1 << 4,
    ATTR_NORM3      = 1 << 5,
} VertexAttributeFlag;

typedef struct
{
    VertexAttributeFlag type;
    int offset;
} VertexAttribute;

typedef struct 
{
    ui32 vertexAttributes;
    Vec3 colorState;
    int nVertices;
    int maxVertices;
    Vec2 *vertices2;
    Vec3 *vertices;
    Vec3 *colors;
    Vec3 *normals;
    Vec2 *texCoords;
    int nIndices;
    int maxIndices;
    ui32 *indices;
} Mesh;

typedef struct 
{
    ui32 vao;
    ui32 vbo;
    ui32 ebo;
    int stride;

    int nVertexAttributes;
    VertexAttribute vertexAttributes[N_ATTRIBUTES];
    int vertexBufferSize;
    int maxVertexBufferSize;
    r32 *vertexBuffer;

    int indexBufferSize;
    int maxIndexBufferSize;
    ui32 *indexBuffer;
} Model;

typedef struct
{
    ui32 program;

    char *fragmentSourcePath;
    char *fragmentSource;

    char *vertexSourcePath;
    char *vertexSource;
} Shader;

typedef struct
{
    Vec3 pos;
    Vec3 lookAt;
    Vec3 spherical;
    Mat4 transform;
} Camera;

typedef struct
{
    stbtt_packedchar charData12[128];
    stbtt_packedchar charData32[128];
    ui32 font12Texture;
    int textureWidth;
    int textureHeight;
} FontRenderer;

