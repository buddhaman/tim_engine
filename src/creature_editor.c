
internal inline b32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

internal inline b32
CreatureEditorWasEditing(CreatureEditorScreen *editor)
{
    return editor->prevEditState!=EDIT_CREATURE_NONE;
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
    CreatureDefinition *def = editor->creatureDefinition;
    BodyPartDefinition *parent = GetBodyPartById(def, id);

    editor->isTextureSquareOccupied[parent->texGridX+parent->texGridY*editor->creatureTextureGridDivs] = 0;
    editor->selectedId = 0;

    ui32 parts[def->nBodyParts];
    ui32 nParts = GetSubNodeBodyPartsById(def, parent, parts);
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < nParts;
            bodyPartIdx++)
    {
        EditorRemoveBodyPart(editor, parts[bodyPartIdx]);
    }
    ArrayRemoveElement(def->bodyParts, sizeof(BodyPartDefinition), def->nBodyParts--, parent);

    RecalculateSubNodeBodyParts(def, def->bodyParts);
    RecalculateBodyPartDrawOrder(def);
    AssignBrainIO(def);
}

internal inline Vec3
NuklearColorFToVec3(struct nk_colorf color)
{
    return vec3(color.r, color.g, color.b);
}

internal inline Vec4
NuklearColorFToVec4(struct nk_colorf color)
{
    return vec4(color.r, color.g, color.b, color.a);
}

internal inline Vec4
GetBrushColor(CreatureEditorScreen *editor)
{
    if(editor->isErasing)
    {
        return vec4(0,0,0,0);
    }
    else
    {
        return NuklearColorFToVec4(editor->brushColor);
    }
}

void
DoDrawAtPoint(CreatureEditorScreen *editor, TextureAtlas *creatureAtlas, Vec2 point)
{
    CreatureDefinition *def = editor->creatureDefinition;
    r32 overhang = def->textureOverhang;
    for(ui32 bodyPartIdx = 0;
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
        Vec2 texCoord = GetCoordinateInBox(partDef->pos, 
                vec2(partDef->width+overhang*2, partDef->height+overhang*2), partDef->angle, point);
        r32 radius = partDef->texScale*editor->brushSize;
        ui32 color = Vec4ToRGBA(GetBrushColor(editor));
        Vec2 creatureTexCoord = v2_add(partDef->uvPos, 
                vec2(partDef->uvDims.x*texCoord.x, partDef->uvDims.y*texCoord.y));
        DrawCircleOnTexture(creatureAtlas, 
                partDef->uvPos, partDef->uvDims,
                creatureTexCoord, radius, color);
    }
}

b32 
NKEditCreatureIO(struct nk_context *ctx,
        char *label,
        char *tooltip,
        b32 *value,
        ui32 brainIdx)
{
    b32 edited = 0;
    nk_layout_row_begin(ctx, NK_DYNAMIC, 30, 2);
    nk_layout_row_push(ctx, 0.5);
    if(nk_widget_is_hovered(ctx))
    {
        nk_tooltip(ctx, tooltip);
    }
    if(nk_checkbox_label(ctx, label, value))
        edited = 1;
    if(*value)
    {
        nk_labelf(ctx, NK_TEXT_LEFT, "Index = %u", brainIdx);
    }
    nk_layout_row_end(ctx);
    return edited;
}

typedef struct
{
    int x; 
    int y;
} TextureGridLocation;

internal inline TextureGridLocation
UseEmptyTextureGridLocation(CreatureEditorScreen *editor)
{
    for(int y = 0; y < editor->creatureTextureGridDivs; y++)
    for(int x = 0; x < editor->creatureTextureGridDivs; x++)
    {
        int idx = x+y*editor->creatureTextureGridDivs;
        if(!editor->isTextureSquareOccupied[idx])
        {
            editor->isTextureSquareOccupied[idx] = 1;
            return (TextureGridLocation){x, y};
        }
    }
    Assert(0);
    return (TextureGridLocation){-1, -1};
}

