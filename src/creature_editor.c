
#define EditorDefaultParameters(editor)\
    BasicRenderTools *renderTools = editor->renderTools;\
    Assets *assets = renderTools->assets;\
    TextureAtlas *defaultAtlas = assets->defaultAtlas;\
    AtlasRegion *circleRegion = defaultAtlas->regions;(void)circleRegion;\
    AtlasRegion *squareRegion = defaultAtlas->regions+1;(void)squareRegion;

internal inline B32
CreatureEditorIsEditing(CreatureEditorScreen *editor)
{
    return editor->editState!=EDIT_CREATURE_NONE;
}

internal inline B32
CreatureEditorWasEditing(CreatureEditorScreen *editor)
{
    return editor->prevEditState!=EDIT_CREATURE_NONE;
}

internal inline B32 
EditorCanAddBodyPart(CreatureEditorScreen *editor)
{
    return editor->creatureDefinition->nBodyParts < MAX_BODYPARTS;
}

internal inline B32
EditorIsJustPressed(AppState *appState, CreatureEditorScreen *editor, KeyAction action)
{
    return !editor->isInputCaptured && IsKeyActionJustDown(appState, action);
}

internal inline B32
EditorIsJustReleased(AppState *appState, 
        CreatureEditorScreen *editor, 
        KeyAction action)
{
    return !editor->isInputCaptured && IsKeyActionJustReleased(appState, action);
}

internal inline B32
EditorIsMousePressed(AppState *appState, 
        CreatureEditorScreen *editor)
{
    return !editor->isInputCaptured && IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_LEFT);
}

internal inline R32
SnapTo(R32 value, R32 res)
{
    return round(value/res)*res;
}

internal inline R32
SnapDim(CreatureEditorScreen *editor, R32 dim)
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

internal inline R32
SnapAngle(CreatureEditorScreen *editor, R32 angle)
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

internal inline R32
SnapEdge(CreatureEditorScreen *editor, R32 offset)
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

// Particles in editor
Particle *
AddEditorParticle(CreatureEditorScreen *editor, Vec2 pos, Vec2 vel, R32 radius, R32 lifetime)
{
    Particle *particle = editor->particles+editor->nParticles++;
    Assert(editor->nParticles < MAX_EDITOR_PARTICLES);
    *particle = (Particle){};

    particle->pos      = pos;
    particle->vel      = vel;
    particle->r        = radius;
    particle->lifetime = lifetime;
    particle->time     = 0;

    return particle;
}

void
EditorRemoveBodyPart(CreatureEditorScreen *editor, U32 id)
{
    CreatureDefinition *def = editor->creatureDefinition;
    BodyPartDefinition *parent = GetBodyPartById(def, id);

    def->isTextureSquareOccupied[parent->texGridX+parent->texGridY*def->creatureTextureGridDivs] = 0;
    editor->selectedId = 0;

    U32 parts[def->nBodyParts];
    U32 nParts = GetSubNodeBodyPartsById(def, parent, parts);
    for(U32 bodyPartIdx = 0;
            bodyPartIdx < nParts;
            bodyPartIdx++)
    {
        EditorRemoveBodyPart(editor, parts[bodyPartIdx]);
    }
    ArrayRemoveElement(def->bodyParts, sizeof(BodyPartDefinition), def->nBodyParts--, parent);

    // TODO: Called wayyy too often now bc of recursion. Instead flag as removed and remove in one batch.
    RecalculateSubNodeBodyParts(def, def->bodyParts);
    RecalculateBodyPartDrawOrder(def);
    AssignBrainIO(def);
}

internal inline Vec3
NuklearColorFToVec3(struct nk_colorf color)
{
    return V3(color.r, color.g, color.b);
}

internal inline Vec4
NuklearColorFToVec4(struct nk_colorf color)
{
    return V4(color.r, color.g, color.b, color.a);
}

