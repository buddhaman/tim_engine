
void
InitAssets(Assets *assets, MemoryArena *arena)
{
    assets->batch = PushStruct(arena, SpriteBatch);
    InitSpriteBatch(assets->batch, 100000, arena);

    assets->defaultAtlas = MakeDefaultTexture(arena, 256);
    assets->creatureTextureAtlas = MakeRandomTextureAtlas(arena);

    assets->spriteShader = PushStruct(arena, Shader);
    InitShader(assets->spriteShader, "assets/shaders/sprite.vert", "assets/shaders/sprite.frag");
    LoadShader(assets->spriteShader);

    assets->fontRenderer = PushStruct(arena, FontRenderer);
    InitFontRenderer(arena, assets->fontRenderer, "assets/DejaVuSansMono.ttf", 24.0);
}

