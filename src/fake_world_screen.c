
void
UpdateFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        RenderContext *renderer, 
        struct nk_context *ctx) 
{
    Camera2D *camera = screen->camera;
    Camera2D *screenCamera = appState->screenCamera;    // Mildly confusing
    SpriteBatch *batch = renderer->batch;
    FontRenderer *fontRenderer = renderer->fontRenderer;
    FakeWorld *world = screen->world;
    TextureAtlas *defaultAtlas = renderer->defaultAtlas;
    Shader *spriteShader = renderer->spriteShader;

    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;
    (void)circleRegion;

    // Adjust timescale
    if(IsKeyActionJustDown(appState, ACTION_Q))
    {
        if(screen->stepsPerFrame >= 2) screen->stepsPerFrame/=2;
    }
    if(IsKeyActionJustDown(appState, ACTION_E))
    {
        if(screen->stepsPerFrame < 1024) screen->stepsPerFrame*=2;
    }
    UpdateCameraInput(appState, camera);

    // Do evolution
    for(ui32 atFrameStep = 0;
            atFrameStep < screen->stepsPerFrame;
            atFrameStep++)
    {
        UpdateFakeWorld(world);
        screen->tick++;
        if(screen->tick >= screen->ticksPerGeneration)
        {
            screen->tick = 0;
            screen->generation++;
            for(ui32 geneIdx = 0;
                    geneIdx < world->nGenes;
                    geneIdx++)
            {
                world->strategies->fitness->v[geneIdx] = CreatureGetFitness(world->creatures+geneIdx);
            }
            screen->avgFitness = VecR32Average(world->strategies->fitness);
            ESNextSolution(world->strategies);
            ESGenerateGenes(world->strategies);
            DestroyFakeWorld(world);
            RestartFakeWorld(world);
        }
    }
    
    // Render world
    UpdateCamera2D(camera, appState);

    glUseProgram(spriteShader->program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    int matLocation = glGetUniformLocation(spriteShader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);

    BeginSpritebatch(batch);

    DrawFakeWorld(world, batch, camera, defaultAtlas);

    EndSpritebatch(batch);

    // Render on screen
    BeginSpritebatch(batch);
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&screenCamera->transform);

    r32 screenWidth = screenCamera->size.x;
    r32 screenHeight = screenCamera->size.y;
    r32 bottomBarHeight = 50;

    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);
    batch->colorState = appState->clearColor;
    PushRect2(batch,
            vec2(0,screenHeight-bottomBarHeight),
            vec2(screenWidth, bottomBarHeight),
            squareRegion->pos,
            squareRegion->size);
    batch->colorState = vec4(1,1,1,0.2);
    PushRect2(batch,
            vec2(0, screenHeight-bottomBarHeight),
            vec2(screenWidth, 2),
            squareRegion->pos,
            squareRegion->size);

    EndSpritebatch(batch);

    // Only text from here
    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, fontRenderer->font12Texture);
    char info[512];

    batch->colorState=vec4(1,1,1,1);
    sprintf(info, "Steps per frame: %u. At Generation %u (%u/%u) fitness = %f", screen->stepsPerFrame,
            screen->generation, 
            screen->tick, 
            screen->ticksPerGeneration, 
            screen->avgFitness);
    DrawString2D(batch, fontRenderer, vec2(20, screenHeight-bottomBarHeight/2+8), info);

    EndSpritebatch(batch);

    // Begin UI
    if(nk_begin(ctx, "Cool Window", nk_rect(50, 50, 220, 320),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE))
    {
        nk_layout_row_dynamic(ctx, 20, 1);
        nk_labelf(ctx,  NK_TEXT_LEFT, "Population: %d", world->nGenes);
        nk_labelf(ctx,  NK_TEXT_LEFT, "Gene size: %d", world->geneSize);
        nk_labelf(ctx,  NK_TEXT_LEFT, "Inputs: %d", world->inputSize);
        nk_labelf(ctx,  NK_TEXT_LEFT, "Outputs: %d", world->outputSize);
        nk_labelf(ctx,  NK_TEXT_LEFT, "Hidden: %d", world->hiddenSize);

        nk_property_float(ctx, "Dev", 0.0, &world->strategies->dev, 0.5, 0.001, 0.001);
        nk_property_float(ctx, "Learning Rate", 0.0, &world->strategies->learningRate, 0.5, 0.001, 0.001);
        if(nk_button_label(ctx, "Editor"))
        {
            appState->currentScreen = SCREEN_CREATURE_EDITOR;
        }

    }
    nk_end(ctx);
}

void 
InitFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        MemoryArena *arena, 
        CreatureDefinition *def,
        ui32 nGenes,
        r32 dev,
        r32 learningRate)
{

    screen->stepsPerFrame = 1;
    screen->generation = 0;
    screen->tick = 0;
    screen->ticksPerGeneration = 60*15;    // 10 seconds
    screen->avgFitness = -100000.0;

    // Camera
    screen->camera = PushStruct(arena, Camera2D);
    InitCamera2D(screen->camera);
    screen->camera->isYUp = 1;
    screen->camera->scale = 1.0;

    // Init fake world
    screen->world = PushStruct(arena, FakeWorld);
    screen->evolutionArena = CreateSubArena(arena, 64L*1000L*1000L);
    InitFakeWorld(screen->world, arena, screen->evolutionArena, def, nGenes, learningRate, dev);

    ESGenerateGenes(screen->world->strategies);
}

