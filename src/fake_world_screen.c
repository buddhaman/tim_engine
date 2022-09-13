
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

internal inline R32
GetBodyPartShade(BodyPart *part, R32 dragColorIntensity)
{
    return 1.0-part->body->drag*dragColorIntensity;
}

internal inline Rect2
DoSimpleTextLayout(RenderGroup *renderGroup, 
        FontRenderer *fontRenderer, 
        Vec2 pos, 
        Vec4 color,
        const char *fmt,
        ...)
{
    char buf[512];
    va_list args;

    va_start(args, fmt);
    vsnprintf(buf, 512, fmt, args);
    va_end(args);

    Rect2 size = GetStringSize(fontRenderer, buf, pos);
    Push2DTextColored(renderGroup, fontRenderer, V2(pos.x, pos.y+size.height), buf, color);
    return size;
}

internal inline void
DrawSolidRigidBody(RenderGroup *renderGroup, 
        RigidBody *body,
        AtlasRegion *texture,
        Vec4 color)
{
    Vec2 pos = GetBodyPos(body);
    R32 angle = GetBodyAngle(body);

    Push2DOrientedRectangleColored(renderGroup, 
            pos,
            V2(body->width, body->height),
            angle,
            texture,
            color);
}

void
DrawBodyPart(RenderGroup *renderGroup,
        BodyPart *part,
        Vec3 creatureColor,
        R32 alpha,
        R32 dragColorIntensity,
        AtlasRegion *texture)
{
    RigidBody *body = part->body;
    R32 shade = GetBodyPartShade(part, dragColorIntensity);
    Vec4 color = V4(shade*creatureColor.x, 
            shade*creatureColor.y, 
            shade*creatureColor.z, 
            alpha);
    DrawSolidRigidBody(renderGroup, body, texture, color);
}

internal inline void
DrawVecR32(RenderGroup *renderGroup,
        VecR32 *vec,
        Vec2 pos, 
        R32 size,
        Vec3 negativeColor,
        Vec3 positiveColor,
        R32 maxAbs,
        int xDir,
        int yDir,
        AtlasRegion *texture)
{
    for(U32 i = 0; i < vec->n; i++)
    {
        R32 v = fabsf(vec->v[i])/maxAbs;
        Vec4 color = vec->v[i] < 0 ? V4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : V4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        Push2DRectColored(renderGroup, V2(pos.x+i*xDir*size, pos.y+i*yDir*size), 
                V2(size, size), texture, color);
    }
}

internal inline void
DrawMatR32(RenderGroup *renderGroup,
        MatR32 *mat,
        Vec2 pos, 
        R32 size,
        Vec3 negativeColor,
        Vec3 positiveColor,
        R32 maxAbs,
        AtlasRegion *texture)
{
    for(U32 i = 0; i < mat->w; i++)
    for(U32 j = 0; j < mat->h; j++)
    {
        R32 m = mat->m[i+mat->w*j];
        R32 v = fabsf(m)/maxAbs;
        Vec4 color =  m < 0 ? V4(negativeColor.x*v, negativeColor.y*v, negativeColor.z*v, 1.0)
            : V4(positiveColor.x*v, positiveColor.y*v, positiveColor.z*v, 1.0);
        Push2DRectColored(renderGroup, V2(pos.x+i*size, pos.y+j*size), 
                V2(size, size), texture, color);
    }
}

internal inline Vec2
DrawBrainVector(RenderGroup *renderGroup, VecR32 *vec, Vec2 pos, R32 size, R32 maxAbs, int xDir, int yDir, AtlasRegion *texture)
{
    DrawVecR32(renderGroup, vec, pos, size, V3(1,0,0), V3(0,1,0), maxAbs, xDir, yDir, texture);
    R32 w = xDir!=0 ? vec->n*size*xDir : size;
    R32 h = yDir!=0 ? vec->n*size*yDir : size;
    return V2(pos.x+w, pos.y+h);
}

