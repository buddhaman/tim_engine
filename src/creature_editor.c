
internal inline b32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

void
UpdateCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        RenderContext *renderer, 
        struct nk_context *ctx) 
{
    Camera2D *camera = editor->camera;
    Camera2D *screenCamera = appState->screenCamera;    // Mildly confusing
    (void) screenCamera;
    SpriteBatch *batch = renderer->batch;
    FontRenderer *fontRenderer = renderer->fontRenderer;
    (void) fontRenderer;
    TextureAtlas *defaultAtlas = renderer->defaultAtlas;
    (void) defaultAtlas;
    Shader *spriteShader = renderer->spriteShader;
    CreatureDefinition *def = editor->creatureDefinition;
    Gui *gui = editor->gui;

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

    DrawGrid(batch, camera, 50, 1.0, squareRegion);

    // Check intersection with bodyparts.
    BodyPartDefinition *bodyPartIntersect = NULL;
    if(!CreatureEditorIsEditing(editor))
    {
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
        if(IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT))
        {
            editor->selectedBodyPart = bodyPartIntersect;
        }
    }

    char info[256];
    memset(info, 0, 256);
    if(editor->editState==EDIT_ADD_BODYPART_FIND_EDGE)
    {
        r32 minDist = 10000000.0;
        BoxEdgeLocation location = {};
        BoxEdgeLocation minLocation = {};
        b32 hasLocation = 0;
        BodyPartDefinition *attachTo = NULL;
        for(ui32 bodyPartIdx = 0;
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            r32 dist = GetNearestBoxEdgeLocation(part->pos, vec2(part->width, part->height), part->angle, mousePos, &location);
            if(dist > 0 
                    && dist < minDist)
            {
                minDist = dist;
                minLocation = location;
                hasLocation = 1;
                attachTo = part;
            }
        }
        if(hasLocation)
        {
            batch->colorState = vec4(0, 1.0, 0.0, 1.0);
            PushCircle2(batch, minLocation.pos, 3, circleRegion);
            sprintf(info, "offset = %.2f, (%d, %d)", minLocation.offset, minLocation.xEdge, minLocation.yEdge);
            if(IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT))
            {
                editor->bodyPartLocation = minLocation;
                editor->attachTo = attachTo;
                editor->editState=EDIT_ADD_BODYPART_PLACE;
            }
        }
    } else if(editor->editState==EDIT_ADD_BODYPART_PLACE)
    {
        BodyPartDefinition *part = editor->attachTo;
        BoxEdgeLocation *location = &editor->bodyPartLocation;
        batch->colorState = vec4(1.0, 1.0, 1.0, 0.4);
        Vec2 from = editor->bodyPartLocation.pos;
        Vec2 to = mousePos;
        Vec2 center = v2_muls(v2_add(from, to), 0.5);
        r32 angle = atan2f(to.y-from.y, to.x-from.x);
        r32 dist = v2_dist(from, to);
        PushOrientedRectangle2(batch,
                center,
                dist,
                20,
                angle,
                squareRegion);
        r32 edgeAngle = atan2f(location->yEdge, location->xEdge);
        r32 localAngle = angle-part->angle - edgeAngle;
        localAngle = NormalizeAngle(localAngle);
        sprintf(info, "%.1f deg", RadToDeg(localAngle));
        if(IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT))
        {
            BodyPartDefinition *newPart = def->bodyParts+def->nBodyParts++;
            
            newPart->id = editor->idCounter++;
            newPart->pos = center;
            newPart->width = dist;
            newPart->height = 20;
            newPart->angle = angle;
            newPart->localAngle = localAngle;
            newPart->connectionId = part->id;
            newPart->offset = location->offset;
            newPart->xEdge = location->xEdge;
            newPart->yEdge = location->yEdge;
            newPart->pivotPoint = location->pos;
            newPart->minAngle = -1;
            newPart->maxAngle = 1;
            editor->editState=EDIT_CREATURE_NONE;
        }
    }

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(editor->selectedBodyPart==part) batch->colorState = vec4(1, 0, 0, 1);
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

    if(editor->selectedBodyPart)
    {
        BodyPartDefinition *part = editor->selectedBodyPart;

        r32 halfWidth = part->width/2;
        r32 halfHeight = part->height/2;
        r32 angleButtonLength = sqrtf(halfWidth*halfWidth+halfHeight*halfHeight);
        r32 angleButtonAngle = part->angle+atan2(-halfHeight, -halfWidth);
        b32 hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                    part->pos, 
                    v2_polar(part->angle, 1.0),
                    &halfWidth,
                    5, editor->editState==EDIT_CREATURE_WIDTH);
        part->width = Max(1, halfWidth*2);

        b32 hitHeightDragButton = DoDragButtonAlongAxis(gui, 
                    part->pos, 
                    v2_polar(part->angle+M_PI/2, 1.0),
                    &halfHeight,
                    5, editor->editState==EDIT_CREATURE_HEIGHT);
        part->height = Max(1, halfHeight*2);

        r32 newAngle = angleButtonAngle;
        b32 hitRotationDragButton = DoRotationDragButton(gui, part->pos,
            &newAngle, angleButtonLength, 5, editor->editState==EDIT_CREATURE_ROTATION);
        part->angle+=newAngle-angleButtonAngle;

        if(!CreatureEditorIsEditing(editor))
        {
            if(hitWidthDragButton) editor->editState = EDIT_CREATURE_WIDTH;
            if(hitHeightDragButton) editor->editState = EDIT_CREATURE_HEIGHT;
            if(hitRotationDragButton) editor->editState = EDIT_CREATURE_ROTATION;
        }

        if(editor->editState==EDIT_CREATURE_HEIGHT)
        {
            if(!hitHeightDragButton) editor->editState = EDIT_CREATURE_NONE;
        }
        if(editor->editState==EDIT_CREATURE_WIDTH)
        {
            if(!hitWidthDragButton) editor->editState = EDIT_CREATURE_NONE;
        }
        if(editor->editState==EDIT_CREATURE_ROTATION)
        {
            if(!hitRotationDragButton) editor->editState = EDIT_CREATURE_NONE;
        }
        
        RecalculateSubNodeBodyParts(def, part);

    }

    EndSpritebatch(batch);

    // Only text from here
    BeginSpritebatch(batch);
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&screenCamera->transform);
    glBindTexture(GL_TEXTURE_2D, fontRenderer->font12Texture);

    batch->colorState=vec4(1,1,1,1);
    if(info[0])
    {
        DrawString2D(batch, fontRenderer, screenCamera->mousePos, info);
    }

    EndSpritebatch(batch);

    // Begin UI
    if(nk_begin(ctx, "Editor", nk_rect(50, 50, 220, 220),
            NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE))
    {
        nk_layout_row_static(ctx, 20, 220, 1);
        nk_labelf(ctx,  NK_TEXT_LEFT, "heyo: %f", 12.3);
        if(nk_button_label(ctx, "Add Bodypart"))
        {
            editor->editState = EDIT_ADD_BODYPART_FIND_EDGE;
        }
    }
    nk_end(ctx);
}

void
InitCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        RenderContext *renderContext,
        MemoryArena *arena)
{
    editor->camera = PushStruct(arena, Camera2D);
    InitCamera2D(editor->camera);
    editor->camera->isYUp = 1;

    editor->creatureDefinition = PushStruct(arena, CreatureDefinition);
    *editor->creatureDefinition = (CreatureDefinition){};
    
    editor->gui = PushStruct(arena, Gui);
    InitGui(editor->gui, appState, editor->camera, renderContext);

    //DefineGuy(editor->creatureDefinition);
    editor->idCounter = 1;
    BodyPartDefinition *torso = editor->creatureDefinition->bodyParts+editor->creatureDefinition->nBodyParts++;
    torso->id = editor->idCounter++;
    torso->pos = vec2(0,0);
    torso->width = 50;
    torso->height = 50;
}