internal inline void
AssignTextureToBodyPartDefinition(CreatureEditorScreen *editor, BodyPartDefinition *part)
{
    int divs = editor->creatureTextureGridDivs;
    TextureGridLocation loc = UseEmptyTextureGridLocation(editor);
    part->uvPos = vec2(((r32)loc.x)/divs, ((r32)loc.y)/divs);
    part->uvDims = vec2(1.0/divs, 1.0/divs);
    part->texGridX = loc.x;
    part->texGridY = loc.y;
}

internal inline void
RecalculateTextureDims(CreatureEditorScreen *editor, r32 totalTextureSize)
{
    r32 pix = 1.0/totalTextureSize;
    r32 divs = editor->creatureTextureGridDivs;
    r32 overhang = editor->creatureDefinition->textureOverhang;
    for(ui32 partIdx = 0;
            partIdx < editor->creatureDefinition->nBodyParts;
            partIdx++)
    {
        BodyPartDefinition *part = editor->creatureDefinition->bodyParts+partIdx;
        r32 w = part->width+overhang*2;
        r32 h = part->height+overhang*2;
        r32 maxDim = Max(w, h);    
        part->texScale = 1.0/(maxDim*divs);
        part->uvDims = vec2(part->texScale*w-pix*4, part->texScale*h-pix*4);    //-4 pixels to prevent bleeding.
        r32 cx = (part->texGridX+0.5)/divs;
        r32 cy = (part->texGridY+0.5)/divs;
        part->uvPos = vec2(cx-part->uvDims.x/2.0, cy-part->uvDims.y/2.0);
    }
}

internal inline EditCreatureTool
GetCreatureTool(CreatureEditorScreen *editor)
{
    switch(editor->editState)
    {

    case EDIT_CREATURE_DRAW:
    {
        return CREATURE_TOOL_BRUSH;
    } break;

    default:
    {
        return CREATURE_TOOL_SELECT;
    } break;

    }
}

internal inline char *
GetToolName(CreatureEditorScreen *editor)
{
    EditCreatureTool tool = GetCreatureTool(editor);
    switch(tool)
    {
    case CREATURE_TOOL_BRUSH:
    {
        return "Brush";
    } break;

    case CREATURE_TOOL_SELECT:
    {
        return "Select";
    } break;

    }
    return CREATURE_TOOL_SELECT;
}

internal inline b32
DoColorPickerButton(struct nk_context *ctx, struct nk_colorf *color, b32 *isColorPickerVisible)
{
    b32 isJustPressed = 0;
    b32 result = 0;
    if(nk_button_color(ctx, (struct nk_color){
            (ui8)(color->r*255),
            (ui8)(color->g*255),
            (ui8)(color->b*255),
            (ui8)(color->a*255)}))
    {
        *isColorPickerVisible = 1;
        isJustPressed = 1;
    }
    if(*isColorPickerVisible)
    {
        result = 1;
        struct nk_rect colorPickerBounds = nk_rect(0, 0, 300, 300);
        nk_popup_begin(ctx, NK_POPUP_STATIC, "Pick Creature Color", NK_WINDOW_NO_SCROLLBAR, colorPickerBounds);
        nk_layout_row_dynamic(ctx, 300, 1);

        struct nk_rect bounds = nk_widget_bounds(ctx);
        bounds.x-=20;bounds.y-=20;bounds.w+=40;bounds.h+=40;
        nk_color_pick(ctx, color, NK_RGB);
        b32 isInsideColorPicker = nk_input_is_mouse_hovering_rect(&ctx->input, bounds);
        if(!isInsideColorPicker && !isJustPressed)
        {
            nk_popup_close(ctx);
            *isColorPickerVisible = 0;
        }

        nk_popup_end(ctx);
    }
    return result;
}