internal inline Vec2
DrawBrainMatrix(RenderGroup *renderGroup, MatR32 *mat, Vec2 pos, R32 size, R32 maxAbs, AtlasRegion *texture)
{
    DrawMatR32(renderGroup, mat, pos, size, V3(1,0,0), V3(0,1,0), maxAbs, texture);
    return V2(pos.x+mat->w*size, pos.y+mat->h*size);
}

void
DrawClock(RenderGroup *renderGroup,
        Vec2 pos, 
        R32 radius,
        R32 radians, 
        AtlasRegion *squareTexture,
        AtlasRegion *circleTexture)
{
    R32 lineWidth = 4;
    R32 c = cosf(radians);
    R32 s = sinf(radians);

    Push2DCircleColored(renderGroup, pos, radius+lineWidth, circleTexture, V4(0,0,0,1));
    Push2DCircleColored(renderGroup, pos, radius, circleTexture, V4(0.5, 0.5, 0.5, 1.0));
    Push2DLineColored(renderGroup, 
            pos, 
            V2Add(pos, V2(c*radius, s*radius)), 
            lineWidth, 
            squareTexture, V4(0, 0, 0, 1));
}

global_variable R32 gWindScale     = 0.1f;
global_variable R32 gWindAmplitude = 4.0f;
global_variable R32 gWindPeriod    = (2.0f*M_PI)/5.0f;

R32 
WindX(R32 x, R32 y, R32 time)
{
    return x+gWindAmplitude*cosf(gWindPeriod*time+(x+y)*gWindScale);
}

