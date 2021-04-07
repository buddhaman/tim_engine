
internal inline b32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

b32 
EditorCanAddBodyPart(CreatureEditorScreen *editor)
{
    return editor->creatureDefinition->nBodyParts < MAX_BODYPARTS;
}

b32
EditorIsMouseJustPressed(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
EditorIsMouseJustReleased(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionJustReleased(appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
EditorIsMousePressed(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT);
}

r32
SnapTo(r32 value, r32 res)
{
    return round(value/res)*res;
}

r32
SnapDim(CreatureEditorScreen *editor, r32 dim)
{
    if(editor->isDimSnapEnabled)
    {
        return SnapTo(dim, editor->dimSnapResolution);
    }
    else
    {
        return dim;
    }
}

r32
SnapAngle(CreatureEditorScreen *editor, r32 angle)
{
    if(editor->isAngleSnapEnabled)
    {
        return SnapTo(angle, editor->angleSnapResolution);
    }
    else
    {
        return angle;
    }
}

r32
SnapEdge(CreatureEditorScreen *editor, r32 offset)
{
    if(editor->isEdgeSnapEnabled)
    {
        return SnapTo(offset, 1.0/editor->edgeSnapDivisions);
    }
    else
    {
        return offset;
    }
}

void
EditorRemoveBodyPart(CreatureEditorScreen *editor, ui32 id)
{
    RemoveBodyPart(editor->creatureDefinition, editor->selectedId);
    editor->selectedId = 0;
    AssignBrainIO(editor->creatureDefinition);
}

b32
NKEditFloatProperty(struct nk_context *ctx, 
        char *label, 
        r32 min, 
        r32 *value, 
        r32 max, 
        r32 stepSize, 
        r32 incPerPixel)
{
    r32 v = *value;
    nk_property_float(ctx, label, min, &v, max, stepSize, incPerPixel);
    if(v!=*value)
    {
        *value = v;
        return 1;
    }
    return 0;
}

b32
NKEditRadInDegProperty(struct nk_context *ctx, 
        char *label, 
        r32 minRad, 
        r32 *rad, 
        r32 maxRad, 
        r32 stepSize, 
        r32 incPerPixel)
{
    r32 valInDeg = RadToDeg(*rad);
    r32 v = valInDeg;
    nk_property_float(ctx, label, RadToDeg(minRad), &v, RadToDeg(maxRad), stepSize, incPerPixel);
    if(v!=valInDeg)
    {
        *rad = DegToRad(v);
        return 1;
    }
    return 0;
}

b32 
NKEditCreatureIO(struct nk_context *ctx,
        char *label,
        b32 *value,
        ui32 brainIdx)
{
    b32 edited = 0;
    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
    nk_layout_row_push(ctx, 0.5);
    if(nk_checkbox_label(ctx, label, value))
        edited = 1;
    if(*value)
    {
        nk_labelf(ctx, NK_TEXT_LEFT, "Index = %u", brainIdx);
    }
    nk_layout_row_end(ctx);
    return edited;
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
    ui32 intersectId;
    if(!CreatureEditorIsEditing(editor))
    {
        for(ui32 bodyPartIdx = 0; 
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            if(OrientedBoxPoint2Intersect(part->pos, vec2(part->width, part->height), part->angle, mousePos))
            {
                intersectId = part->id;
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
        if(EditorCanAddBodyPart(editor)) 
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
                minLocation.offset = SnapEdge(editor, minLocation.offset);
                minLocation.pos = GetBoxEdgePosition(attachTo->pos, 
                        vec2(attachTo->width, attachTo->height), 
                        attachTo->angle, 
                        minLocation.xEdge, 
                        minLocation.yEdge, 
                        minLocation.offset);
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
        }
        else
        {
            editor->editState=EDIT_CREATURE_NONE;
        }
    } else if(editor->editState==EDIT_ADD_BODYPART_PLACE)
    {
        BodyPartDefinition *part = editor->attachTo;
        BoxEdgeLocation *location = &editor->bodyPartLocation;
        Vec2 from = editor->bodyPartLocation.pos;
        Vec2 to = mousePos;
        r32 angle = atan2f(to.y-from.y, to.x-from.x);
        r32 dist = SnapDim(editor, v2_dist(from, to));
        r32 edgeAngle = atan2f(location->yEdge, location->xEdge);
        r32 localAngle = angle-part->angle - edgeAngle;
        localAngle = SnapAngle(editor, NormalizeAngle(localAngle));
        angle = NormalizeAngle(edgeAngle+part->angle)+localAngle;
        Vec2 center = v2_add(from, v2_polar(angle, dist/2));

        sprintf(info, "%.1f deg", RadToDeg(localAngle));
        batch->colorState = vec4(1.0, 1.0, 1.0, 0.4);
        PushOrientedRectangle2(batch,
                center,
                dist,
                20,
                angle,
                squareRegion);
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
                
                newPart->hasDragOutput = 1;
                newPart->hasRotaryMuscleOutput = 1;

                AssignBrainIO(def);
                editor->editState=EDIT_ADD_BODYPART_FIND_EDGE;
            }
        }
    }

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(editor->selectedId==part->id) batch->colorState = vec4(1, 0, 0, 1);
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
        if(intersectId==part->id) batch->colorState = vec4(1, 0, 1, 1);
        else batch->colorState = vec4(0, 0, 0, 1);
        PushOrientedLineRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                1.0,
                squareRegion);
    }

    if(editor->selectedId)
    {
        BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);

        b32 hitWidthDragButton = 0;
        b32 hitHeightDragButton = 0;
        b32 hitRotationDragButton = 0;
        b32 hitRotationAndLengthDragButton = 0;
        b32 hitMinAngleDragButton = 0;
        b32 hitMaxAngleDragButton = 0;

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
        part->height = SnapDim(editor, Max(1.0, halfHeight*2));

        if(!part->connectionId)
        {
            hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                        part->pos, 
                        v2_polar(part->angle, 1.0),
                        &halfWidth,
                        5, 
                        editor->editState==EDIT_CREATURE_WIDTH);
            part->width = SnapDim(editor, Max(1, halfWidth*2));

            r32 newAngle = angleButtonAngle;
            hitRotationDragButton = DoRotationDragButton(gui, 
                    part->pos,
                &newAngle, 
                angleButtonLength, 
                5, 
                editor->editState==EDIT_CREATURE_ROTATION);
            part->angle+=newAngle-angleButtonAngle;
            part->angle=SnapAngle(editor, part->angle);
        }
        else
        {
            BodyPartDefinition *parent = GetBodyPartById(def, part->connectionId);
            Vec2 pivotPoint = part->pivotPoint;
            r32 absAngle = part->angle;
            r32 width = part->width;

            // Draw nice angle thing
            Vec2 to = v2_add(pivotPoint, v2_polar(absAngle, width));
            r32 edgeAngle = GetAbsoluteEdgeAngle(parent, part->xEdge, part->yEdge);
            Vec2 normalPoint = v2_add(pivotPoint, v2_polar(edgeAngle, 50));
            sprintf(angleInfo, "%.1f deg", RadToDeg(part->localAngle));
            angleInfoPos = v2_add(pivotPoint, vec2(10, 10));

            // Do length and rotation dragg button
            hitRotationAndLengthDragButton = DoAngleLengthButton(gui,
                    pivotPoint, 
                    &absAngle,
                    &width,
                    5,
                    editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH);
            part->width = SnapDim(editor, width);
            part->localAngle = SnapAngle(editor, GetLocalAngleFromAbsoluteAngle(def, part, absAngle));

            batch->colorState = vec4(1,1,1,1);
            PushLine2(batch, pivotPoint, to, 1, squareRegion->pos, squareRegion->size);
            PushLine2(batch, pivotPoint, normalPoint, 1, squareRegion->pos, squareRegion->size);
            batch->colorState = vec4(1,1,1,0.5);
            // Turn this into a function
            r32 angDiff = GetNormalizedAngDiff(absAngle, edgeAngle);
            PushSemiCircle2(batch, pivotPoint, 20, edgeAngle, edgeAngle+angDiff, 20, squareRegion);

            r32 minAngle = edgeAngle+part->minAngle;
            to = v2_add(pivotPoint, v2_polar(minAngle, 40));
            hitMinAngleDragButton = hitRotationDragButton = DoRotationDragButton(gui, 
                    pivotPoint,
                    &minAngle, 
                    40,
                    5, 
                    editor->editState==EDIT_CREATURE_MIN_ANGLE);
            batch->colorState = vec4(0.8, 0.3, 0.3, 1.0);
            PushLine2(batch, pivotPoint, to, 1, squareRegion->pos, squareRegion->size);
            part->minAngle = SnapAngle(editor,
                    ClampF(-M_PI+0.1, part->maxAngle-0.1, GetNormalizedAngDiff(minAngle, edgeAngle)));

            r32 maxAngle = edgeAngle+part->maxAngle;
            to = v2_add(pivotPoint, v2_polar(maxAngle, 40));
            hitMaxAngleDragButton = DoRotationDragButton(gui, 
                    pivotPoint,
                    &maxAngle, 
                    40,
                    5, 
                    editor->editState==EDIT_CREATURE_MAX_ANGLE);
            batch->colorState = vec4(0.3, 0.8, 0.3, 1.0);
            PushLine2(batch, pivotPoint, to, 1, squareRegion->pos, squareRegion->size);
            part->maxAngle = SnapAngle(editor, 
                    ClampF(part->minAngle+0.1, M_PI-0.1, GetNormalizedAngDiff(maxAngle, edgeAngle)));
            if(hitMinAngleDragButton || hitMaxAngleDragButton)
            {
                sprintf(info, "%.1f deg", 
                        hitMinAngleDragButton ? RadToDeg(part->minAngle) : RadToDeg(part->maxAngle));
            }

            batch->colorState = vec4(0.3, 0.3, 0.8, 0.5);
            PushSemiCircle2(batch, pivotPoint, 20, edgeAngle+part->minAngle, 
                    edgeAngle+part->maxAngle, 20, squareRegion);

        }

        if(editor->editState==EDIT_CREATURE_HEIGHT &&
            !hitHeightDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_WIDTH 
                && !hitWidthDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_ROTATION
                && !hitRotationDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH
                && !hitRotationAndLengthDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_MIN_ANGLE
                && !hitMinAngleDragButton) editor->editState = EDIT_CREATURE_NONE;
        if(editor->editState==EDIT_CREATURE_MAX_ANGLE
                && !hitMaxAngleDragButton) editor->editState = EDIT_CREATURE_NONE;

        if(!CreatureEditorIsEditing(editor))
        {
            if(hitWidthDragButton) editor->editState = EDIT_CREATURE_WIDTH; 
            if(hitHeightDragButton) editor->editState = EDIT_CREATURE_HEIGHT;
            if(hitRotationDragButton) editor->editState = EDIT_CREATURE_ROTATION;
            if(hitRotationAndLengthDragButton) editor->editState = EDIT_CREATURE_ROTATION_AND_LENGTH;
            if(hitMinAngleDragButton) editor->editState = EDIT_CREATURE_MIN_ANGLE;
            if(hitMaxAngleDragButton) editor->editState = EDIT_CREATURE_MAX_ANGLE;
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
    if(nk_begin(ctx, "Editor", nk_rect(5, 5, 300, 600), 
                NK_WINDOW_TITLE | 
                NK_WINDOW_BORDER | 
                NK_WINDOW_MOVABLE | 
                NK_WINDOW_SCALABLE ))
    {
        nk_layout_row_dynamic(ctx, 30, 2);
        b32 canAddBodyPart = EditorCanAddBodyPart(editor);
        struct nk_color bodyPartTextColor = 
            canAddBodyPart ? nk_rgb(255, 255, 255) : nk_rgb(255, 0, 0);
        nk_labelf_colored(ctx, NK_TEXT_LEFT, bodyPartTextColor, "%u / %u Bodyparts", def->nBodyParts, MAX_BODYPARTS);
        nk_spacing(ctx, 1);
        nk_checkbox_label(ctx, "Snap Dims", &editor->isDimSnapEnabled);
        if(editor->isDimSnapEnabled)
        {
            NKEditFloatProperty(ctx, "Dim Res", 1, &editor->dimSnapResolution, 100, 1.0, 1.0);
        }
        else
        {
            nk_spacing(ctx, 1);
        }

        nk_checkbox_label(ctx, "Snap Angles", &editor->isAngleSnapEnabled);
        if(editor->isAngleSnapEnabled)
        {
            r32 res = RadToDeg(editor->angleSnapResolution);
            NKEditFloatProperty(ctx, "Angle Res", 1, &res, 100, 1.0, 1.0);
            editor->angleSnapResolution = DegToRad(res);
        }
        else
        {
            nk_spacing(ctx, 1);
        }

        nk_checkbox_label(ctx, "Snap Edges", &editor->isEdgeSnapEnabled);
        if(editor->isEdgeSnapEnabled)
        {
            editor->edgeSnapDivisions = nk_propertyi(ctx, "Divs", 2, editor->edgeSnapDivisions, 16, 1, 1);
        }

        //nk_layout_row_static(ctx, 30, 250, 1);
        nk_layout_row_dynamic(ctx, 30, 1);
        if(nk_button_label(ctx, "Add Bodypart"))
        {
            editor->editState = EDIT_ADD_BODYPART_FIND_EDGE;
        }
        if(nk_button_label(ctx, "Cancel"))
        {
            editor->editState = EDIT_CREATURE_NONE;
        }
        if(nk_button_label(ctx, "Delete"))
        {
            if(editor->selectedId > 1)
            {
                EditorRemoveBodyPart(editor, editor->selectedId);
            }
        }
        if(editor->selectedId)
        {
            BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
            b32 isEdited = 0;
            b32 isBrainEdited = 0;

            nk_labelf(ctx, NK_TEXT_LEFT, "BodyPart %d", part->id);

            if(NKEditRadInDegProperty(ctx, "Angle", -M_PI, 
                        part->connectionId ? &part->localAngle : &part->angle, 
                        M_PI, 1.0, 1.0)) 
                isEdited = 1;
            if(NKEditFloatProperty(ctx, "Length", 1, &part->width, 10000, 1.0, 1.0)) 
                isEdited = 1;
            if(NKEditFloatProperty(ctx, "Width", 1, &part->height, 10000, 1.0, 1.0)) 
                isEdited = 1;
            if(NKEditRadInDegProperty(ctx, "Muscle min angle", -M_PI, &part->minAngle, M_PI, 1.0, 1.0)) 
                isEdited = 1;
            if(NKEditRadInDegProperty(ctx, "Muscle max angle", -M_PI, &part->maxAngle, M_PI, 1.0, 1.0)) 
                isEdited = 1;
            nk_labelf(ctx, NK_TEXT_LEFT, "Center: (%.2f %.2f)", part->pos.x, part->pos.y);

            if(NKEditCreatureIO(ctx, "Abs X pos", 
                        &part->hasAbsoluteXPositionInput, 
                        part->absoluteXPositionInputIdx)) isBrainEdited = 1;
            if(NKEditCreatureIO(ctx, "Rotary Muscle", 
                        &part->hasDragOutput, 
                        part->dragOutputIdx)) isBrainEdited = 1;

            // Torso cannot have a rotary muscle.
            if(part->connectionId)
            {
                if(NKEditCreatureIO(ctx, "Rotary Muscle", 
                            &part->hasRotaryMuscleOutput, 
                            part->rotaryMuscleOutputIdx)) isBrainEdited = 1;
            }

            nk_layout_row_dynamic(ctx, 30, 1);

            if(isEdited)
            {
                RecalculateSubNodeBodyParts(def, def->bodyParts);
            }
            if(isBrainEdited)
            {
                AssignBrainIO(def);
            }
        }
        if(nk_tree_push(ctx, NK_TREE_TAB, "Brain Properties", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Inputs : %u", def->nInputs);
            nk_labelf(ctx, NK_TEXT_LEFT, "Outputs : %u", def->nOutputs);
            nk_labelf(ctx, NK_TEXT_LEFT, "Hidden : %u", def->nHidden);
            nk_labelf(ctx, NK_TEXT_LEFT, "Gene Size : %u", def->geneSize);
            nk_tree_pop(ctx);
        }
        if(nk_tree_push(ctx, NK_TREE_TAB, "Simulation Settings", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            editor->nGenes = nk_propertyi(ctx, "Population Size", 2, editor->nGenes, 50, 1, 1);
            //TODO: Make logarithmic ? 
            NKEditFloatProperty(ctx, "LearningRate", 0.0001, &editor->learningRate, 1, 0.001, 0.001);
            NKEditFloatProperty(ctx, "Deviation", 0.001, &editor->deviation, 10, 0.01, 0.01);
            if(nk_button_label(ctx, "Start Simulation"))
            {
                StartFakeWorld(appState, def, editor->nGenes, editor->deviation, editor->learningRate);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);

    if(!CreatureEditorIsEditing(editor) 
            && EditorIsMouseJustReleased(appState, editor))
    {
        editor->selectedId = intersectId;
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

    editor->dimSnapResolution = 10;
    editor->isDimSnapEnabled = 1;

    editor->angleSnapResolution = DegToRad(5);
    editor->isAngleSnapEnabled = 1;

    editor->edgeSnapDivisions = 8;
    editor->isEdgeSnapEnabled = 1;

    editor->nGenes = 12;
    editor->learningRate = 0.03;
    editor->deviation = 0.003;

    editor->idCounter = 1;
    BodyPartDefinition *torso = editor->creatureDefinition->bodyParts+editor->creatureDefinition->nBodyParts++;
    torso->id = editor->idCounter++;
    torso->pos = vec2(0,0);
    torso->width = 50;
    torso->height = 50;
    torso->hasDragOutput = 1;
    torso->hasRotaryMuscleOutput = 0;
    AssignBrainIO(editor->creatureDefinition);
}