void
DoToolWindow(AppState *appState, CreatureEditorScreen *editor, struct nk_context *ctx)
{
    CreatureDefinition *def = editor->creatureDefinition;
    if(nk_begin(ctx, "Tool", nk_rect(5, 610, 300, 300), 
                NK_WINDOW_TITLE | 
                NK_WINDOW_BORDER | 
                NK_WINDOW_MOVABLE | 
                NK_WINDOW_SCALABLE ))
    {
        nk_layout_row_dynamic(ctx, 30, 1);
        nk_label(ctx, GetToolName(editor), NK_TEXT_LEFT);
        EditCreatureTool tool = GetCreatureTool(editor);

        switch(tool)
        {
        case CREATURE_TOOL_SELECT:
        {
            nk_layout_row_dynamic(ctx, 30, 2);
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
            else
            {
                nk_spacing(ctx, 1);
            }
            
            if(editor->selectedId > 1)
            {
                if(nk_button_label(ctx, "Delete bodypart"))
                {
                    nk_layout_row_dynamic(ctx, 30, 1);
                    EditorRemoveBodyPart(editor, editor->selectedId);
                }
            }
        } break;

        case CREATURE_TOOL_BRUSH:
        {
            nk_layout_row_dynamic(ctx, 30, 2);

            nk_label(ctx, "Brush Color", NK_TEXT_LEFT);
            DoColorPickerButton(ctx, &editor->brushColor, &editor->isBrushColorPickerVisible);

            nk_layout_row_dynamic(ctx, 30, 1);
            if(nk_widget_has_mouse_click_down(ctx, NK_BUTTON_LEFT, 1))
            {
                editor->drawBrushInScreenCenter = 1;
            }
            NKEditFloatProperty(ctx, "Brush Size", 1.0, &editor->brushSize, 20.0, 0.5, 0.5);
            nk_layout_row_dynamic(ctx, 30, 2);
            nk_checkbox_label(ctx, "Background", &def->drawSolidColor);
            if(def->drawSolidColor)
            {
                if(DoColorPickerButton(ctx, &editor->creatureSolidColor, 
                            &editor->isCreatureColorPickerVisible))
                {
                    def->solidColor = NuklearColorFToVec3(editor->creatureSolidColor);
                }
            }
        } break;

        default:
        {
        }break;
        }


    }
    nk_end(ctx);
}

void
UpdateCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        RenderContext *renderer, 
        struct nk_context *ctx) 
{
    Camera2D *camera = editor->camera;
    Camera2D *screenCamera = appState->screenCamera;    // Mildly confusing
    SpriteBatch *batch = renderer->batch;
    FontRenderer *fontRenderer = renderer->fontRenderer;
    TextureAtlas *defaultAtlas = renderer->defaultAtlas;
    TextureAtlas *creatureAtlas = renderer->creatureTextureAtlas;
    Shader *spriteShader = renderer->spriteShader;
    CreatureDefinition *def = editor->creatureDefinition;
    Gui *gui = editor->gui;

    AtlasRegion *circleRegion = defaultAtlas->regions+0;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;

    // TODO: DELETE! dont recalculate every frame. Assumes width=height.
    RecalculateTextureDims(editor, creatureAtlas->width);
    b32 hasLastBrushStroke = 0;
    
    // Key and mouse input
    editor->isInputCaptured = nk_window_is_any_hovered(ctx);
    editor->canMoveCameraWithMouse = editor->editState==EDIT_CREATURE_NONE;
    if(IsKeyActionJustDown(appState, ACTION_ESCAPE))
    {
        editor->editState = EDIT_CREATURE_NONE;
    }
    if(IsKeyActionJustDown(appState, ACTION_DELETE))
    {
        if(editor->selectedId > 1)
        {
            EditorRemoveBodyPart(editor, editor->selectedId);
        }
    }

    UpdateCameraInput(appState, camera);
    UpdateCamera2D(camera, appState);
    Vec2 mousePos = camera->mousePos;

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glUseProgram(spriteShader->program);