internal inline Vec4
GetBrushColor(CreatureEditorScreen *editor)
{
    if(editor->isErasing)
    {
        return V4(0,0,0,0);
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
    R32 overhang = def->textureOverhang;
    Vec2 texCoord = GetCoordinateInBox(partDef->pos, 
            V2(partDef->width+overhang*2, partDef->height+overhang*2), partDef->angle, point);
    R32 radius = partDef->texScale*editor->brushSize;
    U32 color = Vec4ToRGBA(GetBrushColor(editor));
    Vec2 creatureTexCoord = V2Add(partDef->uvPos, 
            V2(partDef->uvDims.x*texCoord.x, partDef->uvDims.y*texCoord.y));
    DrawCircleOnTexture(creatureAtlas, 
            partDef->uvPos, partDef->uvDims,
            creatureTexCoord, radius, color);
}

internal B32
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
        for(U32 bodyPartIdx = 0;
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
        for(U32 bodyPartIdx = 0;
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *partDef = def->bodyParts+bodyPartIdx;
            DoDrawAtPointForBodyPart(editor, creatureAtlas, point, partDef);
        }
    }
}

B32 
NKEditCreatureIO(struct nk_context *ctx,
        char *label,
        char *tooltip,
        B32 *value,
        U32 brainIdx)
{
    B32 edited = 0;
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
    part->uvPos = V2(((R32)loc.x)/divs, ((R32)loc.y)/divs);
    part->uvDims = V2(1.0/divs, 1.0/divs);
    part->texGridX = loc.x;
    part->texGridY = loc.y;
}

