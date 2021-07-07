
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
    Mat3 *transform;

} ShaderInstance;

