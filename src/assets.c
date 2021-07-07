
void
InitAssets(Assets *assets, MemoryArena *arena)
{
    assets->batch = PushStruct(arena, SpriteBatch);
    InitSpriteBatch(assets->batch, 100000, arena);

    assets->defaultAtlas = MakeDefaultTexture(arena, 256);
    assets->creatureTextureAtlas = MakeRandomTextureAtlas(arena);

    assets->spriteShader = PushStruct(arena, Shader);
    InitShader(arena, assets->spriteShader, "assets/shaders/sprite.vert", "assets/shaders/sprite.frag");
    LoadShader(assets->spriteShader);

    assets->blurShader = PushStruct(arena, Shader);
    InitShader(arena, assets->blurShader, "assets/shaders/sprite.vert", "assets/shaders/blur.frag");
    LoadShader(assets->blurShader);

    assets->blurShaderInstance = PushStruct(arena, ShaderInstance);
    InitBlurShader(assets->blurShaderInstance, assets->blurShader);

    assets->fontRenderer = PushStruct(arena, FontRenderer);
    InitFontRenderer(arena, assets->fontRenderer, "assets/DejaVuSansMono.ttf", 24.0);
}

