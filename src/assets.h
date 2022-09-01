
typedef struct 
{
    Mesh2D *batch;
    TextureAtlas *defaultAtlas;
    TextureAtlas *creatureTextureAtlas;
    FontRenderer *fontRenderer;
    
    Shader *spriteShader;

    Shader *blurShader;
    ShaderInstance *blurShaderInstance;
} Assets;

