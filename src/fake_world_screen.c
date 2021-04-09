
void 
AdjustFakeWorldTimeScale(FakeWorldScreen *screen, int upOrDown)
{
    Assert(upOrDown==-1 || upOrDown==1);
    if(upOrDown==-1 && screen->stepsPerFrame >= 2) screen->stepsPerFrame/=2;
    if(upOrDown==1 && screen->stepsPerFrame < 1024) screen->stepsPerFrame*=2;
}

void
SetFakeWorldSelection(FakeWorldScreen *screen,
        Creature *creature, 
        BodyPart *part)
{
    screen->selectedCreature = creature;
    screen->selectedBodyPart = part;
}

void
DrawBodyPart(SpriteBatch *batch, 
        BodyPart *part,
        Vec3 creatureColor,
        r32 alpha,
        AtlasRegion *texture)
{
    RigidBody *body = part->body;
    Vec2 pos = GetBodyPos(body);
    r32 angle = GetBodyAngle(body);
    r32 shade = 1.0-part->body->drag;

    batch->colorState = vec4(shade*creatureColor.x, 
            shade*creatureColor.y, 
            shade*creatureColor.z, 
            alpha);
    PushOrientedRectangle2(batch, 
            pos,
            body->width,
            body->height,
            angle,
            texture);
}

internal inline void
DrawVecR32(SpriteBatch *batch,
        VecR32 *vec,
        Vec2 pos, 
        r32 size,
        Vec3 negativeColor,
        Vec3 positiveColor,
        r32 maxAbs,
        int xDir,
        int yDir,
        AtlasRegion *texture)
{
    for(ui32 i = 0; i < vec->n; i++)
    {
        r32 v = fabsf(vec->v[i])/maxAbs;
        batch->colorState = vec->v[i] < 0 ? vec4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : vec4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        PushRect2(batch, vec2(pos.x+i*xDir*size, pos.y+i*yDir*size), 
                vec2(size, size), texture->pos, texture->size);
    }
}

void
DrawClock(SpriteBatch *batch, 
        Vec2 pos, 
        r32 radius,
        r32 radians, 
        AtlasRegion *squareTexture,
        AtlasRegion *circleTexture)
{
    r32 lineWidth = 4;
    r32 c = cosf(radians);
    r32 s = sinf(radians);

    batch->colorState = vec4(0,0,0,1);
    PushCircle2(batch, pos, radius+lineWidth, circleTexture);
    batch->colorState = vec4(0.5,0.5,0.5,1);
    PushCircle2(batch, pos, radius, circleTexture);
    batch->colorState = vec4(0,0,0,1);
    PushLine2(batch, 
            pos, 
            v2_add(pos, vec2(c*radius, s*radius)), 
            lineWidth, 
            squareTexture->pos, 
            squareTexture->size);
}

// For now assumes batch has already begun.
void
DrawFakeWorld(FakeWorldScreen *screen, 
        SpriteBatch *batch, 
        Camera2D *camera, 
        TextureAtlas *atlas)
{
    FakeWorld *world = screen->world;
    AtlasRegion *circleRegion = atlas->regions;
    AtlasRegion *squareRegion = atlas->regions+1;
    r32 lineWidth = 2;

#if 0
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < world->nRigidBodies;
            bodyPartIdx++)
    {
        RigidBody *body = world->rigidBodies+bodyPartIdx;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);

        batch->colorState = vec4(0.0, 0.0, 0.0, 1.0);
        PushOrientedLineRectangle2(batch, 
                pos,
                body->width+lineWidth,
                body->height+lineWidth,
                angle,
                2,
                texture);
    }
