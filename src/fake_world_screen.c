
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

internal inline r32
GetBodyPartShade(BodyPart *part, r32 dragColorIntensity)
{
    return 1.0-part->body->drag*dragColorIntensity;
}

void
DrawBodyPart(SpriteBatch *batch, 
        BodyPart *part,
        Vec3 creatureColor,
        r32 alpha,
        r32 dragColorIntensity,
        AtlasRegion *texture)
{
    RigidBody *body = part->body;
    Vec2 pos = GetBodyPos(body);
    r32 angle = GetBodyAngle(body);
    r32 shade = GetBodyPartShade(part, dragColorIntensity);

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

internal inline void
DrawMatR32(SpriteBatch *batch,
        MatR32 *mat,
        Vec2 pos, 
        r32 size,
        Vec3 negativeColor,
        Vec3 positiveColor,
        r32 maxAbs,
        AtlasRegion *texture)
{
    for(ui32 i = 0; i < mat->w; i++)
    for(ui32 j = 0; j < mat->h; j++)
    {
        r32 m = mat->m[i+mat->w*j];
        r32 v = fabsf(m)/maxAbs;
        batch->colorState =  m < 0 ? vec4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : vec4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        PushRect2(batch, vec2(pos.x+i*size, pos.y+j*size), 
                vec2(size, size), texture->pos, texture->size);
    }
}

internal inline Vec2
DrawBrainVector(SpriteBatch *batch, VecR32 *vec, Vec2 pos, r32 size, r32 maxAbs, int xDir, int yDir, AtlasRegion *texture)
{
    DrawVecR32(batch, vec, pos, size, vec3(1,0,0), vec3(0,1,0), maxAbs, xDir, yDir, texture);
    r32 w = xDir!=0 ? vec->n*size*xDir : size;
    r32 h = yDir!=0 ? vec->n*size*yDir : size;
    return vec2(pos.x+w, pos.y+h);
}

internal inline Vec2
DrawBrainMatrix(SpriteBatch *batch, MatR32 *mat, Vec2 pos, r32 size, r32 maxAbs, AtlasRegion *texture)
{
    DrawMatR32(batch, mat, pos, size, vec3(1,0,0), vec3(0,1,0), maxAbs, texture);
    return vec2(pos.x+mat->w*size, pos.y+mat->h*size);
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
        TextureAtlas *defaultAtlas,
        TextureAtlas *creatureTextureAtlas)
{
    BeginSpritebatch(batch);
    FakeWorld *world = screen->world;
    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;

    DrawGrid(batch, camera, 10.0, 1.0, squareRegion);
    DrawGrid(batch, camera, 50.0, 2.0, squareRegion);
    batch->colorState = vec4(1,1,1, 0.5);
    PushCircle2(batch, vec2(0, 0), 3, circleRegion);

    if(screen->isPopulationVisible)
    {
        for(ui32 creatureIdx = 1;
                creatureIdx < world->nCreatures;
                creatureIdx++)
        {
            Creature *creature = world->creatures+creatureIdx;
            Vec3 creatureColor = creature->solidColor;
            r32 alpha = 0.2;
            for(ui32 bodyPartIdx = 0;
                    bodyPartIdx < creature->nBodyParts;
                    bodyPartIdx++)
            {
                BodyPart *part = creature->bodyParts+bodyPartIdx;
                DrawBodyPart(batch, part, creatureColor, alpha, 1.0, squareRegion);
            }
        }
    }

    // Draw first creature
    Creature *creature = world->creatures+0;
    if(creature->drawSolidColor)
    {
        Vec3 creatureColor = creature->solidColor;
        batch->colorState = vec4(1,1,1,1);
        for(int bodyPartIdx = creature->nBodyParts-1;
                bodyPartIdx >= 0;
                bodyPartIdx--)
        {
            BodyPart *part = creature->bodyParts+bodyPartIdx;
            r32 dragFactor = screen->isDragVisible ? 1.0 :  0.0;
            DrawBodyPart(batch, part, creatureColor, 1.0, dragFactor, squareRegion);
        }
    }

    EndSpritebatch(batch);

    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, creatureTextureAtlas->textureHandle);
    // Only draw first creature with fancy texture
    // Draw in reverse order. Should draw in reverse order of degree. 
    batch->colorState = vec4(1,1,1,1);
    for(int bodyPartIdx = creature->nBodyParts-1;
            bodyPartIdx >= 0;
            bodyPartIdx--)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        RigidBody *body = part->body;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);
        r32 shade = screen->isDragVisible ? GetBodyPartShade(part, 1.0) : 1.0;
        batch->colorState = vec4(shade, shade, shade, 1.0);
        DrawBodyPartWithTexture(batch, part->def, pos, angle, world->def.textureOverhang);
    }
    EndSpritebatch(batch);
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

    char toolTip[512];
    (void)toolTip;

    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;

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
                // Next Generation
                for(ui32 geneIdx = 0;
                        geneIdx < world->nGenes;
                        geneIdx++)
                {
                    world->strategies->fitness->v[geneIdx] = CreatureGetFitness(world->creatures+geneIdx);
                }
                screen->avgFitness = VecR32Average(world->strategies->fitness);
                screen->fitnessGraph[screen->generation] = screen->avgFitness;
                screen->generation++;
                screen->tick = 0;

                // Setup new gene and restart world.
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

    DrawFakeWorld(screen, batch, camera, defaultAtlas, renderer->creatureTextureAtlas);

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
        MinimalGatedUnit *brain = creature->brain;

        r32 size = 10.0;
        r32 maxAbs = 1.0;
        r32 pad = 3.0;
        r32 totalBrainWidth = (brain->Wf.w+brain->Uf.w+1)*size+pad*2;
        r32 leftX = screenWidth - totalBrainWidth - 10;
        r32 atX = leftX;
        r32 atY = 10;
        r32 maxRowY = 0;
        Vec2 lastPos;

        lastPos = DrawBrainMatrix(batch, &brain->Wf, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(batch, &brain->Uf, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(batch, &brain->bf, vec2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        lastPos = DrawBrainMatrix(batch, &brain->Wh, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(batch, &brain->Uh, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(batch, &brain->bh, vec2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        DrawBrainVector(batch, &brain->x, vec2(atX, atY), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(batch, &brain->h, vec2(atX, atY+size), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(batch, &brain->hc, vec2(atX, atY+size*2), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(batch, &brain->f, vec2(atX, atY+size*3), size, 1.0, 1, 0, squareRegion);
        atY+=size*4+pad*3;

        // Draw clocks
        for(ui32 clockIdx = 0;
                clockIdx < creature->nInternalClocks;
                clockIdx++)
        {
            r32 radians = GetInternalClockValue(creature, clockIdx);
            r32 radius = 20;
            DrawClock(batch, vec2(atX+radius+radius*2*clockIdx+pad*2*clockIdx, atY+radius), 
                    radius, radians, squareRegion, circleRegion);
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

    // Draw tooltip
    if(toolTip[0])
    {
        DrawString2D(batch, fontRenderer, screenCamera->mousePos, toolTip);
    }

    if(IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT) && !screen->isGuiInputCaptured)
    {
        if(screen->hitBodyPart)
        {
            // Only select from first creature always for now.
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

        if(nk_tree_push(ctx, NK_TREE_TAB, "Settings", NK_MINIMIZED))
        {
            screen->isPopulationVisible = nk_check_label(ctx, "Show entire population", screen->isPopulationVisible);
            screen->isDragVisible = nk_check_label(ctx, "Show drag", screen->isDragVisible);
            nk_tree_pop(ctx);
        }

        if(nk_button_label(ctx, "Editor"))
        {
            appState->currentScreen = SCREEN_CREATURE_EDITOR;
        }

    }
    nk_end(ctx);

#if 0
    // Begin UI
    if(nk_begin(ctx, "Graph", nk_rect(50, 400, 280, 400),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE))
    {
        nk_layout_row_dynamic(ctx, 30,1);
        nk_label(ctx, "llksdjf", NK_TEXT_LEFT);
        nk_layout_row_dynamic(ctx, 400, 1);
        nk_plot(ctx, NK_CHART_LINES, screen->fitnessGraph, screen->generation, 0);
    }
    nk_end(ctx);
#endif

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
    screen->fitnessGraph = PushAndZeroArray(arena, r32, 10000);

    // Settings
    screen->isPopulationVisible = 0;
    screen->isDragVisible = 0;

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