internal inline void
RecalculateTextureDims(CreatureDefinition *def, R32 totalTextureSize)
{
    R32 pix = 1.0/totalTextureSize;
    R32 divs = def->creatureTextureGridDivs;
    R32 overhang = def->textureOverhang;
    for(U32 partIdx = 0;
            partIdx < def->nBodyParts;
            partIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+partIdx;
        R32 w = part->width+overhang*2;
        R32 h = part->height+overhang*2;
        R32 maxDim = Max(w, h);    
        part->texScale = 1.0/(maxDim*divs);
        part->uvDims = V2(part->texScale*w-pix*4, part->texScale*h-pix*4);    //-4 pixels to prevent bleeding.
        R32 cx = (part->texGridX+0.5)/divs;
        R32 cy = (part->texGridY+0.5)/divs;
        part->uvPos = V2(cx-part->uvDims.x/2.0, cy-part->uvDims.y/2.0);
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
SetDrawState(CreatureEditorScreen *editor, B32 isErasing, B32 keepSelection)
{
    // If already drawing, keep selection, otherwise clear.
    if(!keepSelection)
    {
        editor->selectedId = 0;
    }
    editor->editState = EDIT_CREATURE_DRAW;
    editor->isErasing = isErasing;
}

internal inline B32
DoColorPickerButton(struct nk_context *ctx, struct nk_colorf *color, B32 *isColorPickerVisible)
{
    B32 isJustPressed = 0;
    B32 result = 0;
    if(nk_button_color(ctx, (struct nk_color){
            (U8)(color->r*255),
            (U8)(color->g*255),
            (U8)(color->b*255),
            (U8)(color->a*255)}))
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
        B32 isInsideColorPicker = nk_input_is_mouse_hovering_rect(&ctx->input, bounds);
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
    for(U32 partDefIdx = 0;
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
SetBodyPartValuesForAnimation(BodyPartDefinition *bodyPart, R32 timeFactor)
{
    R32 time = timeFactor*timeFactor;
    bodyPart->localAngle = bodyPart->localAngle+0.75f*sinf(time*20)*time;
    bodyPart->width = bodyPart->width*(1.0f-time);
    bodyPart->height = bodyPart->height*(1.0f-time);
}

void
StartAnimatingBodyPart(CreatureEditorScreen *editor, BodyPartDefinition *bodyPart)
{
    editor->animateBodyPart = bodyPart;
    editor->animationTime = 1.0f;
}

internal inline void
EditorDrawBodyPartTextures(CreatureEditorScreen *editor)
{
    EditorDefaultParameters(editor);
    TextureAtlas *creatureAtlas = assets->creatureTextureAtlas;
    CreatureDefinition *def = editor->creatureDefinition;
    // Draw bodypart textures
    B32 hasFocusedBodyPart = editor->editState==EDIT_CREATURE_DRAW && editor->selectedId;
    for(U32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->drawOrder[bodyPartIdx];
        R32 alpha = hasFocusedBodyPart ? (part->id==editor->selectedId ? 1.0 : 0.3) : 1.0;
        Vec4 creatureColor = V4(1,1,1, alpha);
        DrawBodyPartWithTexture(renderTools->worldRenderGroup, part, part->pos, part->angle, def->textureOverhang, creatureAtlas->textureHandle, creatureColor);
    }

    // If drawing and has selected: Draw on top.
    if(hasFocusedBodyPart)
    {
        BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
        DrawBodyPartWithTexture(renderTools->worldRenderGroup, part, part->pos, part->angle, 
                def->textureOverhang, creatureAtlas->textureHandle, V4(1,1,1,1));
    }
}

void 
EditBodypartDefinition(CreatureEditorScreen *editor, 
        AppState *appState,
        BodyPartDefinition *part, 
        char *info)
{
    EditorDefaultParameters(editor);
    Gui *gui = editor->gui;
    CreatureDefinition *def = editor->creatureDefinition;
    RenderGroup *renderGroup = renderTools->worldRenderGroup;
    R32 lineWidth = 1.0*renderTools->camera->scale;
    R32 buttonRadius = 10;

    R32 halfWidth = part->width/2;
    R32 halfHeight = part->height/2;
    R32 angleButtonLength = sqrtf(halfWidth*halfWidth+halfHeight*halfHeight);
    R32 angleButtonAngle = part->angle+atan2(-halfHeight, -halfWidth);
    B32 isEditing = 0;

    if(DoDragButtonAlongAxis(gui, 
            "hitHeightDragButton",
            part->pos, 
            V2Polar(part->angle+M_PI/2, 1.0),
            &halfHeight,
            buttonRadius))
    {
        isEditing = 1;
    }
    part->height = Max(1.0, SnapDim(editor, halfHeight*2));

    if(!part->connectionId)
    {
        if(DoDragButtonAlongAxis(gui, 
                "hitWidthDragButton",
                part->pos,
                V2Polar(part->angle, 1.0),
                &halfWidth,
                buttonRadius))
        {
            isEditing = 1;
        }
        part->width = Max(1, SnapDim(editor, halfWidth*2));

        R32 newAngle = angleButtonAngle;
        if(DoRotationDragButton(gui, 
                "hitRotationDragButton",
                part->pos,
                &newAngle, 
                angleButtonLength, 
                buttonRadius))
        {
            isEditing = 1;
        }
        part->angle+=newAngle-angleButtonAngle;
        part->angle=SnapAngle(editor, part->angle);
    }
    else
    {
        BodyPartDefinition *parent = GetBodyPartById(def, part->connectionId);
        Vec2 pivotPoint = part->pivotPoint;
        R32 absAngle = part->angle;
        R32 width = part->width;

        // Draw nice angle thing
        Vec2 to = V2Add(pivotPoint, V2Polar(absAngle, width));
        R32 edgeAngle = GetAbsoluteEdgeAngle(parent, part->xEdge, part->yEdge);
        Vec2 normalPoint = V2Add(pivotPoint, V2Polar(edgeAngle, 50));

        // Do length and rotation drag button
        Vec4 lineColor = V4(1,1,1,1);
        Push2DLineColored(renderGroup, pivotPoint, to, lineWidth, squareRegion, lineColor);
        Push2DLineColored(renderGroup, pivotPoint, normalPoint, lineWidth, squareRegion, lineColor);
        if(DoAngleLengthButton(gui,
                "hitRotationAndLengthDragButton",
                pivotPoint, 
                &absAngle,
                &width,
                buttonRadius))
        {
            isEditing = 1;
        }
        part->width = SnapDim(editor, width);
        part->width = Max(5, part->width);
        part->localAngle = SnapAngle(editor, GetLocalAngleFromAbsoluteAngle(def, part, absAngle));
#if 0
        if(hitRotationAndLengthDragButton)
        {
            sprintf(info, "%.1f deg", RadToDeg(part->localAngle));
        }
#endif

        // Turn this into a function
        R32 angDiff = GetNormalizedAngDiff(absAngle, edgeAngle);
        Push2DSemiCircleColored(renderGroup, 
                pivotPoint, 
                20.0, 
                edgeAngle, 
                edgeAngle+angDiff, 
                16, 
                squareRegion,
                V4(1, 1, 1, 0.4));

        R32 minAngle = edgeAngle+part->minAngle;
        Vec4 minLineColor = V4(0.8, 0.3, 0.3, 1.0);
        if(DoRotationDragButtonWithLine(gui, 
                "hitMinAngleDragButton",
                pivotPoint,
                &minAngle, 
                40,
                buttonRadius,
                minLineColor))
        {
            isEditing = 1;
            part->minAngle = SnapAngle(editor,
                    ClampF(-M_PI+0.1, part->maxAngle-0.1, GetNormalizedAngDiff(minAngle, edgeAngle)));
        }

        R32 maxAngle = edgeAngle+part->maxAngle;
        Vec4 maxLineColor = V4(0.3, 0.8, 0.3, 1.0);
        if(DoRotationDragButtonWithLine(gui, 
                "hitMaxAngleDragButton",
                pivotPoint,
                &maxAngle, 
                40,
                buttonRadius,
                maxLineColor))
        {
            isEditing = 1;
            part->maxAngle = SnapAngle(editor, 
                    ClampF(part->minAngle+0.1, M_PI-0.1, GetNormalizedAngDiff(maxAngle, edgeAngle)));
        }
#if 0
        if(hitMinAngleDragButton || hitMaxAngleDragButton)
        {
            sprintf(info, "%.1f deg", 
                    hitMinAngleDragButton ? RadToDeg(part->minAngle) : RadToDeg(part->maxAngle));
        }
#endif

        Vec4 angleCircleColor = V4(0.3, 0.3, 0.8, 0.5);
        Push2DSemiCircleColored(renderGroup, pivotPoint, 20, edgeAngle+part->minAngle, 
                edgeAngle+part->maxAngle, 24, squareRegion, angleCircleColor);
    }

    if(isEditing)
    {
        editor->editState = EDIT_CREATURE_BODYPART;
        RecalculateSubNodeBodyParts(def, def->bodyParts);
    }
    else if(editor->editState==EDIT_CREATURE_BODYPART)
    {
        editor->editState=EDIT_CREATURE_NONE;
    }
}

int
BeginCenterPopup(AppState *appState, 
        struct nk_context *ctx,
        R32 width, R32 height,
        char *title)
{
    R32 x = appState->screenWidth/2-width/2;
    R32 y = appState->screenHeight/2-height/2;
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
                R32 res = RadToDeg(editor->angleSnapResolution);
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
DrawBodyPartWithOverhang(Mesh2D *batch, 
        BodyPartDefinition *def, 
        R32 overhang,
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
EditorDoGui(AppState *appState, CreatureEditorScreen *editor)
{
    EditorDefaultParameters(editor);
    Gui *gui = editor->gui;

    R32 screenWidth = appState->screenWidth;
    Vec2 buttonDims = V2(180, 40);
    U32 nButtons = 2;

    R32 atX = screenWidth/2.0-buttonDims.x*nButtons/2.0;
    R32 atY = 0.0;

    GuiBeginContext(gui, "top bar");
    if(DoTabBarRadioButton(gui, "Edit Body", V2(atX, atY), buttonDims, 
                editor->editPhase==EDIT_PHASE_BODY))
    {
        editor->editPhase = EDIT_PHASE_BODY;
    }
    atX+=buttonDims.x;
    if(DoTabBarRadioButton(gui, "Edit Brain", V2(atX, atY), buttonDims,
                editor->editPhase==EDIT_PHASE_BRAIN))
    {
        editor->editPhase = EDIT_PHASE_BRAIN;
    }
    GuiEndContext(gui);

    GuiBeginContext(gui, "test context");
    local_persist char text[128];
    DoTextField(gui, "name", text, V2(400, 200), V2(200, 24));
    GuiEndContext(gui);
}

void
UpdateCreatureEditorScreen(AppState *appState, 
        CreatureEditorScreen *editor, 
        struct nk_context *ctx) 
{
    EditorDefaultParameters(editor);
    Camera2D *camera = renderTools->camera;
    Camera2D *screenCamera = renderTools->screenCamera;
    FontRenderer *fontRenderer = assets->fontRenderer;
    TextureAtlas *creatureAtlas = assets->creatureTextureAtlas;
    CreatureDefinition *def = editor->creatureDefinition;

    RenderGroup *worldRenderGroup = renderTools->worldRenderGroup;
    RenderGroup *screenRenderGroup = renderTools->screenRenderGroup;
    ShaderInstance *worldShader = renderTools->worldShader;
    ShaderInstance *screenShader = renderTools->screenShader;

    // TODO: DELETE! dont recalculate every frame. Only when dirty. Assumes width=height.
    RecalculateTextureDims(def, creatureAtlas->width);

    B32 hasLastBrushStroke = 0;
    
    // Key and mouse input
    editor->isInputCaptured = nk_window_is_any_hovered(ctx) || 
        GuiHasCapturedInput(editor->gui) || editor->showSaveScreen ||
        editor->showLoadScreen;
    editor->canMoveCameraWithMouse = editor->editState==EDIT_CREATURE_NONE ||
        (editor->editState==EDIT_CREATURE_DRAW && !editor->isMouseInDrawArea);
    editor->canScrollCameraWithMouse = 1;
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

    // TODO: This is my own gui, develop later.
    // EditorDoGui(appState, editor);

    UpdateCamera2D(camera, appState);
    Vec2 mousePos = camera->mousePos;

    Push2DCircleColored(worldRenderGroup, 
            V2(0,0),
            10,
            circleRegion,
            V4(1,1,1,0.1));

    BodyPartDefinition animateBodyPartBackup;
    if(editor->animateBodyPart)
    {
        animateBodyPartBackup = *editor->animateBodyPart;
        SetBodyPartValuesForAnimation(editor->animateBodyPart, editor->animationTime);
        RecalculateSubNodeBodyParts(editor->creatureDefinition, editor->creatureDefinition->bodyParts);
    }

    if(def->drawSolidColor)
    {
        Vec4 solidColor = V4(def->solidColor.x, def->solidColor.y, def->solidColor.z, 1.0);
        for(U32 bodyPartIdx = 0; 
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            Push2DOrientedRectangleColored(worldRenderGroup,
                    part->pos,
                    V2(part->width, part->height),
                    part->angle,
                    squareRegion,
                    solidColor);
        }
    }

    EditorDrawBodyPartTextures(editor);

    if(editor->animateBodyPart)
    {
        *editor->animateBodyPart = animateBodyPartBackup;
        editor->animationTime-=(1.0f/60.0f);
        if(editor->animationTime <= 0.0f)
        {
            editor->animateBodyPart = NULL;
        }
    }

    // Check intersection with bodyparts.
    U32 intersectId = 0;
    if(!CreatureEditorIsEditing(editor))
    {
        for(U32 bodyPartIdx = 0; 
                bodyPartIdx < def->nBodyParts;
                bodyPartIdx++)
        {
            BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
            if(OrientedBoxPoint2Intersect(part->pos, V2(part->width, part->height), part->angle, mousePos))
            {
                intersectId = part->id;
            }
        }
    }

    R32 outlineSize = 1.0 * camera->scale;

    // Draw creature outline
    for(U32 bodyPartIdx = 0; 
            bodyPartIdx < def->nBodyParts;
            bodyPartIdx++)
    {
        BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
        Vec4 color;
        if(intersectId==part->id) color = V4(1, 0, 1, 1);
        else if(editor->selectedId==part->id) color = V4(1, 0, 0, 1);
        else color = V4(0, 0, 0, 1);
        Push2DOrientedLineRectangleColored(worldRenderGroup,
                part->pos,
                V2(part->width, part->height),
                part->angle,
                outlineSize,
                squareRegion,
                color);
    }

    // TODO: collect in editor and display at the same time. Just like spritebatch.
    char info[256];
    memset(info, 0, 256);

    if(editor->editPhase==EDIT_PHASE_BODY)
    {
        if(editor->selectedId && editor->editState!=EDIT_CREATURE_DRAW)
        {
            BodyPartDefinition *part = GetBodyPartById(def, editor->selectedId);
            EditBodypartDefinition(editor, appState, part, info);
        }

        if(editor->editState==EDIT_CREATURE_NONE)
        {
            if(EditorCanAddBodyPart(editor)) 
            {
                R32 minPlaceDistance = 20.0;
                R32 minDist = 10000000.0;
                BoxEdgeLocation location = {};
                BoxEdgeLocation minLocation = {};
                B32 hasLocation = 0;
                BodyPartDefinition *attachTo = NULL;
                for(U32 bodyPartIdx = 0;
                        bodyPartIdx < def->nBodyParts;
                        bodyPartIdx++)
                {
                    BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
                    R32 dist = GetNearestBoxEdgeLocation(part->pos, 
                            V2(part->width, part->height), part->angle, mousePos, &location);
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
                            V2(attachTo->width, attachTo->height), 
                            attachTo->angle, 
                            minLocation.xEdge, 
                            minLocation.yEdge, 
                            minLocation.offset);
                    Vec4 intersectCircleColor = V4(0, 1.0, 0.0, 1.0);
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
            R32 angle = atan2f(to.y-from.y, to.x-from.x);
            R32 dist = SnapDim(editor, V2Dist(from, to));
            dist = fmaxf(dist, 5.0);
            R32 edgeAngle = atan2f(location->yEdge, location->xEdge);
            R32 localAngle = angle-part->angle - edgeAngle;
            localAngle = SnapAngle(editor, NormalizeAngle(localAngle));
            angle = NormalizeAngle(edgeAngle+part->angle)+localAngle;
            Vec2 center = V2Add(from, V2Polar(angle, dist/2));

            sprintf(info, "%.1f deg", RadToDeg(localAngle));
            Vec4 ghostLimbColor = V4(1.0, 1.0, 1.0, 0.4);
            Push2DOrientedRectangleColored(worldRenderGroup,
                    center,
                    V2(dist, 20),
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

                    R32 c = cosf(newPart->angle);
                    R32 s = sinf(newPart->angle);
                    for(int i = 0; i < 10; i++)
                    {
                        Vec2 pVel = V2Add(V2MulS(RandomNormalPair(), 1.5f), V2(0,RandomR32(0, 1)));
                        pVel = V2Add(pVel, V2(c*3, s*3));
                        pVel = V2MulS(pVel, 0.8f);
                        AddEditorParticle(editor, newPart->pivotPoint, pVel, 8.0f, RandomR32(0.2f, 0.3f));
                    }

                    StartAnimatingBodyPart(editor, newPart);
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
                    U32 maxPoints = 8;
                    for(U32 pointIdx = 0;
                            pointIdx < maxPoints; 
                            pointIdx++)
                    {
                        R32 factor = ((R32)pointIdx)/(maxPoints-1);
                        Vec2 point = V2Lerp(editor->lastBrushStroke, mousePos, factor);
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
            Mesh2D *batch = assets->batch;
            
            glBindTexture(GL_TEXTURE_2D, assets->defaultAtlas->textureHandle);
            BeginShaderInstance(worldShader);

            BeginStencilShape();

            BeginMesh2D(batch);
            if(editor->selectedId)
            {
                BodyPartDefinition *part = def->bodyParts+GetIndexOfBodyPart(def, editor->selectedId);
                DrawBodyPartWithOverhang(batch, part, def->textureOverhang, squareRegion);
            }
            else
            {
                for(U32 bodyPartIdx = 0;
                        bodyPartIdx < def->nBodyParts;
                        bodyPartIdx++)
                {
                    BodyPartDefinition *part = def->bodyParts+bodyPartIdx;
                    DrawBodyPartWithOverhang(batch, part, def->textureOverhang, squareRegion);
                }
            }
            EndMesh2D(batch);
            EndStencilShape();

            glStencilFunc(GL_NOTEQUAL, 1, 0xFF);

            DrawDirectRect(batch, 
                    worldShader,
                    V2(camera->pos.x-camera->size.x/2.0, camera->pos.y-camera->size.y/2.0),
                    camera->size,
                    defaultAtlas->textureHandle,
                    squareRegion->pos,
                    squareRegion->size,
                    V4(1,1,1,0.3));

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

        Vec3 solidColor = editor->creatureDefinition->solidColor;
        // Draw and update particles
        for(int particleIdx = editor->nParticles-1;
                particleIdx > 0;
                particleIdx--)
        {
            Particle *p = editor->particles+particleIdx;
            p->time+=(1.0f/60.0f);
            if(p->time >= p->lifetime)
            {
                editor->particles[particleIdx] = editor->particles[editor->nParticles-1];
                editor->nParticles--;
            }
            R32 timeFactor = 1.0f - p->time/p->lifetime;

            p->vel = V2Add(p->vel, editor->gravity);
            p->pos = V2Add(p->pos, p->vel);

            Vec2 from = V2Sub(p->pos, V2MulS(p->vel, 1.4f));
            Vec2 to   = V2Add(p->pos, V2MulS(p->vel, 1.4f));
            R32 shade = 0.7f;
            Push2DLineColored(worldRenderGroup, from, to, p->r*timeFactor, circleRegion, 
                    V4(solidColor.r*shade, solidColor.g*shade, solidColor.b*shade, 0.8f));
        }

        if(editor->editState==EDIT_CREATURE_DRAW 
                && !editor->isInputCaptured
                && editor->isMouseInDrawArea)
        {
            R32 brushSize = editor->brushSize/camera->scale;
            SDL_ShowCursor(SDL_DISABLE);
            if(editor->isErasing)
            {
                Push2DLineCircleColored(screenRenderGroup, 
                        screenCamera->mousePos, brushSize, 24, 2.0, squareRegion, V4(1,1,1,0.5));
            }
            else
            {
                Push2DCircleColored(screenRenderGroup, screenCamera->mousePos, brushSize+2, 
                        circleRegion, V4(0,0,0,0.5));
                Push2DCircleColored(screenRenderGroup, screenCamera->mousePos, brushSize, 
                        circleRegion, brushColor);
            }
        }
        else 
        {
            SDL_ShowCursor(SDL_ENABLE);
        }

        B32 isRightButtonDown = IsKeyActionDown(appState, ACTION_MOUSE_BUTTON_RIGHT);
        B32 showRadialMenu = editor->rightSelectedId && isRightButtonDown;
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
    }
    else if(editor->editPhase==EDIT_PHASE_BRAIN)
    {

    }

    if(info[0])
    {
        Push2DTextColored(screenRenderGroup, fontRenderer, screenCamera->mousePos, info, V4(1,1,1,1));
    }

    ExecuteAndFlushRenderGroup(worldRenderGroup, assets, worldShader);
    ExecuteAndFlushRenderGroup(screenRenderGroup, assets, screenShader);

    GuiUpdate(editor->gui);

    // Do camera handling
    B32 stopDragging = 0;
    if(!editor->isInputCaptured)
    {
        if(editor->canScrollCameraWithMouse)
        {
            UpdateCameraScrollInput(appState, camera);
        }
        if(editor->canMoveCameraWithMouse)
        {
            UpdateCameraDragInput(appState, camera);
            UpdateCameraKeyMovementInput(appState, camera);
        }
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
        B32 canAddBodyPart = EditorCanAddBodyPart(editor);
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
                B32 isEdited = 0;
                B32 isBrainEdited = 0;

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
                U32 creatureCounter = 0;
                char *creatureFiles[maxFiles];
                U32 textureCounter = 0;
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
                        B32 isValid = 0;
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
                StartFakeWorld(appState, def, assets, editor->nGenes, editor->deviation, editor->learningRate);
            }
            nk_tree_pop(ctx);
        }
    }
    nk_end(ctx);
    
    if(editor->showLoadScreen)
    {
        R32 savedCreatureWidth = 400;
        R32 savedCreatureHeight = 400;
        if(BeginCenterPopup(appState, ctx, savedCreatureWidth, savedCreatureHeight, "Load Creature"))
        {
            nk_layout_row_dynamic(ctx, 30, 1);
            for(U32 creatureIdx = 0;
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
        R32 saveScreenWidth = 400;
        R32 saveScreenHeight = 400;
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
    editor->renderTools = CreateBasicRenderTools(arena, appState, assets);
    editor->creatureDefinition = PushStruct(arena, CreatureDefinition);
    *editor->creatureDefinition = (CreatureDefinition){};
    GenerateRandomName(editor->creatureDefinition->name, MAX_CREATURE_NAME_LENGTH);

    BasicRenderTools *renderTools = editor->renderTools;
    editor->gui = PushStruct(arena, Gui);
    InitGui(editor->gui, arena, appState, renderTools->camera, assets);
    editor->dimSnapResolution = 10;
    editor->isDimSnapEnabled = 1;

    editor->angleSnapResolution = DegToRad(5);
    editor->isAngleSnapEnabled = 1;

    editor->edgeSnapDivisions = 8;
    editor->isEdgeSnapEnabled = 1;

    editor->nGenes = 12;
    editor->learningRate = 0.03;
    editor->deviation = 0.003;

    editor->gravity = V2(0, -0.1f);
    editor->nParticles = 0;

    // Texture
    editor->creatureDefinition->creatureTextureGridDivs = 4;
    memset(editor->creatureDefinition->isTextureSquareOccupied,
            0, 
            sizeof(editor->creatureDefinition->isTextureSquareOccupied));
    editor->brushSize = 3.0;
    editor->brushColor = (struct nk_colorf){1.0, 0.0, 0.0, 1.0};
    editor->creatureSolidColor = (struct nk_colorf){0.6, 0.6, 0.6, 1.0};

    editor->editPhase = EDIT_PHASE_BODY;

    editor->creatureDefinition->nHidden = 6;
    editor->creatureDefinition->textureOverhang = 5;
    editor->creatureDefinition->nInternalClocks = 2;
    editor->creatureDefinition->drawSolidColor = 1;
    editor->creatureDefinition->solidColor = NuklearColorFToVec3(editor->creatureSolidColor);
    editor->idCounter = 1;
    BodyPartDefinition *torso = editor->creatureDefinition->bodyParts+editor->creatureDefinition->nBodyParts++;
    torso->id = editor->idCounter++;
    torso->degree = 0U;
    torso->pos = V2(0,0);
    torso->width = 50;
    torso->height = 50;
    torso->hasAbsoluteAngleInput = 1;
    torso->hasDragOutput = 0;
    torso->hasRotaryMuscleOutput = 0;
    AssignTextureToBodyPartDefinition(editor->creatureDefinition, torso);
    AssignBrainIO(editor->creatureDefinition);
    RecalculateBodyPartDrawOrder(editor->creatureDefinition);
}

#undef EditorDefaultParameters
