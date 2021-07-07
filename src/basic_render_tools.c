
BasicRenderTools *
CreateBasicRenderTools(MemoryArena *arena, AppState *appState, Assets *assets)
{
    BasicRenderTools *renderTools = PushStruct(arena, BasicRenderTools);
    
    renderTools->assets = assets;

    renderTools->camera = PushStruct(arena, Camera2D);
    InitCamera2D(renderTools->camera);
    renderTools->camera->isYUp = 1;
    renderTools->camera->scale = 1.0;

    renderTools->screenCamera = appState->screenCamera;
    
    renderTools->worldRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, renderTools->worldRenderGroup, 1024);
    renderTools->screenRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, renderTools->screenRenderGroup, 4096);

    renderTools->worldShader = PushStruct(arena, ShaderInstance);
    InitSpriteShaderInstance(renderTools->worldShader, 
            assets->spriteShader, 
            &renderTools->camera->transform);
    renderTools->screenShader= PushStruct(arena, ShaderInstance);
    InitSpriteShaderInstance(renderTools->screenShader, 
            assets->spriteShader, 
            &renderTools->screenCamera->transform);

    return renderTools;
}

