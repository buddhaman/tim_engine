

void AdjustFakeWorldTimeScale(FakeWorldScreen *screen, int upOrDown)
{
    Assert(upOrDown==-1 || upOrDown==1);
    if(upOrDown==-1 && screen->stepsPerFrame >= 2) screen->stepsPerFrame/=2;
    if(upOrDown==1 && screen->stepsPerFrame < 1024) screen->stepsPerFrame*=2;
}

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
        AdjustFakeWorldTimeScale(screen, -1);
    }
    if(IsKeyActionJustDown(appState, ACTION_E))
    {
        AdjustFakeWorldTimeScale(screen, 1);
    }
    UpdateCameraInput(appState, camera);

    // Do evolution
    if(!screen->isPaused)
    {
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
    if(nk_begin(ctx, "Simulation", nk_rect(50, 50, 280, 400),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
    {
        nk_layout_row_static(ctx, 30, 30, 4);

        // Edit time
        if(nk_button_label(ctx, "<<"))
        {
            AdjustFakeWorldTimeScale(screen, -1);
        }
        if(nk_button_label(ctx, screen->isPaused ? ">" : "||"))
        {
            screen->isPaused = !screen->isPaused;
        }
        if(nk_button_label(ctx, ">>"))
        {
            AdjustFakeWorldTimeScale(screen, 1);
        }
        nk_labelf(ctx, NK_TEXT_LEFT, "x%u", screen->stepsPerFrame);

        nk_layout_row_dynamic(ctx, 30, 1);

        int seconds = screen->ticksPerGeneration/60;
        seconds = nk_propertyi(ctx, "Seconds per generation", 1, seconds, 60, 1, 1);
        screen->ticksPerGeneration = seconds*60;

        r32 parameterScale = 1000.0;

        r32 deviationScaled = parameterScale * world->strategies->dev;
        NKEditFloatPropertyWithTooltip(ctx, "Dev", "How much is each creature mutated.", 
                1.0, &deviationScaled, parameterScale, 1.0, 1.0);
        world->strategies->dev = deviationScaled/parameterScale;

        r32 learningRateScaled = parameterScale*world->strategies->learningRate;
        NKEditFloatPropertyWithTooltip(ctx, "Learning Rate", "How fast will genes adapt.", 
                1.0, &learningRateScaled, parameterScale, 1.0, 1.0);
        world->strategies->learningRate = learningRateScaled/parameterScale;

        if(nk_tree_push(ctx, NK_TREE_TAB, "Properties", NK_MINIMIZED))
        {
            nk_labelf(ctx,  NK_TEXT_LEFT, "Population: %d", world->nGenes);
            nk_labelf(ctx,  NK_TEXT_LEFT, "Gene size: %d", world->def.geneSize);
            nk_labelf(ctx,  NK_TEXT_LEFT, "Inputs: %d", world->def.nInputs);
            nk_labelf(ctx,  NK_TEXT_LEFT, "Outputs: %d", world->def.nOutputs);
            nk_labelf(ctx,  NK_TEXT_LEFT, "Hidden: %d", world->def.nHidden);
            nk_tree_pop(ctx);
        }

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

