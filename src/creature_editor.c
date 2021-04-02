
internal inline b32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

b32
EditorIsMouseJustPressed(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
EditorIsMouseJustReleased(AppState *appState, CreatureEditorScreen *editor)
{
    return IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
EditorIsMousePressed(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT);
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

    Vec4 creatureColor = RGBAToVec4(0x7c4d35ff);
    
    // Key and mouse input
    editor->isInputCaptured = nk_window_is_any_hovered(ctx);
    if(IsKeyActionJustDown(appState, ACTION_ESCAPE))
    {
        editor->editState = EDIT_CREATURE_NONE;
    }

    UpdateCameraInput(appState, camera);
    UpdateCamera2D(camera, appState);
    Vec2 mousePos = camera->mousePos;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    glUseProgram(spriteShader->program);

    int matLocation = glGetUniformLocation(spriteShader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);
    BeginSpritebatch(batch);

    batch->colorState = vec4(1,1,1,0.1);
    PushRect2(batch, 
            vec2(-10, -10),
            vec2(20, 20),
            circleRegion->pos,
            circleRegion->size);

    DrawGrid(batch, camera, 50, 2.0, squareRegion);

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
    }

    // TODO: collect in editor and display at the same time. Just like spritebatch.
    char info[256];
    memset(info, 0, 256);

    char angleInfo[256];
    memset(angleInfo, 0, 256);
    Vec2 angleInfoPos = vec2(0,0);

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
        if(EditorIsMouseJustReleased(appState, editor))
        {
            if(editor->isInputCaptured)
            {
                editor->editState=EDIT_CREATURE_NONE;
            }
            else
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
                editor->editState=EDIT_ADD_BODYPART_FIND_EDGE;
            }
        }
    }

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(editor->selectedBodyPart==part) batch->colorState = vec4(1, 0, 0, 1);
        else batch->colorState = creatureColor;
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

        b32 hitWidthDragButton = 0;
        b32 hitHeightDragButton = 0;
        b32 hitRotationDragButton = 0;
        b32 hitRotationAndLengthDragButton = 0;

        r32 halfWidth = part->width/2;
        r32 halfHeight = part->height/2;
        r32 angleButtonLength = sqrtf(halfWidth*halfWidth+halfHeight*halfHeight);
        r32 angleButtonAngle = part->angle+atan2(-halfHeight, -halfWidth);

        hitHeightDragButton = DoDragButtonAlongAxis(gui, 
                    part->pos, 
                    v2_polar(part->angle+M_PI/2, 1.0),
                    &halfHeight,
                    5, 
                    editor->editState==EDIT_CREATURE_HEIGHT);
        part->height = Max(1, halfHeight*2);

        if(!part->connectionId)
        {
            hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                        part->pos, 
                        v2_polar(part->angle, 1.0),
                        &halfWidth,
                        5, 
                        editor->editState==EDIT_CREATURE_WIDTH);
            part->width = Max(1, halfWidth*2);

            r32 newAngle = angleButtonAngle;
            hitRotationDragButton = DoRotationDragButton(gui, 
                    part->pos,
                &newAngle, 
                angleButtonLength, 
                5, 
                editor->editState==EDIT_CREATURE_ROTATION);
            part->angle+=newAngle-angleButtonAngle;
        }
        else
        {
            BodyPartDefinition *parent = GetBodyPartById(def, part->connectionId);
            Vec2 pivotPoint = part->pivotPoint;
            r32 absAngle = part->angle;
            r32 width = part->width;
            hitRotationAndLengthDragButton = DoAngleLengthButton(gui,
                    pivotPoint, 
                    &absAngle,
                    &width,
                    5,
                    editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH);
            part->width = width;
            //part->pos = v2_add(pivotPoint, v2_polar(absAngle, width/2.0));
            SetLocalAngleFromAbsoluteAngle(def, part, absAngle);
            // Draw nice angle thing
            Vec2 to = v2_add(pivotPoint, v2_polar(absAngle, width));
            r32 edgeAngle = GetAbsoluteEdgeAngle(parent, part->xEdge, part->yEdge);
            Vec2 normalPoint = v2_add(pivotPoint, v2_polar(edgeAngle, 50));
            sprintf(angleInfo, "%.1f deg", RadToDeg(part->localAngle));
            angleInfoPos = v2_add(pivotPoint, vec2(10, 10));

            batch->colorState = vec4(1,1,1,1);
            PushLine2(batch, pivotPoint, to, 1, squareRegion->pos, squareRegion->size);
            PushLine2(batch, pivotPoint, normalPoint, 1, squareRegion->pos, squareRegion->size);
            batch->colorState = vec4(1,1,1,0.5);
            // Turn this into a function
            r32 angDiff = GetNormalizedAngDiff(absAngle, edgeAngle);
            PushSemiCircle2(batch, pivotPoint, 20, edgeAngle, edgeAngle+angDiff, 20, squareRegion);
        }

        if(editor->editState==EDIT_CREATURE_HEIGHT &&
            !hitHeightDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_WIDTH 
                && !hitWidthDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_ROTATION
                && !hitRotationDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH
                && !hitRotationAndLengthDragButton) editor->editState = EDIT_CREATURE_NONE;

        if(!CreatureEditorIsEditing(editor))
        {
            if(hitWidthDragButton) {
            editor->editState = EDIT_CREATURE_WIDTH;
                DebugOut("start editing width");
            }
            if(hitHeightDragButton) editor->editState = EDIT_CREATURE_HEIGHT;
            if(hitRotationDragButton) editor->editState = EDIT_CREATURE_ROTATION;
            if(hitRotationAndLengthDragButton) editor->editState = EDIT_CREATURE_ROTATION_AND_LENGTH;
        }
        
        RecalculateSubNodeBodyParts(def, def->bodyParts);
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
    if(angleInfo[0])
    {
        Vec2 pos = CameraToScreenPos(camera, appState, angleInfoPos);
        DrawString2D(batch, fontRenderer, 
                pos,
                angleInfo);
    }

    EndSpritebatch(batch);

    // Begin UI
    //r32 screenWidth = screenCamera->size.x;
    r32 screenHeight = screenCamera->size.y;
    if(nk_begin(ctx, "Editor", nk_rect(5, 5, 300, screenHeight/2-10), 
                NK_WINDOW_TITLE | 
                NK_WINDOW_BORDER | 
                NK_WINDOW_MOVABLE | 
                NK_WINDOW_SCALABLE | 
                NK_WINDOW_NO_SCROLLBAR))
    {
        nk_layout_row_static(ctx, 30, 200, 1);
        if(nk_button_label(ctx, "Add Bodypart"))
        {
            editor->editState = EDIT_ADD_BODYPART_FIND_EDGE;
        }
        if(editor->editState==EDIT_ADD_BODYPART_FIND_EDGE)
        {
            if(nk_button_label(ctx, "Cancel"))
            {
                editor->editState = EDIT_CREATURE_NONE;
            }
        }
    }
    nk_end(ctx);

    if(!CreatureEditorIsEditing(editor) 
            && EditorIsMousePressed(appState, editor))
    {
        editor->selectedBodyPart = bodyPartIntersect;
    }
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

