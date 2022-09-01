
typedef struct 
{
    Mesh2D *mesh;
    TextureAtlas *defaultAtlas;
    TextureAtlas *creatureTextureAtlas;
    FontRenderer *fontRenderer;
    
    Shader *spriteShader;

    Shader *blurShader;
    ShaderInstance *blurShaderInstance;
} Assets;