    int matLocation = glGetUniformLocation(spriteShader->program, "transform");
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);

    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    batch->colorState = vec4(1,1,1,0.1);
    PushRect2(batch, 
            vec2(-10, -10),
            vec2(20, 20),
            circleRegion->pos,
            circleRegion->size);
    DrawGrid(batch, camera, 50, 2.0, squareRegion);

    if(def->drawSolidColor)
    {
        batch->colorState = vec4(def->solidColor.x, def->solidColor.y, def->solidColor.z, 1.0);
        for(ui32 bodyPartIdx = 0; 
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            PushOrientedRectangle2(batch,
                    part->pos,
                    part->width,
                    part->height,
                    part->angle,
                    squareRegion);
        }
    }

    EndSpritebatch(batch);

    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, renderer->creatureTextureAtlas->textureHandle);

    batch->colorState = vec4(1,1,1,1);
    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->drawOrder[bodyPartIdx];
        DrawBodyPartWithTexture(batch, part, part->pos, part->angle, def->textureOverhang);
    }
    EndSpritebatch(batch);

    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);

    // Check intersection with bodyparts.
    ui32 intersectId = 0;
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

    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        if(intersectId==part->id) batch->colorState = vec4(1, 0, 1, 1);
        else if(editor->selectedId==part->id) batch->colorState = vec4(1, 0, 0, 1);
        else batch->colorState = vec4(0, 0, 0, 1);
        PushOrientedLineRectangle2(batch,
                part->pos,
                part->width,
                part->height,
                part->angle,
                1.0,
                squareRegion);
    }

    // TODO: collect in editor and display at the same time. Just like spritebatch.
    char info[256];
    memset(info, 0, 256);

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
        part->height = Max(1.0, SnapDim(editor, halfHeight*2));

        if(!part->connectionId)
        {
            hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                        part->pos, 
                        v2_polar(part->angle, 1.0),
                        &halfWidth,
                        5, 
                        editor->editState==EDIT_CREATURE_WIDTH);
            part->width = Max(1, SnapDim(editor, halfWidth*2));

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

            // Do length and rotation dragg button
            hitRotationAndLengthDragButton = DoAngleLengthButton(gui,
                    pivotPoint, 
                    &absAngle,
                    &width,
                    5,
                    editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH);
            part->width = SnapDim(editor, width);
            part->width = Max(5, part->width);
            part->localAngle = SnapAngle(editor, GetLocalAngleFromAbsoluteAngle(def, part, absAngle));
            if(hitRotationAndLengthDragButton)
            {
                sprintf(info, "%.1f deg", RadToDeg(part->localAngle));
            }

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

    if(editor->editState==EDIT_CREATURE_NONE)
    {
        if(EditorCanAddBodyPart(editor)) 
        {
            r32 minPlaceDistance = 20.0;
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
                r32 dist = GetNearestBoxEdgeLocation(part->pos, 
                        vec2(part->width, part->height), part->angle, mousePos, &location);
                if(dist > 0 && dist < minDist)
                {
                    minDist = dist;
                    minLocation = location;
                    hasLocation = 1;
                    attachTo = part;
                }
            }
            if(hasLocation
                    && minDist < minPlaceDistance 
                    && !intersectId)
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
                sprintf(info, "place on edge %.2f.", minLocation.offset);
                if(IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT))
                {
                    if(editor->isInputCaptured)
                    {
                        editor->editState = EDIT_CREATURE_NONE;
                    }
                    else
                    {
                        editor->bodyPartLocation = minLocation;
                        editor->attachTo = attachTo;
                        editor->editState=EDIT_ADD_BODYPART_PLACE;
                    }
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
        dist = fmaxf(dist, 5.0);
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

                AssignTextureToBodyPartDefinition(editor, newPart);

                RecalculateSubNodeBodyParts(def, def->bodyParts);
                RecalculateBodyPartDrawOrder(def);
                AssignBrainIO(def);
                
                editor->selectedId = newPart->id;
                editor->editState=EDIT_CREATURE_NONE;
            }
        }
    } 
    else if(editor->editState==EDIT_CREATURE_DRAW)
    {
        if(EditorIsMousePressed(appState, editor))
        {
            hasLastBrushStroke = 1;
            if(editor->hasLastBrushStroke)
            {
                ui32 maxPoints = 8;
                // Draw line. TODO: Make actual line rendering algorithm. This is expensive.
                for(ui32 pointIdx = 0;
                        pointIdx < maxPoints; 
                        pointIdx++)
                {
                    r32 factor = ((r32)pointIdx)/(maxPoints-1);
                    Vec2 point = v2_lerp(editor->lastBrushStroke, mousePos, factor);
                    // Sets active texture. Dont forget to switch back.
                    DoDrawAtPoint(editor, creatureAtlas, point);
                }
            }
            else
            {
                DoDrawAtPoint(editor, creatureAtlas, mousePos);
            }
            editor->lastBrushStroke = mousePos;
            glBindTexture(GL_TEXTURE_2D, defaultAtlas->textureHandle);
        }
    }
    editor->hasLastBrushStroke = hasLastBrushStroke;

    if(editor->drawBrushInScreenCenter)
    {
        batch->colorState = GetBrushColor(editor);
        PushCircle2(batch, camera->pos, editor->brushSize*camera->scale, circleRegion);
        editor->drawBrushInScreenCenter = 0;
    }

    EndSpritebatch(batch);

    // Do own interface here.
    BeginSpritebatch(batch);
    glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&screenCamera->transform);
    if(editor->editState==EDIT_CREATURE_DRAW 
            && !editor->isInputCaptured)
    {
        SDL_ShowCursor(SDL_DISABLE);
        r32 brushSize = editor->brushSize/camera->scale;
        if(editor->isErasing)
        {
            batch->colorState = vec4(1.0, 1.0, 1.0, 1.0);
            PushLineCircle2(batch, screenCamera->mousePos, brushSize, 2.0, 32, squareRegion);
        }
        else
        {
            batch->colorState = GetBrushColor(editor);
            PushCircle2(batch, screenCamera->mousePos, brushSize, circleRegion);
        }
    }
    else 
    {
        SDL_ShowCursor(SDL_ENABLE);
    }
    EndSpritebatch(batch);

    // Only text from here
    BeginSpritebatch(batch);
    glBindTexture(GL_TEXTURE_2D, fontRenderer->font12Texture);

    batch->colorState=vec4(1,1,1,1);
    if(info[0])
    {
        DrawString2D(batch, fontRenderer, screenCamera->mousePos, info);
    }

    EndSpritebatch(batch);

    // Do camera handling
    if(editor->isDraggingCamera)
    {
        if(!IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT)
                || !editor->canMoveCameraWithMouse)
        {
            editor->isDraggingCamera = 0;
        }
        else
        {
            camera->pos.x-=appState->dx*camera->scale;
            camera->pos.y+=appState->dy*camera->scale;
        }
    }
    else if(editor->canMoveCameraWithMouse 
            && EditorIsMouseJustPressed(appState, editor))
    {
        editor->isDraggingCamera = 1;
    }

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
        nk_labelf_colored(ctx, NK_TEXT_LEFT, bodyPartTextColor, 
                "%u / %u Bodyparts", def->nBodyParts, MAX_BODYPARTS);

        EditCreatureTool tool = GetCreatureTool(editor);
        nk_layout_row_static(ctx, 50, 50, 3);
        if(nk_widget_is_hovered(ctx))
        {
            nk_tooltip(ctx, "Select and edit creature.");
        }
        if(nk_button_image(ctx, tool==CREATURE_TOOL_SELECT ? 
                    nuklearMedia.select_check : nuklearMedia.select))
        {
            editor->editState = EDIT_CREATURE_NONE;
        }
        if(nk_widget_is_hovered(ctx))
        {
            nk_tooltip(ctx, "Draw.");
        }
        if(nk_button_image(ctx, tool==CREATURE_TOOL_BRUSH && !editor->isErasing ? 
                    nuklearMedia.brush_check : nuklearMedia.brush))
        {
            editor->editState = EDIT_CREATURE_DRAW;
            editor->isErasing = 0;
        }
        if(nk_widget_is_hovered(ctx))
        {
            nk_tooltip(ctx, "Erase.");
        }
        if(nk_button_image(ctx, tool==CREATURE_TOOL_BRUSH && editor->isErasing ? 
                    nuklearMedia.erase_check : nuklearMedia.erase))
        {
            editor->editState = EDIT_CREATURE_DRAW;
            editor->isErasing = 1;
        }
        if(nk_tree_push(ctx, NK_TREE_TAB, "Bodypart Properties", NK_MAXIMIZED))
        {
            if(editor->selectedId)
            {
                BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
                b32 isEdited = 0;
                b32 isBrainEdited = 0;

                nk_labelf(ctx, NK_TEXT_LEFT, "BodyPart %u %u", part->id, part->degree);

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

                nk_label(ctx, "Sensors", NK_TEXT_LEFT);
                if(NKEditCreatureIO(ctx, "Abs X pos", 
                            "This bodypart can sense how much it deviates from the y-axis",
                            &part->hasAbsoluteXPositionInput, 
                            part->absoluteXPositionInputIdx)) isBrainEdited = 1;
                if(NKEditCreatureIO(ctx, "Abs Y pos", 
                            "This bodypart can sense how much it deviates from the x-axis",
                            &part->hasAbsoluteYPositionInput, 
                            part->absoluteYPositionInputIdx)) isBrainEdited = 1;

                nk_layout_row_dynamic(ctx, 30, 1);
                nk_label(ctx, "Actuators", NK_TEXT_LEFT);

                if(NKEditCreatureIO(ctx, "Drag", 
                            "This bodypart can adjust its friction",
                            &part->hasDragOutput, 
                            part->dragOutputIdx)) isBrainEdited = 1;

                // Torso cannot have a rotary muscle.
                if(part->connectionId)
                {
                    if(NKEditCreatureIO(ctx, "Rotary Muscle", 
                                "This bodypart can move",
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
            nk_tree_pop(ctx);
        }
        if(nk_tree_push(ctx, NK_TREE_TAB, "Brain Properties", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            nk_labelf(ctx, NK_TEXT_LEFT, "Inputs : %u", def->nInputs);
            nk_labelf(ctx, NK_TEXT_LEFT, "Outputs : %u", def->nOutputs);
            int nHidden = nk_propertyi(ctx, "Hidden Neurons", 0, def->nHidden, 16, 1, 1);
            if(nHidden!=def->nHidden)
            {
                def->nHidden = nHidden;
                AssignBrainIO(def);
            }
            nk_labelf(ctx, NK_TEXT_LEFT, "Gene Size : %u", def->geneSize);
            int nInternalClocks = nk_propertyi(ctx, "Internal Clocks", 0, def->nInternalClocks, 4, 1, 1);
            if(nInternalClocks != def->nInternalClocks)
            {
                def->nInternalClocks = nInternalClocks;
                AssignBrainIO(def);
            }
            nk_tree_pop(ctx);
        }
        if(nk_tree_push(ctx, NK_TREE_TAB, "Simulation Settings", NK_MAXIMIZED))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            editor->nGenes = nk_propertyi(ctx, "Population Size", 2, editor->nGenes, 50, 1, 1);
            //TODO: Make logarithmic ?  Or multiply by a thousand 
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

    DoToolWindow(appState, editor, ctx);

    if(!CreatureEditorIsEditing(editor) 
            && !CreatureEditorWasEditing(editor)
            && EditorIsMouseJustReleased(appState, editor))
    {
        editor->selectedId = intersectId;
    }
    editor->prevEditState = editor->editState;
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

    // Texture
    editor->creatureTextureGridDivs = 4;
    memset(editor->isTextureSquareOccupied, 0, sizeof(editor->isTextureSquareOccupied));
    editor->brushSize = 3.0;
    editor->brushColor = (struct nk_colorf){1.0, 0.0, 0.0, 1.0};
    editor->creatureSolidColor = (struct nk_colorf){0.8, 0.8, 0.8, 1.0};

    editor->creatureDefinition->nHidden = 6;
    editor->creatureDefinition->textureOverhang = 5;
    editor->creatureDefinition->nInternalClocks = 2;
    editor->creatureDefinition->drawSolidColor = 1;
    editor->creatureDefinition->solidColor = NuklearColorFToVec3(editor->creatureSolidColor);
    editor->idCounter = 1;
    BodyPartDefinition *torso = editor->creatureDefinition->bodyParts+editor->creatureDefinition->nBodyParts++;
    torso->id = editor->idCounter++;
    torso->degree = 0U;
    torso->pos = vec2(0,0);
    torso->width = 50;
    torso->height = 50;
    torso->hasDragOutput = 1;
    torso->hasRotaryMuscleOutput = 0;
    AssignTextureToBodyPartDefinition(editor, torso);
    AssignBrainIO(editor->creatureDefinition);
    RecalculateBodyPartDrawOrder(editor->creatureDefinition);
}

