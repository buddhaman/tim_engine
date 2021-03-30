
internal inline b32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

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
    Gui *gui = screen->gui;

    AtlasRegion *circleRegion = defaultAtlas->regions;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;
    (void)circleRegion;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    glUseProgram(spriteShader->program);

    UpdateCameraInput(appState, camera);
    UpdateCamera2D(camera, appState);
    Vec2 mousePos = camera->mousePos;

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

    DrawGrid(batch, camera, 10, 0.5, squareRegion);

    // Check intersection with bodyparts.
    BodyPartDefinition *bodyPartIntersect = NULL;
    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(OrientedBoxPoint2Intersect(part->pos, vec2(part->width, part->height), part->angle, mousePos))
        {
            bodyPartIntersect = part;
        }
    }
    if(!CreatureEditorIsEditing(screen)
            && IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT))
    {
        screen->selectedBodyPart = bodyPartIntersect;
        if(screen->selectedBodyPart)
        {
            BodyPartDefinition *part = screen->selectedBodyPart;
            // Setup editing info
            r32 c = cosf(part->angle);
            r32 s = sinf(part->angle);
            screen->widthDragPos = v2_add(part->pos, vec2(c*part->width/2.0, s*part->width/2.0));
            screen->heightDragPos = v2_add(part->pos, vec2(-s*part->height/2.0, c*part->height/2.0));
        }
    }

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(screen->selectedBodyPart==part) batch->colorState = vec4(1, 0, 0, 1);
        else batch->colorState = vec4(1,1,1,1);
        PushOrientedRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                squareRegion);
    }

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(bodyPartIntersect==part) batch->colorState = vec4(1, 0, 1, 1);
        else batch->colorState = vec4(0, 0, 0, 1);
        PushOrientedLineRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                1.0,
                squareRegion);
    }

    if(screen->selectedBodyPart)
    {
        BodyPartDefinition *part = screen->selectedBodyPart;
        r32 c = cosf(part->angle);
        r32 s = sinf(part->angle);
        b32 hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                    &screen->widthDragPos, 
                    5, part->pos, vec2(c,s), screen->editState==EDIT_CREATURE_WIDTH);
        if(hitWidthDragButton) screen->editState = EDIT_CREATURE_WIDTH;
        if(screen->editState==EDIT_CREATURE_WIDTH)
        {
            part->width = v2_dist(part->pos, screen->widthDragPos)*2.0;
            if(!hitWidthDragButton) screen->editState = EDIT_CREATURE_NONE;
        }

        // TODO: turn into function
        b32 hitHeightDragButton = DoDragButtonAlongAxis(gui, 
                    &screen->heightDragPos, 
                    5, part->pos, vec2(-s,c), screen->editState==EDIT_CREATURE_HEIGHT);
        if(hitHeightDragButton) screen->editState = EDIT_CREATURE_HEIGHT;
        if(screen->editState==EDIT_CREATURE_HEIGHT)
        {
            part->height = v2_dist(part->pos, screen->heightDragPos)*2.0;
            if(!hitHeightDragButton) screen->editState = EDIT_CREATURE_NONE;
        }
    }

    EndSpritebatch(batch);
}

void
InitCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *screen, 
        RenderContext *renderContext,
        MemoryArena *arena)
{
    screen->camera = PushStruct(arena, Camera2D);
    InitCamera2D(screen->camera);
    screen->camera->isYUp = 1;

    screen->creatureDefinition = PushStruct(arena, CreatureDefinition);
    *screen->creatureDefinition = (CreatureDefinition){};
    
    screen->gui = PushStruct(arena, Gui);
    InitGui(screen->gui, appState, screen->camera, renderContext);

    DefineBodyPart(screen->creatureDefinition, vec2(0,0), vec2(500, 50), 0.5);
}

