
typedef struct
{
    U32 program;

    char *fragmentSourcePath;
    char *fragmentSource;

    char *vertexSourcePath;
    char *vertexSource;
} Shader;

typedef struct
{
    Shader *shader;

    U32 transformLocation;
    B32 hasMat3Transform;
    Mat3 *mat3Transform;
    B32 hasMat4Transform;
    Mat3 *mat4Transform;

    U32 radiusLocation;
    B32 hasRadius;
    R32 radius;

    U32 dir2Location;
    B32 hasDir2;
    Vec2 dir2;

} ShaderInstance;

