
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

internal inline void
DrawSolidRigidBody(RenderGroup *renderGroup, 
        RigidBody *body,
        AtlasRegion *texture,
        Vec4 color)
{
    Vec2 pos = GetBodyPos(body);
    r32 angle = GetBodyAngle(body);

    Push2DOrientedRectangleColored(renderGroup, 
            pos,
            vec2(body->width, body->height),
            angle,
            texture,
            color);
}

void
DrawBodyPart(RenderGroup *renderGroup,
        BodyPart *part,
        Vec3 creatureColor,
        r32 alpha,
        r32 dragColorIntensity,
        AtlasRegion *texture)
{
    RigidBody *body = part->body;
    r32 shade = GetBodyPartShade(part, dragColorIntensity);
    Vec4 color = vec4(shade*creatureColor.x, 
            shade*creatureColor.y, 
            shade*creatureColor.z, 
            alpha);
    DrawSolidRigidBody(renderGroup, body, texture, color);
}

internal inline void
DrawVecR32(RenderGroup *renderGroup,
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
        Vec4 color = vec->v[i] < 0 ? vec4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : vec4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        Push2DRectColored(renderGroup, vec2(pos.x+i*xDir*size, pos.y+i*yDir*size), 
                vec2(size, size), texture, color);
    }
}

internal inline void
DrawMatR32(RenderGroup *renderGroup,
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
        Vec4 color =  m < 0 ? vec4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : vec4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        Push2DRectColored(renderGroup, vec2(pos.x+i*size, pos.y+j*size), 
                vec2(size, size), texture, color);
    }
}

internal inline Vec2
DrawBrainVector(RenderGroup *renderGroup, VecR32 *vec, Vec2 pos, r32 size, r32 maxAbs, int xDir, int yDir, AtlasRegion *texture)
{
    DrawVecR32(renderGroup, vec, pos, size, vec3(1,0,0), vec3(0,1,0), maxAbs, xDir, yDir, texture);
    r32 w = xDir!=0 ? vec->n*size*xDir : size;
    r32 h = yDir!=0 ? vec->n*size*yDir : size;
    return vec2(pos.x+w, pos.y+h);
}

internal inline Vec2
DrawBrainMatrix(RenderGroup *renderGroup, MatR32 *mat, Vec2 pos, r32 size, r32 maxAbs, AtlasRegion *texture)
{
    DrawMatR32(renderGroup, mat, pos, size, vec3(1,0,0), vec3(0,1,0), maxAbs, texture);
    return vec2(pos.x+mat->w*size, pos.y+mat->h*size);
}

void
DrawClock(RenderGroup *renderGroup,
        Vec2 pos, 
        r32 radius,
        r32 radians, 
        AtlasRegion *squareTexture,
        AtlasRegion *circleTexture)
{
    r32 lineWidth = 4;
    r32 c = cosf(radians);
    r32 s = sinf(radians);

    Push2DCircleColored(renderGroup, pos, radius+lineWidth, circleTexture, vec4(0,0,0,1));
    Push2DCircleColored(renderGroup, pos, radius, circleTexture, vec4(0.5, 0.5, 0.5, 1.0));
    Push2DLineColored(renderGroup, 
            pos, 
            v2_add(pos, vec2(c*radius, s*radius)), 
            lineWidth, 
            squareTexture, vec4(0, 0, 0, 1));
}

void
DrawFakeWorld(FakeWorldScreen *screen, 
        RenderGroup *renderGroup,
        Camera2D *camera, 
        TextureAtlas *defaultAtlas,
        TextureAtlas *creatureTextureAtlas)
{
    FakeWorld *world = screen->world;
    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;

    Push2DCircleColored(renderGroup, vec2(0, 0), 3, circleRegion, vec4(1,1,1,0.5));

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
                DrawBodyPart(renderGroup, part, creatureColor, alpha, 1.0, squareRegion);
            }
        }
    }

    for(ui32 floorIdx = 0;
            floorIdx < world->nStaticBodies;
            floorIdx++)
    {
        RigidBody *floor = world->staticBodies[floorIdx];
        DrawSolidRigidBody(renderGroup, floor, squareRegion, vec4(0.5, 0.5, 0.5, 1.0));
    }

    // Draw first creature
    Creature *creature = world->creatures+0;
    if(creature->drawSolidColor)
    {
        Vec3 creatureColor = creature->solidColor;
        for(int bodyPartIdx = creature->nBodyParts-1;
                bodyPartIdx >= 0;
                bodyPartIdx--)
        {
            BodyPart *part = creature->bodyParts+bodyPartIdx;
            r32 dragFactor = screen->isDragVisible ? 1.0 :  0.0;
            DrawBodyPart(renderGroup, part, creatureColor, 1.0, dragFactor, squareRegion);
        }
    }

    for(int bodyPartIdx = creature->nBodyParts-1;
            bodyPartIdx >= 0;
            bodyPartIdx--)
    {
        BodyPart *part = creature->bodyParts+bodyPartIdx;
        RigidBody *body = part->body;
        Vec2 pos = GetBodyPos(body);
        r32 angle = GetBodyAngle(body);
        r32 shade = screen->isDragVisible ? GetBodyPartShade(part, 1.0) : 1.0;
        Vec4 color = vec4(shade, shade, shade, 1.0);
        DrawBodyPartWithTexture(renderGroup, part->def, pos, angle, world->def.textureOverhang, 
                creatureTextureAtlas->textureHandle, color);
    }
}

