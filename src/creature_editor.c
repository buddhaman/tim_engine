
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

internal inline b32 
EditorCanAddBodyPart(CreatureEditorScreen *editor)
{
    return editor->creatureDefinition->nBodyParts < MAX_BODYPARTS;
}

internal inline b32
EditorIsJustPressed(AppState *appState, CreatureEditorScreen *editor, KeyAction action)
{
    return !editor->isInputCaptured && IsKeyActionJustDown(appState, action);
}

internal inline b32
EditorIsJustReleased(AppState *appState, CreatureEditorScreen *editor, KeyAction action)
{
    return !editor->isInputCaptured && IsKeyActionJustReleased(appState, action);
}

internal inline b32
EditorIsMousePressed(AppState *appState, CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT);
}

internal inline r32
SnapTo(r32 value, r32 res)
{
    return round(value/res)*res;
}

internal inline r32
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

internal inline r32
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

internal inline r32
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

    def->isTextureSquareOccupied[parent->texGridX+parent->texGridY*def->creatureTextureGridDivs] = 0;
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

internal inline void
DoDrawAtPointForBodyPart(CreatureEditorScreen *editor, 
        TextureAtlas *creatureAtlas, 
        Vec2 point, 
        BodyPartDefinition *partDef)
{
    CreatureDefinition *def = editor->creatureDefinition;
    r32 overhang = def->textureOverhang;
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

internal b32
IsInsideBodyPartTextureArea(CreatureEditorScreen *editor, Vec2 point)
{
    CreatureDefinition *def = editor->creatureDefinition;
    if(editor->selectedId)
    {
        BodyPartDefinition *partDef = GetBodyPartById(def, editor->selectedId);
        return BodyPartTexturePoint2Intersect(partDef, editor->brushSize+def->textureOverhang, point);
    }
    else
    {
        for(ui32 bodyPartIdx = 0;
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
            if(BodyPartTexturePoint2Intersect(partDef, editor->brushSize+def->textureOverhang, point))
            {
                return 1;
            }
        }
        return 0;
    }
}

internal void
DoDrawAtPoint(CreatureEditorScreen *editor, TextureAtlas *creatureAtlas, Vec2 point)
{
    CreatureDefinition *def = editor->creatureDefinition;
    if(editor->selectedId)
    {
        BodyPartDefinition *partDef = GetBodyPartById(def, editor->selectedId);
        DoDrawAtPointForBodyPart(editor, creatureAtlas, point, partDef);
    }
    else
    {
        for(ui32 bodyPartIdx = 0;
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
            DoDrawAtPointForBodyPart(editor, creatureAtlas, point, partDef);
        }
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
UseEmptyTextureGridLocation(CreatureDefinition *def)
{
    for(int y = 0; y < def->creatureTextureGridDivs; y++)
    for(int x = 0; x < def->creatureTextureGridDivs; x++)
    {
        int idx = x+y*def->creatureTextureGridDivs;
        if(!def->isTextureSquareOccupied[idx])
        {
            def->isTextureSquareOccupied[idx] = 1;
            return (TextureGridLocation){x, y};
        }
    }
    Assert(0);
    return (TextureGridLocation){-1, -1};
}

internal inline void
AssignTextureToBodyPartDefinition(CreatureDefinition *def, BodyPartDefinition *part)
{
    int divs = def->creatureTextureGridDivs;
    TextureGridLocation loc = UseEmptyTextureGridLocation(def);
    part->uvPos = vec2(((r32)loc.x)/divs, ((r32)loc.y)/divs);
    part->uvDims = vec2(1.0/divs, 1.0/divs);
    part->texGridX = loc.x;
    part->texGridY = loc.y;
}

internal inline void
RecalculateTextureDims(CreatureDefinition *def, r32 totalTextureSize)
{
    r32 pix = 1.0/totalTextureSize;
    r32 divs = def->creatureTextureGridDivs;
    r32 overhang = def->textureOverhang;
    for(ui32 partIdx = 0;
            partIdx < def->nBodyParts;
            partIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+partIdx;
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

internal inline void
SetDrawState(CreatureEditorScreen *editor, b32 isErasing, b32 keepSelection)
{
    // If already drawing, keep selection, otherwise clear.
    if(!keepSelection)
    {
        editor->selectedId = 0;
    }
    editor->editState = EDIT_CREATURE_DRAW;
    editor->isErasing = isErasing;
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
LoadCreatureDefinition(CreatureEditorScreen *editor,
        Assets *assets,
        CreatureDefinitionFile *file)
{
    TextureAtlas *creatureAtlas = assets->creatureTextureAtlas;
    CreatureDefinition *def = editor->creatureDefinition;
    *def = (CreatureDefinition){};
    Serializer serializer;
    BeginSerializing(&serializer, file->dataPath, 0);
    SerializeCreatureDefinition(&serializer, def);
    EndSerializing(&serializer);
    RecalculateSubNodeBodyParts(def, def->bodyParts);
    RecalculateBodyPartDrawOrder(def);
    int x, y, n;
    unsigned char *data = stbi_load(file->texturePath, &x, &y, &n, 4);
    DebugOut("loaded creature texture %dx%d with %d components per pixel", 
            x, y, n);
    if(x==CREATURE_TEX_SIZE && n==4)
    {
        SetTextureAtlasImageData(creatureAtlas, data);
    }
    else
    {
        DebugOut("Incorrect texture format, should be 2048x2048 png with 4 components per pixel (rgba).");
    }
    stbi_image_free(data);

    editor->idCounter = 1;
    for(ui32 partDefIdx = 0;
            partDefIdx < def->nBodyParts;
            partDefIdx++)
    {
        BodyPartDefinition *partDef = def->bodyParts+partDefIdx;
        editor->idCounter = Max(editor->idCounter, partDef->id+1);
    }
    editor->selectedId = 0;
    editor->rightSelectedId = 0;
    DebugOut("done loading");
}

void 
EditBodypartDefinition(CreatureEditorScreen *editor, 
        Assets *assets, 
        BodyPartDefinition *part, 
        char *info)
{
    Gui *gui = editor->gui;
    CreatureDefinition *def = editor->creatureDefinition;
    RenderGroup *renderGroup = editor->worldRenderGroup;
    //AtlasRegion *circleRegion = renderContext->defaultAtlas->regions+0;
    AtlasRegion *squareRegion = assets->defaultAtlas->regions+1;
    Camera2D *camera = editor->camera;
    r32 lineWidth = 2.0*camera->scale;
    r32 buttonRadius = 10*camera->scale;

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
                buttonRadius,
                editor->editState==EDIT_CREATURE_HEIGHT);
    part->height = Max(1.0, SnapDim(editor, halfHeight*2));

    if(!part->connectionId)
    {
        hitWidthDragButton = DoDragButtonAlongAxis(gui, 
                    part->pos, 
                    v2_polar(part->angle, 1.0),
                    &halfWidth,
                    buttonRadius,
                    editor->editState==EDIT_CREATURE_WIDTH);
        part->width = Max(1, SnapDim(editor, halfWidth*2));

        r32 newAngle = angleButtonAngle;
        hitRotationDragButton = DoRotationDragButton(gui, 
                part->pos,
            &newAngle, 
            angleButtonLength, 
            buttonRadius,
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
        Vec4 lineColor = vec4(1,1,1,1);
        Push2DLineColored(renderGroup, pivotPoint, to, lineWidth, squareRegion, lineColor);
        Push2DLineColored(renderGroup, pivotPoint, normalPoint, lineWidth, squareRegion, lineColor);
        hitRotationAndLengthDragButton = DoAngleLengthButton(gui,
                pivotPoint, 
                &absAngle,
                &width,
                buttonRadius,
                editor->editState==EDIT_CREATURE_ROTATION_AND_LENGTH);
        part->width = SnapDim(editor, width);
        part->width = Max(5, part->width);
        part->localAngle = SnapAngle(editor, GetLocalAngleFromAbsoluteAngle(def, part, absAngle));
        if(hitRotationAndLengthDragButton)
        {
            sprintf(info, "%.1f deg", RadToDeg(part->localAngle));
        }

        // Turn this into a function
        r32 angDiff = GetNormalizedAngDiff(absAngle, edgeAngle);
        Push2DSemiCircleColored(renderGroup, 
                pivotPoint, 
                20.0, 
                edgeAngle, 
                edgeAngle+angDiff, 
                16, 
                squareRegion,
                vec4(1, 1, 1, 0.4));

        r32 minAngle = edgeAngle+part->minAngle;
        to = v2_add(pivotPoint, v2_polar(minAngle, 40));
        hitMinAngleDragButton = hitRotationDragButton = DoRotationDragButton(gui, 
                pivotPoint,
                &minAngle, 
                40,
                buttonRadius,
                editor->editState==EDIT_CREATURE_MIN_ANGLE);
        Vec4 minLineColor = vec4(0.8, 0.3, 0.3, 1.0);
        Push2DLineColored(renderGroup, pivotPoint, to, lineWidth, squareRegion, minLineColor);
        part->minAngle = SnapAngle(editor,
                ClampF(-M_PI+0.1, part->maxAngle-0.1, GetNormalizedAngDiff(minAngle, edgeAngle)));

        r32 maxAngle = edgeAngle+part->maxAngle;
        to = v2_add(pivotPoint, v2_polar(maxAngle, 40));
        hitMaxAngleDragButton = DoRotationDragButton(gui, 
                pivotPoint,
                &maxAngle, 
                40,
                buttonRadius,
                editor->editState==EDIT_CREATURE_MAX_ANGLE);
        Vec4 maxLineColor = vec4(0.3, 0.8, 0.3, 1.0);
        Push2DLineColored(renderGroup, pivotPoint, to, lineWidth, squareRegion, maxLineColor);
        part->maxAngle = SnapAngle(editor, 
                ClampF(part->minAngle+0.1, M_PI-0.1, GetNormalizedAngDiff(maxAngle, edgeAngle)));
        if(hitMinAngleDragButton || hitMaxAngleDragButton)
        {
            sprintf(info, "%.1f deg", 
                    hitMinAngleDragButton ? RadToDeg(part->minAngle) : RadToDeg(part->maxAngle));
        }

        Vec4 angleCircleColor = vec4(0.3, 0.3, 0.8, 0.5);
        Push2DSemiCircleColored(renderGroup, pivotPoint, 20, edgeAngle+part->minAngle, 
                edgeAngle+part->maxAngle, 24, squareRegion, angleCircleColor);
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

int
BeginCenterPopup(AppState *appState, 
        struct nk_context *ctx,
        r32 width, r32 height,
        char *title)
{
    r32 x = appState->screenWidth/2-width/2;
    r32 y = appState->screenHeight/2-height/2;
    return nk_begin(ctx, title, nk_rect(x, y, width, height), 
                NK_WINDOW_TITLE | 
                NK_WINDOW_BORDER | 
                NK_WINDOW_MOVABLE | 
                NK_WINDOW_SCALABLE );
}

void
EndCenterPopup(struct nk_context *ctx)
{
    nk_end(ctx);
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

internal inline void 
DrawBodyPartWithOverhang(SpriteBatch *batch, 
        BodyPartDefinition *def, 
        r32 overhang,
        AtlasRegion *tex)
{
    PushOrientedRectangle2(batch, 
            def->pos,
            def->width+overhang*2,
            def->height+overhang*2,
            def->angle,
            tex);
}

void
UpdateCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        Assets *assets, 
        struct nk_context *ctx) 
{
    Camera2D *camera = editor->camera;
    Camera2D *screenCamera = appState->screenCamera;    // Mildly confusing
    FontRenderer *fontRenderer = assets->fontRenderer;
    TextureAtlas *defaultAtlas = assets->defaultAtlas;
    TextureAtlas *creatureAtlas = assets->creatureTextureAtlas;
    Shader *spriteShader = assets->spriteShader;
    CreatureDefinition *def = editor->creatureDefinition;

    RenderGroup *worldRenderGroup = editor->worldRenderGroup;
    RenderGroup *screenRenderGroup = editor->screenRenderGroup;

    AtlasRegion *circleRegion = defaultAtlas->regions+0;
    AtlasRegion *squareRegion = defaultAtlas->regions+1;

    // TODO: DELETE! dont recalculate every frame. Assumes width=height.
    RecalculateTextureDims(def, creatureAtlas->width);
    b32 hasLastBrushStroke = 0;
    
    // Key and mouse input
    editor->isInputCaptured = nk_window_is_any_hovered(ctx) ||
        editor->showSaveScreen ||
        editor->showLoadScreen;
    editor->canMoveCameraWithMouse = editor->editState==EDIT_CREATURE_NONE ||
        (editor->editState==EDIT_CREATURE_DRAW && !editor->isMouseInDrawArea);
    if(IsKeyActionJustDown(appState, ACTION_ESCAPE))
    {
        editor->editState = EDIT_CREATURE_NONE;
    }
    if(EditorIsJustReleased(appState, editor, ACTION_DELETE))
    {
        if(editor->selectedId > 1)
        {
            EditorRemoveBodyPart(editor, editor->selectedId);
        }
    }

    if(!editor->isInputCaptured)
    {
    }
    UpdateCamera2D(camera, appState);
    Vec2 mousePos = camera->mousePos;

    Push2DRectColored(worldRenderGroup, 
            vec2(-10, -10),
            vec2(20, 20),
            circleRegion,
            vec4(1,1,1,0.1));

    // TODO: Fix grid. DrawGrid(batch, camera, 50, 2.0, squareRegion);

    if(def->drawSolidColor)
    {
        Vec4 solidColor = vec4(def->solidColor.x, def->solidColor.y, def->solidColor.z, 1.0);
        for(ui32 bodyPartIdx = 0; 
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            Push2DOrientedRectangleColored(worldRenderGroup,
                    part->pos,
                    vec2(part->width, part->height),
                    part->angle,
                    squareRegion,
                    solidColor);
        }
    }


    // Test rendergroup
#if 0
    RenderGroup *renderGroup = editor->worldRenderGroup;

    local_persist r32 angleCounter = 0.0;
    angleCounter+=0.01;
    Push2DRectColored(renderGroup, vec2(20, 20), vec2(30, 30), vec4(1,1,0,1), squareRegion);
    Push2DTextColored(renderGroup, fontRenderer, vec2(20, 20), vec4(1,1,1,1), "hello");
    Push2DRectColored(renderGroup, vec2(20, 20), vec2(30, 32), vec4(1,0,1,1), circleRegion);
    Push2DLineColored(renderGroup, vec2(-10, -100), vec2(200, -12), 1.0, squareRegion, vec4(0.5,1,0.87,1));
    Push2DOrientedRectangleColored(renderGroup, vec2(-10, -100), vec2(40, 80), angleCounter, circleRegion, vec4(1,0,1,1));

    ExecuteRenderGroup(renderGroup, renderContext, camera, spriteShader);
#endif

    b32 hasFocusedBodyPart = editor->editState==EDIT_CREATURE_DRAW && editor->selectedId;
    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->drawOrder[bodyPartIdx];
        r32 alpha = hasFocusedBodyPart ? (part->id==editor->selectedId ? 1.0 : 0.3) : 1.0;
        Vec4 creatureColor = vec4(1,1,1, alpha);
        DrawBodyPartWithTexture(worldRenderGroup, part, part->pos, part->angle, def->textureOverhang, creatureAtlas->textureHandle, creatureColor);
    }
    // If drawing and has selected: Draw on top.
    if(hasFocusedBodyPart)
    {
        BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
        DrawBodyPartWithTexture(worldRenderGroup, part, part->pos, part->angle, 
                def->textureOverhang, creatureAtlas->textureHandle, vec4(1,1,1,1));
    }

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

    r32 outlineSize = 2.0 * camera->scale;

    // Draw creature outline
    for(ui32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        Vec4 color;
        if(intersectId==part->id) color = vec4(1, 0, 1, 1);
        else if(editor->selectedId==part->id) color = vec4(1, 0, 0, 1);
        else color = vec4(0, 0, 0, 1);
        Push2DOrientedLineRectangleColored(worldRenderGroup,
                part->pos,
                vec2(part->width, part->height),
                part->angle,
                outlineSize,
                squareRegion,
                color);
    }

    // TODO: collect in editor and display at the same time. Just like spritebatch.
    char info[256];
    memset(info, 0, 256);

    if(editor->selectedId && editor->editState!=EDIT_CREATURE_DRAW)
    {
        BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
        EditBodypartDefinition(editor, assets, part, info);
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
                Vec4 intersectCircleColor = vec4(0, 1.0, 0.0, 1.0);
                Push2DCircleColored(worldRenderGroup, 
                        minLocation.pos, 
                        5.0*camera->scale, 
                        circleRegion,
                        intersectCircleColor);
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
        Vec4 ghostLimbColor = vec4(1.0, 1.0, 1.0, 0.4);
        Push2DOrientedRectangleColored(worldRenderGroup,
                center,
                vec2(dist, 20),
                angle,
                squareRegion,
                ghostLimbColor);
        if(EditorIsJustReleased(appState, editor, ACTION_MOUSE_BUTTON_LEFT))
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
                
                newPart->hasDragOutput = 0;
                newPart->hasRotaryMuscleOutput = 1;
                newPart->hasAbsoluteAngleInput = 1;

                AssignTextureToBodyPartDefinition(def, newPart);

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
        editor->isMouseInDrawArea = IsInsideBodyPartTextureArea(editor, mousePos);
        if(EditorIsMousePressed(appState, editor))
        {
            hasLastBrushStroke = 1;
            if(editor->hasLastBrushStroke)
            {
                ui32 maxPoints = 8;
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
        }

#if 1
        // Render drawable surface
        glEnable(GL_STENCIL_TEST);
        SpriteBatch *batch = assets->batch;
        
        SetupSpriteBatch(batch, camera, spriteShader);
        glBindTexture(GL_TEXTURE_2D, assets->defaultAtlas->textureHandle);

        BeginStencilShape();

        BeginSpritebatch(batch);
        if(editor->selectedId)
        {
            BodyPartDefinition *part = def->bodyParts+GetIndexOfBodyPart(def, editor->selectedId);
            DrawBodyPartWithOverhang(batch, part, def->textureOverhang, squareRegion);
        }
        else
        {
            for(ui32 bodyPartIdx = 0;
                    bodyPartIdx < def->nBodyParts;
                    bodyPartIdx++)
            {
                BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
                DrawBodyPartWithOverhang(batch, part, def->textureOverhang, squareRegion);
            }
        }
        EndSpritebatch(batch);
        EndStencilShape();

        glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

        DrawDirectRect(batch, 
                camera, 
                spriteShader, 
                vec2(camera->pos.x-camera->size.x/2.0, camera->pos.y-camera->size.y/2.0),
                camera->size,
                defaultAtlas->textureHandle,
                squareRegion->pos,
                squareRegion->size,
                vec4(1,1,1,0.3));

        glDisable(GL_STENCIL_TEST);
#endif

    }
    editor->hasLastBrushStroke = hasLastBrushStroke;

    Vec4 brushColor = GetBrushColor(editor);
    if(editor->drawBrushInScreenCenter)
    {
        Push2DCircleColored(worldRenderGroup, camera->pos, editor->brushSize, circleRegion, brushColor);
        editor->drawBrushInScreenCenter = 0;
    }

    if(editor->editState==EDIT_CREATURE_DRAW 
            && !editor->isInputCaptured
            && editor->isMouseInDrawArea)
    {
        r32 brushSize = editor->brushSize/camera->scale;
        SDL_ShowCursor(SDL_DISABLE);
        if(editor->isErasing)
        {
            Push2DLineCircleColored(screenRenderGroup, 
                    screenCamera->mousePos, brushSize, 24, 2.0, squareRegion, vec4(1,1,1,0.5));
        }
        else
        {
            Push2DCircleColored(screenRenderGroup, screenCamera->mousePos, brushSize+2, 
                    circleRegion, vec4(0,0,0,0.5));
            Push2DCircleColored(screenRenderGroup, screenCamera->mousePos, brushSize, 
                    circleRegion, brushColor);
        }
    }
    else 
    {
        SDL_ShowCursor(SDL_ENABLE);
    }

    b32 isRightButtonDown = IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_RIGHT);
    b32 showRadialMenu = editor->rightSelectedId && isRightButtonDown;
    int selectedFromRadialMenu = DoRadialMenu(editor->gui, screenCamera->mousePos, showRadialMenu, 
            3, "Draw On Bodypart", "Select", "Delete");
    if(selectedFromRadialMenu >= 0)
    {
        if(selectedFromRadialMenu==0)
        {
            editor->selectedId = editor->rightSelectedId;
            editor->rightSelectedId = 0;
            SetDrawState(editor, 0, 1);
        }
        else if(selectedFromRadialMenu==1)
        {
            editor->selectedId = editor->rightSelectedId;
        }
        else if(selectedFromRadialMenu==2)
        {
            if(editor->rightSelectedId > 1)
            {
                EditorRemoveBodyPart(editor, editor->rightSelectedId);
                editor->rightSelectedId = 0;
            }
        }
    }

    if(info[0])
    {
        Push2DTextColored(screenRenderGroup, fontRenderer, screenCamera->mousePos, info, vec4(1,1,1,1));
    }

    ExecuteAndFlushRenderGroup(worldRenderGroup, assets, camera, spriteShader);
    ExecuteAndFlushRenderGroup(screenRenderGroup, assets, screenCamera, spriteShader);

    GuiUpdate(editor->gui, screenCamera, camera);

    b32 stopDragging = 0;
    // Do camera handling
    if(editor->canMoveCameraWithMouse
            && !editor->isInputCaptured)
    {
        UpdateCameraInput(appState, camera);
    }
    else
    {
        stopDragging = 1;
    }
    if(stopDragging || !IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT))
    {
        CameraStopDragging(camera);
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
            SetDrawState(editor, 0, editor->editState==EDIT_CREATURE_DRAW);
        }
        if(nk_widget_is_hovered(ctx)) {
            nk_tooltip(ctx, "Erase.");
        }
        if(nk_button_image(ctx, tool==CREATURE_TOOL_BRUSH && editor->isErasing ? 
                    nuklearMedia.erase_check : nuklearMedia.erase))
        {
            SetDrawState(editor, 1, editor->editState==EDIT_CREATURE_DRAW);
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
                if(NKEditCreatureIO(ctx, "Abs Y pos", 
                            "This bodypart can sense how much it deviates from the x-axis",
                            &part->hasAbsoluteYPositionInput, 
                            part->absoluteYPositionInputIdx)) isBrainEdited = 1;
                if(NKEditCreatureIO(ctx, "Angle", 
                            "This bodypart can sense how much it points towards the target",
                            &part->hasAngleTowardsTargetInput, 
                            part->angleTowardsTargetInputIdx)) isBrainEdited = 1;
                if(NKEditCreatureIO(ctx, "Absolute Angle", 
                            "This bodypart can sense its vertical angle",
                            &part->hasAbsoluteAngleInput, 
                            part->absoluteAngleInputIdx)) isBrainEdited = 1;

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
            if(nk_button_label(ctx, "Save"))
            {
                editor->showSaveScreen = 1;
            }
            if(nk_button_label(ctx, "Load"))
            {
                // Show list of creatures.

                int maxFiles = 64;
                char *fileNameBuffer = malloc(sizeof(char)*maxFiles*128);
                char *buffer = fileNameBuffer;
                ui32 creatureCounter = 0;
                char *creatureFiles[maxFiles];
                ui32 textureCounter = 0;
                char *textureFiles[maxFiles];

                tinydir_dir dir;
                tinydir_open(&dir, CREATURE_FOLDER_NAME);
                while(dir.has_next)
                {
                    tinydir_file file;
                    tinydir_readfile(&dir, &file);
                    DebugOut("%s", file.name);
                    size_t l = strlen(file.name);
                    if(!file.is_dir && l < 64)
                    {
                        b32 isValid = 0;
                        size_t l = strlen(file.name);
                        if(StringEndsWith(file.name, ".crdf"))
                        {
                            creatureFiles[creatureCounter++] = buffer;
                            isValid = 1;
                        }
                        if(StringEndsWith(file.name, ".png"))
                        {
                            textureFiles[textureCounter++] = buffer;
                            isValid = 1;
                        }
                        if(isValid)
                        {
                            strcpy(buffer, file.name);
                            buffer+=(l+1);
                        }
                    }
                    tinydir_next(&dir);
                }

                DebugOut("creature files: %d, texture files: %d", creatureCounter, textureCounter);

                editor->nCreatureFiles = PairCreatureFiles(MAX_CREATURE_FILES, 
                                                            editor->creatureFiles,
                                                            creatureCounter, 
                                                            creatureFiles,
                                                            textureCounter,
                                                            textureFiles);
                        
                free(fileNameBuffer);

                editor->showLoadScreen = 1;

            }
            if(nk_button_label(ctx, "Start Simulation"))
            {
                StartFakeWorld(appState, def, editor->nGenes, editor->deviation, editor->learningRate);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
    
    if(editor->showLoadScreen)
    {
        r32 savedCreatureWidth = 400;
        r32 savedCreatureHeight = 400;
        if(BeginCenterPopup(appState, ctx, savedCreatureWidth, savedCreatureHeight, "Load Creature"))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            for(ui32 creatureIdx = 0;
                    creatureIdx < editor->nCreatureFiles;
                    creatureIdx++)
            {
                CreatureDefinitionFile *file = editor->creatureFiles+creatureIdx;
                if(nk_button_label(ctx, file->name))
                {
                    LoadCreatureDefinition(editor, assets, file);
                    editor->showLoadScreen = 0;
                }
            }
            if(nk_button_label(ctx, "Cancel"))
            {
                editor->showLoadScreen = 0;
            }
        }
        EndCenterPopup(ctx);
    }

    if(editor->showSaveScreen)
    {
        r32 saveScreenWidth = 400;
        r32 saveScreenHeight = 400;
        if(BeginCenterPopup(appState, ctx, saveScreenWidth, saveScreenHeight, "Save Creature"))
        {
            nk_layout_row_dynamic(ctx, 30, 1);

            nk_edit_string_zero_terminated(ctx, 
                    NK_EDIT_FIELD, 
                    def->name, 
                    MAX_CREATURE_NAME_LENGTH-10, nk_filter_default);

            nk_layout_row_dynamic(ctx, 30, 2);
            if(nk_button_label(ctx, "Okay"))
            {
                char dataPath[256];
                char texturePath[256];
                GeneratePathNames(def, dataPath, texturePath);

                DebugOut("Saving to %s and %s.", dataPath, texturePath);
                Serializer serializer;
                BeginSerializing(&serializer, dataPath, 1);
                SerializeCreatureDefinition(&serializer, def);
                EndSerializing(&serializer);

                // Save texture image as png
                stbi_write_png(texturePath, 
                        creatureAtlas->width,
                        creatureAtlas->height,
                        4,
                        creatureAtlas->image,
                        creatureAtlas->width*4);
                DebugOut("done saving");
                editor->showSaveScreen = 0;
            }
            if(nk_button_label(ctx, "Cancel"))
            {
                editor->showSaveScreen = 0;
            }
        }
        EndCenterPopup(ctx);
    }

    DoToolWindow(appState, editor, ctx);

    if(!CreatureEditorIsEditing(editor) 
            && !CreatureEditorWasEditing(editor))
    {
        if(EditorIsJustReleased(appState, editor, ACTION_MOUSE_BUTTON_LEFT)) 
        {
            editor->selectedId = intersectId;
        }
        if(EditorIsJustPressed(appState, editor, ACTION_MOUSE_BUTTON_RIGHT)) 
        {
            editor->rightSelectedId = intersectId;
        }
    }
    editor->prevEditState = editor->editState;
}

void
InitCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        Assets *assets,
        MemoryArena *arena)
{
    editor->camera = PushStruct(arena, Camera2D);
    InitCamera2D(editor->camera);
    editor->camera->isYUp = 1;

    editor->creatureDefinition = PushStruct(arena, CreatureDefinition);
    *editor->creatureDefinition = (CreatureDefinition){};
    GenerateRandomName(editor->creatureDefinition->name, MAX_CREATURE_NAME_LENGTH);

    editor->screenRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, editor->screenRenderGroup, 1024);

    editor->worldRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, editor->worldRenderGroup, 1024);
    
    editor->gui = PushStruct(arena, Gui);
    InitGui(editor->gui, arena, appState, editor->camera, assets);

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
    editor->creatureDefinition->creatureTextureGridDivs = 4;
    memset(editor->creatureDefinition->isTextureSquareOccupied,
            0, 
            sizeof(editor->creatureDefinition->isTextureSquareOccupied));
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
    torso->hasAbsoluteAngleInput = 1;
    torso->hasDragOutput = 0;
    torso->hasRotaryMuscleOutput = 0;
    AssignTextureToBodyPartDefinition(editor->creatureDefinition, torso);
    AssignBrainIO(editor->creatureDefinition);
    RecalculateBodyPartDrawOrder(editor->creatureDefinition);
}