R32 
WindY(R32 x, R32 y, R32 time)
{
    return y+gWindAmplitude*sinf(gWindPeriod*time+(x+y)*gWindScale);
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

    Vec4 groundColor = RGBAToVec4(0x996437FF);

    Push2DCircleColored(renderGroup, V2(0, 0), 3, circleRegion, V4(1,1,1,0.5));

    if(screen->isPopulationVisible)
    {
        for(U32 creatureIdx = 1;
                creatureIdx < world->nCreatures;
                creatureIdx++)
        {
            Creature *creature = world->creatures+creatureIdx;
            Vec3 creatureColor = creature->solidColor;
            R32 alpha = 0.4f;
            for(U32 bodyPartIdx = 0;
                    bodyPartIdx < creature->nBodyParts;
                    bodyPartIdx++)
            {
                BodyPart *part = creature->bodyParts+bodyPartIdx;
                DrawBodyPart(renderGroup, part, creatureColor, alpha, 1.0, squareRegion);
            }
        }
    }


    local_persist R32 time = 0.0f;
    time+=(1.0f/60.0f);
    Vec4 grassColor = V4(0.0f, 1.0f, 0.0f, 1.0f);
    R32 r = 7.0f;
    Vec4 tallGrassColor = ShadeRGB(grassColor, 0.8f);
    Vec4 bushColor = ShadeRGB(tallGrassColor, 0.8f);

    for(I32 bushIdx = 0;
            bushIdx < world->nBushes;
            bushIdx++)
    {
        Bush *bush = &world->bushes[bushIdx];
        for(int i = 0; i < bush->nLeafs; i++)
        {
            Vec2 pos = V2(
                    WindX(bush->leafs[i].x, bush->leafs[i].y, time),
                    WindY(bush->leafs[i].x, bush->leafs[i].y, time)
                    );
            Push2DCircleColored(renderGroup,
                    pos,
                    bush->r[i],
                    circleRegion,
                    bushColor);
        }
    }

    for(I32 grassIdx = 0;
            grassIdx < world->nTallGrass;
            grassIdx++)
    {
        TallGrass *grass = &world->tallGrass[grassIdx];
        Vec2 from = grass->from;

        for(int i = 0; i < grass->nBlades; i++)
        {
            Vec2 to = V2(
                    WindX(grass->to[i].x, grass->to[i].y, time),
                    WindY(grass->to[i].x, grass->to[i].y, time)
                    );
            Push2DLineColored(renderGroup,
                    from,
                    to,
                    r*2,
                    squareRegion,
                    tallGrassColor);
            Push2DCircleColored(renderGroup,
                    to,
                    r,
                    circleRegion,
                    tallGrassColor);
        }
    }

    for(I32 platformIdx = 0;
            platformIdx < world->nStaticPlatforms;
            platformIdx++)
    {
        StaticPlatform *platform = &world->staticPlatforms[platformIdx];

        Push2DOrientedRectangleColored(renderGroup, 
                platform->bounds.pos, 
                platform->bounds.dims,
                0.0f, 
                squareRegion,
                groundColor);
    }

    for(I32 grassIdx = 0;
            grassIdx < world->nGrass;
            grassIdx++)
    {
        Grass *grass = &world->grass[grassIdx];

        Push2DRectColored(renderGroup, 
                grass->pos, 
                V2(grass->width, grass->topHeight),
                squareRegion, 
                grassColor);

        R32 x0 = WindX(grass->pos.x, grass->pos.y, time);
        R32 y0 = WindY(grass->pos.x, grass->pos.y, time);

        R32 x1 = WindX(grass->pos.x+grass->width, grass->pos.y, time);

        Push2DRectColored(renderGroup, 
                V2(x0, y0-grass->ovalHeight/2.0f),
                V2(x1-x0, grass->ovalHeight),
                circleRegion, 
                grassColor);
    }
#if 1

#endif

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
            R32 dragFactor = screen->isDragVisible ? 1.0 :  0.0;
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
        R32 angle = GetBodyAngle(body);
        R32 shade = screen->isDragVisible ? GetBodyPartShade(part, 1.0) : 1.0;
        Vec4 color = V4(shade, shade, shade, 1.0);
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
        UpdateCameraDragInput(appState, camera);
        UpdateCameraScrollInput(appState, camera);
        UpdateCameraKeyMovementInput(appState, camera);

        Rect2 bounds;
        bounds.pos = world->origin;
        bounds.dims = world->size;
        if(camera->scale > 2) camera->scale = 2;
        UpdateCameraSize(camera, appState);
        KeepCameraInBounds(camera, bounds);
    }

    if(!IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT))
    {
        CameraStopDragging(camera);
    }

    // Do evolution
    if(!screen->isPaused)
    {
        for(U32 atFrameStep = 0;
                atFrameStep < screen->stepsPerFrame;
                atFrameStep++)
        {
            UpdateFakeWorld(world);
            screen->tick++;
            if(screen->tick >= screen->ticksPerGeneration)
            {
                // Next Generation
                for(U32 geneIdx = 0;
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

    // Draw background TODO: Turn into function.
    Vec4 topColor    = appState->clearColor;
    Vec4 bottomColor = V4(1.0, 1.0, 1.0, 1.0);
    Mesh2D *batch = assets->batch;
    BeginMesh2D(batch);
    BeginShaderInstance(renderTools->worldShader);
    U16 lastIdx = batch->nVertices;
    Vec2 size = world->size;
    Vec2 origin = world->origin;
    Vec2 p0 = V2(origin.x, origin.y);
    Vec2 p1 = V2(origin.x+size.x, origin.y);
    Vec2 p2 = V2(origin.x+size.x, origin.y+size.y);
    Vec2 p3 = V2(origin.x, origin.y+size.y);
    Vec2 texOrig = squareRegion->pos;
    Vec2 texSize = squareRegion->size;
    PushVertex2(batch, p0, V2(texOrig.x, texOrig.y+texSize.y), bottomColor);
    PushVertex2(batch, p1, V2(texOrig.x+texSize.x, texOrig.y+texSize.y), bottomColor);
    PushVertex2(batch, p2, V2(texOrig.x+texSize.x, texOrig.y), topColor);
    PushVertex2(batch, p3, texOrig, topColor);
    PushIndex(batch, lastIdx);
    PushIndex(batch, lastIdx+1);
    PushIndex(batch, lastIdx+2);
    PushIndex(batch, lastIdx+2);
    PushIndex(batch, lastIdx+3);
    PushIndex(batch, lastIdx);
    Assert(batch->nIndices < batch->maxVertices);
    EndMesh2D(batch);

    DrawFakeWorld(screen, worldRenderGroup, camera, defaultAtlas, assets->creatureTextureAtlas);

    if(world->trainingType==TRAIN_DISTANCE_TARGET)
    {
        Vec4 circleColor = V4(1.0, 1.0, 0.0, 1.0);
        Push2DCircleColored(worldRenderGroup, world->target, 10, circleRegion, circleColor);
    }

    // Render on screen

    R32 screenWidth = screenCamera->size.x;
    R32 screenHeight = screenCamera->size.y;
    R32 bottomBarHeight = 50;

    Push2DRectColored(screenRenderGroup,
            V2(0,screenHeight-bottomBarHeight),
            V2(screenWidth, bottomBarHeight),
            squareRegion,
            appState->clearColor);

    Push2DRectColored(screenRenderGroup,
            V2(0, screenHeight-bottomBarHeight),
            V2(screenWidth, 2),
            squareRegion,
            V4(1,1,1,1));

    if(screen->selectedCreature)
    {
        Creature *creature = screen->selectedCreature;
        MinimalGatedUnit *brain = creature->brain;

        R32 size = 10.0;
        R32 maxAbs = 1.0;
        R32 pad = 3.0;
        R32 totalBrainWidth = (brain->Wf.w+brain->Uf.w+1)*size+pad*2;
        R32 leftX = screenWidth - totalBrainWidth - 10;
        R32 atX = leftX;
        R32 atY = 10;
        R32 maxRowY = 0;
        Vec2 lastPos;

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Wf, V2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Uf, V2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(screenRenderGroup, &brain->bf, V2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Wh, V2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainMatrix(screenRenderGroup, &brain->Uh, V2(atX, atY), size, maxAbs, squareRegion);
        atX = lastPos.x+pad;
        maxRowY = Max(maxRowY, lastPos.y);

        lastPos = DrawBrainVector(screenRenderGroup, &brain->bh, V2(atX, atY), size, maxAbs, 0, 1, squareRegion);
        atX = leftX;
        atY = maxRowY+pad;
        maxRowY = 0;

        DrawBrainVector(screenRenderGroup, &brain->x, V2(atX, atY), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->h, V2(atX, atY+size), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->hc, V2(atX, atY+size*2), size, 1.0, 1, 0, squareRegion);
        DrawBrainVector(screenRenderGroup, &brain->f, V2(atX, atY+size*3), size, 1.0, 1, 0, squareRegion);
        atY+=size*4+pad*3;

        // Draw clocks
        R32 clockRadius = 20.0f;
        for(U32 clockIdx = 0;
                clockIdx < creature->nInternalClocks;
                clockIdx++)
        {
            R32 radians = GetInternalClockValue(creature, clockIdx);
            DrawClock(screenRenderGroup, 
                    V2(atX+clockRadius+clockRadius*2*clockIdx+pad*2*clockIdx, atY+clockRadius), 
                    clockRadius, radians, squareRegion, circleRegion);
        }
        atY += (clockRadius*2+pad*2);

        // Draw inputs
        BodyPart *selectedBodyPart = screen->selectedBodyPart;
        R32 lineWidth = 2.0;

        if(selectedBodyPart)
        {
            Vec4 textColor = V4(0,0,0,1);
            R32 angle = GetBodyAngle(selectedBodyPart->body);
            Vec2 dir = V2Polar(-angle, 200.0);
            Vec2 pos = CameraToScreenPos(camera, appState, GetBodyPos(selectedBodyPart->body));
            Vec2 to = V2Add(pos, dir);
            Push2DLineColored(screenRenderGroup, 
                    pos, 
                    to, 
                    lineWidth,
                    squareRegion, 
                    V4(1, 0, 0, 1));

            R32 startDebugX = screenWidth - 300.0f;
            atX = startDebugX;

            Rect2 size = DoSimpleTextLayout(screenRenderGroup, 
                    fontRenderer, 
                    V2(atX, atY), 
                    textColor,
                    "Angle (absolute): %.2f", angle);
            atY+=(size.height+pad);
            atX=startDebugX;

            if(selectedBodyPart->parent)
            {
                BodyPart *parent = selectedBodyPart->parent;
                R32 parentAngle = GetBodyAngle(parent->body);
                R32 relativeAngle = angle - parentAngle;
                pos = CameraToScreenPos(camera, appState, GetBodyPos(parent->body));
                dir = V2Polar(-parentAngle, 200.0f);
                to = V2Add(pos, dir);

                Push2DLineColored(screenRenderGroup, 
                        pos, 
                        to, 
                        lineWidth,
                        squareRegion, 
                        V4(1, 0, 1, 1));

                size = DoSimpleTextLayout(screenRenderGroup, 
                        fontRenderer, 
                        V2(atX, atY), 
                        textColor,
                        "Angle (relative): %.2f", relativeAngle);
                atY+=(size.height+pad);
                atX=startDebugX;
            }
        }
    }

    // Only text from here
    char info[512];

    sprintf(info, "Steps per frame: %u. At Generation %u (%u/%u) fitness = %f", screen->stepsPerFrame,
            screen->generation, 
            screen->tick, 
            screen->ticksPerGeneration, 
            screen->avgFitness);
    Push2DText(screenRenderGroup, fontRenderer, V2(20, screenHeight-bottomBarHeight/2+8), info);

    R32 dx = appState->mx - appState->mousePressedAtX;
    R32 dy = appState->my - appState->mousePressedAtY;
    R32 mouseDiff2 = dx*dx + dy*dy;
    if(IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT) && 
            (!screen->isGuiInputCaptured) &&
            (mouseDiff2 < 8.0f) )
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

    ShaderInstance *blurShaderInstance = assets->blurShaderInstance;
    Camera2D shadowCamera;
    Vec2 shadowOffset = V2(-5, -5);
    shadowCamera.pos = V2Sub(camera->pos, shadowOffset);
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

    R32 invScreenX = 1.0/camera->size.x;
    R32 invScreenY = 1.0/camera->size.y;
    blurShaderInstance->mat3Transform = &camera->transform;
    blurShaderInstance->dir2 = V2(invScreenX, invScreenY);
    blurShaderInstance->radius = 1800;
    DrawDirectRect(assets->batch,
            blurShaderInstance,
            V2(camera->pos.x-camera->size.x/2, camera->pos.y-camera->size.y/2),
            camera->size,
            frameBuffer0->colorTexture,
            V2(0,1),
            V2(1,-1),
            V4(1, 1, 1, 1.0));

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, appState->screenWidth, appState->screenHeight);

    blurShaderInstance->dir2 = V2(-invScreenY, invScreenX);
    DrawDirectRect(assets->batch,
            blurShaderInstance,
            V2(camera->pos.x-camera->size.x/2, camera->pos.y-camera->size.y/2),
            camera->size,
            frameBuffer1->colorTexture,
            V2(0,1),
            V2(1,-1),
            V4(0, 0, 0, 0.4));

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

        R32 parameterScale = 1000.0;

        R32 deviationScaled = parameterScale * world->strategies->dev;
        NKEditFloatPropertyWithTooltip(ctx, "Mutation Rate", "How much is each creature mutated", 
                1.0, &deviationScaled, parameterScale, 1.0, 1.0);
        world->strategies->dev = deviationScaled/parameterScale;

        R32 learningRateScaled = parameterScale*world->strategies->learningRate;
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
        Assets *assets,
        CreatureDefinition *def,
        U32 nGenes,
        R32 dev,
        R32 learningRate)
{
    screen->renderTools = CreateBasicRenderTools(arena, appState, assets);
    U32 shadowResolution = 512;
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