#endif

    DrawGrid(batch, camera, 10.0, 1.0, squareRegion);
    DrawGrid(batch, camera, 50.0, 2.0, squareRegion);
    batch->colorState = vec4(1,1,1, 0.5);
    PushCircle2(batch, vec2(0, 0), 3, circleRegion);

    Vec3 creatureColor = vec3(1.0, 0.8, 0.8);
    for(ui32 creatureIdx = 1;
            creatureIdx < world->nCreatures;
            creatureIdx++)
    {
        Creature *creature = world->creatures+creatureIdx;
        r32 alpha = 0.2;
        for(ui32 bodyPartIdx = 0;
                bodyPartIdx < creature->nBodyParts;
                bodyPartIdx++)
        {
            BodyPart *part = creature->bodyParts+bodyPartIdx;
            DrawBodyPart(batch, part, creatureColor, alpha, squareRegion);
        }
    }
    Creature *creature = world->creatures+0;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < creature->nBodyParts;
            bodyPartIdx++)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        RigidBody *body = part->body;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);
        batch->colorState = vec4(0,0,0,1);
        PushOrientedRectangle2(batch, 
                pos,
                body->width+lineWidth,
                body->height+lineWidth,
                angle,
                squareRegion);
        DrawBodyPart(batch, part, creatureColor, 1.0, squareRegion);
    }
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
    
    // Update camera
    UpdateCamera2D(camera, appState);

    Vec2 mousePos = camera->mousePos;

    // Calculate bodypart intersection of first creature
    screen->hitBodyPart = GetCreatureBodyPartAt(world->creatures, mousePos);

    glUseProgram(spriteShader->program);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    int matLocation = glGetUniformLocation(spriteShader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);

    BeginSpritebatch(batch);

    DrawFakeWorld(screen, batch, camera, defaultAtlas);

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

    if(screen->selectedCreature)
    {
        Creature *creature = screen->selectedCreature;
        for(ui32 clockIdx = 0;
                clockIdx < creature->nInternalClocks;
                clockIdx++)
        {
            r32 radians = GetInternalClockValue(creature, clockIdx);
            DrawClock(batch, vec2(screenWidth-200+60*clockIdx, 200), 20, radians, squareRegion, circleRegion);
            DrawVecR32(batch,
                    &creature->brain->x,
                    vec2(screenWidth-200, 10),
                    10,
                    vec3(1, 0, 0),
                    vec3(0, 1, 0),
                    1.0,
                    1, 
                    0, 
                    squareRegion);
            DrawVecR32(batch,
                    &creature->brain->h,
                    vec2(screenWidth-200, 20),
                    10,
                    vec3(1, 0, 0),
                    vec3(0, 1, 0),
                    1.0,
                    1, 
                    0, 
                    squareRegion);
            DrawVecR32(batch,
                    &creature->brain->f,
                    vec2(screenWidth-200, 30),
                    10,
                    vec3(1, 0, 0),
                    vec3(0, 1, 0),
                    1.0,
                    1, 
                    0, 
                    squareRegion);
            DrawVecR32(batch,
                    &creature->brain->hc,
                    vec2(screenWidth-200, 40),
                    10,
                    vec3(1, 0, 0),
                    vec3(0, 1, 0),
                    1.0,
                    1, 
                    0, 
                    squareRegion);
        }
    }

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

    if(IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT) && !screen->isGuiInputCaptured)
    {
        if(screen->hitBodyPart)
        {
            // Only select first creature always for now.
            SetFakeWorldSelection(screen, world->creatures, screen->hitBodyPart);
        }
        else
        {
            SetFakeWorldSelection(screen, NULL, NULL);
        }
    }
    if(screen->selectedCreature)
    {
        Vec2 bodyPartPos = CameraToScreenPos(camera, appState, GetBodyPartPos(screen->selectedBodyPart));
        DrawString2D(batch, fontRenderer, bodyPartPos, "hit");
    }

    EndSpritebatch(batch);

    // Begin UI
    if(nk_begin(ctx, "Simulation", nk_rect(50, 50, 280, 400),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
    {
        nk_layout_row_static(ctx, 30, 40, 4);

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
        NKEditFloatPropertyWithTooltip(ctx, "Dev", "How much is each creature mutated", 
                1.0, &deviationScaled, parameterScale, 1.0, 1.0);
        world->strategies->dev = deviationScaled/parameterScale;

        r32 learningRateScaled = parameterScale*world->strategies->learningRate;
        NKEditFloatPropertyWithTooltip(ctx, "Learning Rate", "How fast will genes adapt", 
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
    screen->isGuiInputCaptured = nk_window_is_any_hovered(ctx);
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

