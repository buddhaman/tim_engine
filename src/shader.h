
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
    Shader *shader;

    ui32 transformLocation;
    b32 hasMat3Transform;
    Mat3 *mat3Transform;
    b32 hasMat4Transform;
    Mat3 *mat4Transform;

    ui32 radiusLocation;
    b32 hasRadius;
    r32 radius;

    ui32 dir2Location;
    b32 hasDir2;
    Vec2 dir2;

} ShaderInstance;

