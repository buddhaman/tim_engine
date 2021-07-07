
typedef struct BasicRenderTools BasicRenderTools;
struct BasicRenderTools
{
    Assets *assets;
    Camera2D *camera;
    Camera2D *screenCamera;

    RenderGroup *worldRenderGroup;
    RenderGroup *screenRenderGroup;

    ShaderInstance *worldShader;
    ShaderInstance *screenShader;
};
