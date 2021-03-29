
void
UpdateCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *screen, 
        RenderContext *renderer, 
        struct nk_context *ctx) 
{
    Camera2D *camera = screen->camera;
    Camera2D *screenCamera = appState->screenCamera;    // Mildly confusing
    (void) screenCamera;
    SpriteBatch *batch = renderer->batch;
    FontRenderer *fontRenderer = renderer->fontRenderer;
    (void) fontRenderer;
    TextureAtlas *defaultAtlas = renderer->defaultAtlas;
    (void) defaultAtlas;
    Shader *spriteShader = renderer->spriteShader;
    CreatureDefinition *def = screen->creatureDefinition;

    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;
    (void)circleRegion;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    glUseProgram(spriteShader->program);

    UpdateCameraInput(appState, camera);
    UpdateCamera2D(camera, appState);
    int matLocation = glGetUniformLocation(spriteShader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);
    BeginSpritebatch(batch);
    local_persist r32 time = 0.0;
    time+=1.0/60;

    batch->colorState = vec4(1,1,1,0.1);
    PushRect2(batch, 
            vec2(-10, -10),
            vec2(20, 20),
            circleRegion->pos,
            circleRegion->size);
    DrawGrid(batch, camera, 50, 2, squareRegion);

    for(ui32 bodypartIdx = 0; 
            bodypartIdx < def->nBodyParts;
            bodypartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodypartIdx;
        batch->colorState = vec4(1, 1, 0, 1);
        PushOrientedRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                squareRegion);
    }

    for(ui32 bodypartIdx = 0; 
            bodypartIdx < def->nBodyParts;
            bodypartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodypartIdx;
        batch->colorState = vec4(0, 0, 0, 1);
        PushOrientedLineRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                1.0,
                squareRegion);
    }

    EndSpritebatch(batch);
}

void
InitCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *screen, 
        MemoryArena *arena)
{
    screen->camera = PushStruct(arena, Camera2D);
    InitCamera2D(screen->camera);
    screen->camera->isYUp = 1;

    screen->creatureDefinition = PushStruct(arena, CreatureDefinition);
    *screen->creatureDefinition = (CreatureDefinition){};
    DefineGuy(screen->creatureDefinition);
}