void
UpdateFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        struct nk_context *ctx) 
{
    BasicRenderTools *renderTools = screen->renderTools;
    Camera2D *camera = renderTools->camera;
    Camera2D *screenCamera = renderTools->screenCamera;
    Assets *assets = renderTools->assets;

    RenderGroup *worldRenderGroup = renderTools->worldRenderGroup;
    RenderGroup *screenRenderGroup = renderTools->screenRenderGroup;

    FontRenderer *fontRenderer = assets->fontRenderer;
    FakeWorld *world = screen->world;
    TextureAtlas *defaultAtlas = assets->defaultAtlas;

    char toolTip[512];
    memset(toolTip, 0, sizeof(toolTip));
    (void)toolTip;

    AtlasRegion *circleRegion = assets->defaultAtlas->regions;
    AtlasRegion *squareRegion = assets->defaultAtlas->regions+1;

    screen->isInputCaptured = nk_window_is_any_hovered(ctx);

    // Adjust timescale
    if(IsKeyActionJustDown(appState, ACTION_Q))
    {
        AdjustFakeWorldTimeScale(screen, -1);
    }
    if(IsKeyActionJustDown(appState, ACTION_E))
    {
        AdjustFakeWorldTimeScale(screen, 1);
    }

    if(!screen->isInputCaptured)
    {
        UpdateCameraInput(appState, camera);
    }

    if(!IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT))
    {
        CameraStopDragging(camera);
    }

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
                    world->strategies->fitness->v[geneIdx] = CreatureGetFitness(world, world->creatures+geneIdx);
                }
                screen->avgFitness = VecR32Average(world->strategies->fitness);
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

    DrawFakeWorld(screen, worldRenderGroup, camera, defaultAtlas, assets->creatureTextureAtlas);

    if(world->trainingType==TRAIN_DISTANCE_TARGET)
    {
        Vec4 circleColor = vec4(1.0, 1.0, 0.0, 1.0);
        Push2DCircleColored(worldRenderGroup, world->target, 10, circleRegion, circleColor);
    }

    // Render on screen

    r32 screenWidth = screenCamera->size.x;
    r32 screenHeight = screenCamera->size.y;
    r32 bottomBarHeight = 50;

    Push2DRectColored(screenRenderGroup,
            vec2(0,screenHeight-bottomBarHeight),
            vec2(screenWidth, bottomBarHeight),
            squareRegion,
            appState->clearColor);

    Push2DRectColored(screenRenderGroup,
            vec2(0, screenHeight-bottomBarHeight),
            vec2(screenWidth, 2),
            squareRegion,
            vec4(1,1,1,1));

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

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Wf, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Uf, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(screenRenderGroup, &brain->bf, vec2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Wh, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Uh, vec2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(screenRenderGroup, &brain->bh, vec2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        DrawBrainVector(screenRenderGroup, &brain->x, vec2(atX, atY), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->h, vec2(atX, atY+size), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->hc, vec2(atX, atY+size*2), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->f, vec2(atX, atY+size*3), size, 1.0, 1, 0, squareRegion);
        atY+=size*4+pad*3;

        // Draw clocks
        for(ui32 clockIdx = 0;
                clockIdx < creature->nInternalClocks;
                clockIdx++)
        {
            r32 radians = GetInternalClockValue(creature, clockIdx);
            r32 radius = 20;
            DrawClock(screenRenderGroup, vec2(atX+radius+radius*2*clockIdx+pad*2*clockIdx, atY+radius), 
                    radius, radians, squareRegion, circleRegion);
        }

        // Draw inputs

        BodyPart *selectedBodyPart = screen->selectedBodyPart;
        r32 lineWidth = 2.0;
        if(selectedBodyPart && 
                world->trainingType==TRAIN_DISTANCE_TARGET)
        {
            r32 angle = GetBodyAngle(selectedBodyPart->body);
            Vec2 dir = v2_polar(-angle, 100.0);
            Vec2 pos = CameraToScreenPos(camera, appState, GetBodyPos(selectedBodyPart->body));
            Vec2 targetPos = CameraToScreenPos(camera, appState, world->target);
            Vec2 to = v2_add(pos, dir);
            Push2DLineColored(screenRenderGroup, 
                    pos, 
                    to, 
                    lineWidth,
                    squareRegion, vec4(1, 0, 0, 1));
            Push2DLineColored(screenRenderGroup, 
                    pos, 
                    targetPos,
                    lineWidth,
                    squareRegion,
                    vec4(0, 1, 0, 1));
        }
    }

    // Only text from here
    char info[512];

    sprintf(info, "Steps per frame: %u. At Generation %u (%u/%u) fitness = %f", screen->stepsPerFrame,
            screen->generation, 
            screen->tick, 
            screen->ticksPerGeneration, 
            screen->avgFitness);
    Push2DText(screenRenderGroup, fontRenderer, vec2(20, screenHeight-bottomBarHeight/2+8), info);

    // Draw tooltip
    if(toolTip[0])
    {
        Push2DText(screenRenderGroup, fontRenderer, screenCamera->mousePos, toolTip);
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
        Creature *creature = screen->selectedCreature;
        BodyPart *part = screen->selectedBodyPart;
        Vec2 bodyPartPos = CameraToScreenPos(camera, appState, GetBodyPartPos(screen->selectedBodyPart));
        if(part->def->hasAngleTowardsTargetInput)
        {
            r32 activation = creature->brain->x.v[part->def->angleTowardsTargetInputIdx];
            char bodyPartInfo[128];
            sprintf(bodyPartInfo, "angle activation = %.2f", activation); 
            Push2DText(screenRenderGroup, fontRenderer, bodyPartPos, bodyPartInfo);
        } 
        else if(part->def->hasAbsoluteAngleInput)
        {
            r32 activation = creature->brain->x.v[part->def->absoluteAngleInputIdx];
            char bodyPartInfo[128];
            sprintf(bodyPartInfo, "angle activation = %.2f", activation); 
            Push2DText(screenRenderGroup, fontRenderer, bodyPartPos, bodyPartInfo);
        }
    }

    ShaderInstance *blurShaderInstance = assets->blurShaderInstance;
    Camera2D shadowCamera;
    Vec2 shadowOffset = vec2(-5, -5);
    shadowCamera.pos = v2_sub(camera->pos, shadowOffset);
    shadowCamera.scale = camera->scale;
    shadowCamera.isYUp = camera->isYUp;
    UpdateCamera2D(&shadowCamera, appState);

    FrameBuffer *frameBuffer0 = screen->frameBuffer0;
    FrameBuffer *frameBuffer1 = screen->frameBuffer1;

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer0->fbo);
    glViewport(0, 0, frameBuffer0->width, frameBuffer0->height);
    glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
    renderTools->worldShader->mat3Transform = &shadowCamera.transform;
    ExecuteRenderGroup(worldRenderGroup, assets, renderTools->worldShader);
    renderTools->worldShader->mat3Transform = &camera->transform;

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer1->fbo);
    glClear(GL_COLOR_BUFFER_BIT);

    blurShaderInstance->mat3Transform = &camera->transform;
    blurShaderInstance->dir2 = vec2(1.0/camera->size.x,0);
    blurShaderInstance->radius = 1800;
    DrawDirectRect(assets->batch,
            blurShaderInstance,
            vec2(camera->pos.x-camera->size.x/2, camera->pos.y-camera->size.y/2),
            camera->size,
            frameBuffer0->colorTexture,
            vec2(0,1),
            vec2(1,-1),
            vec4(1, 1, 1, 0.4));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, appState->screenWidth, appState->screenHeight);

    blurShaderInstance->dir2 = vec2(0, 1.0/camera->size.y);
    DrawDirectRect(assets->batch,
            blurShaderInstance,
            vec2(camera->pos.x-camera->size.x/2, camera->pos.y-camera->size.y/2),
            camera->size,
            frameBuffer1->colorTexture,
            vec2(0,1),
            vec2(1,-1),
            vec4(0, 0, 0, 1));

    ExecuteAndFlushRenderGroup(worldRenderGroup, assets, renderTools->worldShader);
    ExecuteAndFlushRenderGroup(screenRenderGroup, assets, renderTools->screenShader);

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

        NKEditFloatPropertyWithTooltip(ctx, "Blur Radius", "How fast will genes adapt", 
                1.0, &assets->blurShaderInstance->radius, 100, 0.1, 0.1);

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
        Assets *assets,
        CreatureDefinition *def,
        ui32 nGenes,
        r32 dev,
        r32 learningRate)
{
    screen->renderTools = CreateBasicRenderTools(arena, appState, assets);
    ui32 shadowResolution = 512;
    screen->frameBuffer0 = CreateFrameBuffer(arena, shadowResolution, shadowResolution);
    screen->frameBuffer1 = CreateFrameBuffer(arena, shadowResolution, shadowResolution);

    screen->stepsPerFrame = 1;
    screen->generation = 0;
    screen->tick = 0;
    screen->ticksPerGeneration = 60*15;    // 10 seconds
    screen->avgFitness = -100000.0;

    // Settings
    screen->isPopulationVisible = 0;
    screen->isDragVisible = 0;

    // Init fake world
    screen->world = PushStruct(arena, FakeWorld);
    screen->evolutionArena = CreateSubArena(arena, 64L*1000L*1000L);
    InitFakeWorld(screen->world, arena, screen->evolutionArena, def, nGenes, learningRate, dev);

    ESGenerateGenes(screen->world->strategies);
}

