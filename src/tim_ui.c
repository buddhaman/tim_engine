
#define GuiDefaultParameters(gui)\
    BasicRenderTools *renderTools = gui->renderTools;\
    Camera2D *camera = renderTools->camera;(void)camera;\
    Camera2D *screenCamera = renderTools->screenCamera;(void)screenCamera;\
    Assets *assets = renderTools->assets;\
    AtlasRegion *circleRegion = assets->defaultAtlas->regions;(void)circleRegion;\
    AtlasRegion *squareRegion = assets->defaultAtlas->regions+1;(void)squareRegion;\
    FontRenderer *fontRenderer = assets->fontRenderer;(void)fontRenderer;

internal inline GuiId
GuiHash(const ui8 *input, ui32 lengthInBytes)
{
    ui32 hash = 5381;
    ui32 c;
    for(ui32 i = 0; i < lengthInBytes; i++)
    {
        c = *input++;
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

internal inline GuiId
GuiHashCString(char *string)
{
    ui32 length = (ui32)strlen(string);
    return GuiHash((ui8*)string, length);
}

internal inline GuiId
GuiHashI32(i32 value)
{
    return GuiHash((ui8*)&value, 4);
}

internal inline GuiId
GuiHashTupleI32(i32 value1, i32 value2)
{
    i32 values[2] = {value1, value2};
    return GuiHash((ui8 *)values, 8);
}

internal inline GuiId
GuiHashPointer(void *pointer)
{
    return GuiHash((ui8 *)&pointer, sizeof(void*));
}

internal inline GuiId
GuiGetNameHash(Gui *gui, char *name)
{
    // TODO: Use current context in hash to ensure uniequeness.
    return GuiHashCString(name);
}

b32
GuiIsHot(Gui *gui, GuiId id)
{
    return gui->prevHot==id;
}

b32
GuiIsActive(Gui *gui, GuiId id)
{
    return gui->prevActive==id;
}

b32
GuiHasCapturedInput(Gui *gui)
{
    return !!gui->prevActive;
}

internal inline void
GuiPushWorldCircle(Gui *gui, Vec2 worldPos, r32 screenRadius, Vec4 color)
{
    RenderGroup *renderGroup = gui->renderTools->worldRenderGroup;
    Assets *assets = gui->renderTools->assets;
    Push2DCircleColored(renderGroup, 
            worldPos, 
            screenRadius*gui->renderTools->camera->scale, 
            assets->defaultAtlas->regions, 
            color);
}

void 
DoButtonLogic(Gui *gui, GuiId id, b32 hit)
{
    b32 isActive = GuiIsActive(gui, id);
    if(hit)
    {
        gui->hot = id;
        if((isActive && gui->isMouseDown) || 
                IsKeyActionJustDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT))
        {
            gui->active = id;
        }
    }
}

Vec4
GetButtonColor(Gui *gui, GuiId id)
{
    b32 isHot = GuiIsHot(gui, id);
    b32 isActive = GuiIsActive(gui, id);
    if(isActive)
    {
        return gui->pressedColor;
    }
    else if(isHot)
    {
        return gui->hitColor;
    }
    else
    {
        return gui->defaultColor;
    }
}

b32
DoCircularButtonOnPress(Gui *gui, GuiId id, Vec2 pos, r32 screenRadius)
{
    GuiDefaultParameters(gui);
    r32 radius = camera->scale*screenRadius;
    b32 hit = CirclePointIntersect(pos, radius, camera->mousePos);
    DoButtonLogic(gui, id, hit);
    Vec4 color = GetButtonColor(gui, id);
    GuiPushWorldCircle(gui, pos, screenRadius, color);
    return hit & IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
DoLabelButton(Gui *gui, char *label, Vec2 pos)
{
    GuiDefaultParameters(gui);
    GuiId id = GuiGetNameHash(gui, label);

    Rect2 textRect = GetStringSize(fontRenderer, label, pos);
    Rect2 rect = textRect;
    r32 dy = pos.y-rect.pos.y;
    rect.pos.y+=dy;

    b32 hit = BoxPoint2Intersect(rect.pos, rect.dims, screenCamera->mousePos);
    DoButtonLogic(gui, id, hit);
    Vec4 color = GetButtonColor(gui, id);

    Push2DRectColored(renderTools->screenRenderGroup, rect.pos, rect.dims, squareRegion, color);
    Push2DText(renderTools->screenRenderGroup, fontRenderer, 
            vec2(pos.x, pos.y+dy), label);
    Push2DCircleColored(renderTools->screenRenderGroup, pos, 1, circleRegion, vec4(1,0,0,1));

    return gui->isMouseJustReleased && GuiIsHot(gui, id);
}

// Returns true if dragging
b32
DoDragCircularButtonOnPress(Gui *gui, char *name, Vec2 pos, r32 screenRadius)
{
    b32 result = 0;
    GuiId id = GuiGetNameHash(gui, name);
    b32 isDragging = GuiIsActive(gui, id);
    if(isDragging)
    {
        result = IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
        if(result) 
        {
            gui->active = id;
        }
        GuiPushWorldCircle(gui, pos, screenRadius, gui->pressedColor);
    }
    else
    {
        result = DoCircularButtonOnPress(gui, id, pos, screenRadius);
    }
    return result;
}

b32
DoAngleLengthButton(Gui *gui,
        char *name,
        Vec2 center,
        r32 *angle,
        r32 *length,
        r32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 pos = v2_add(center, v2_polar(*angle, *length));
    b32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);
    if(isDragging)
    {
        Vec2 mousePos = camera->mousePos;
        *angle = atan2f(mousePos.y-center.y, mousePos.x-center.x);
        *length = v2_dist(center, mousePos);
    }
    return isDragging;
}

b32
DoRotationDragButton(Gui *gui, 
        char *name,
        Vec2 center, 
        r32 *angle, 
        r32 length,
        r32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 mousePos = camera->mousePos;
    Vec2 pos = v2_add(center, v2_polar(*angle, length));
    b32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);

    if(isDragging)
    {
        *angle  = atan2f(mousePos.y-center.y, mousePos.x-center.x);
    }
    return isDragging;
}

b32
DoRotationDragButtonWithLine(Gui *gui,
        char *name,
        Vec2 center,
        r32 *angle,
        r32 length,
        r32 screenRadius,
        Vec4 lineColor)
{
    GuiDefaultParameters(gui);
    r32 lineWidth = 2.0 *camera->scale;
    Vec2 to = v2_add(center, v2_polar(*angle, length));
    Push2DLineColored(renderTools->worldRenderGroup, center, to, lineWidth, squareRegion, lineColor);
    return DoRotationDragButton(gui, name, center, angle, length, screenRadius);
}

b32
DoDragButtonAlongAxis(Gui *gui, 
        char *name, 
        Vec2 axisOrigin, 
        Vec2 axis, 
        r32 *length, 
        r32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 mousePos = camera->mousePos;
    Vec2 pos = v2_add(axisOrigin, v2_muls(axis, *length));

    b32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);
    if(isDragging)
    {
        r32 dotPos = v2_dot(axis, mousePos);
        r32 dotOffset = v2_dot(axis, axisOrigin);
        *length = dotPos-dotOffset;
    }
    return isDragging;
}

int
DoRadialMenuV(Gui *gui, Vec2 center, b32 isActive, int nItems, va_list args)
{   
    GuiDefaultParameters(gui);
    int result = -1;
    if(gui->isRadialMenuActive)
    {
        RenderGroup *renderGroup = renderTools->screenRenderGroup;
        FontRenderer *fontRenderer = renderTools->assets->fontRenderer;
        r32 dAngle = 2*M_PI/nItems;
        for(ui32 itemIdx = 0;
                itemIdx < nItems;
                itemIdx++)
        {
            r32 angle = itemIdx*dAngle-M_PI/2.0;
            r32 radius = 120.0;
            r32 renderRadius = radius * gui->radialTimer;
            r32 c = cosf(angle);
            r32 s = sinf(angle);

            b32 hit = 0;
            r32 dx = gui->appState->screenCamera->mousePos.x - gui->radialMenuPos.x;
            r32 dy = gui->appState->screenCamera->mousePos.y - gui->radialMenuPos.y;
            r32 mAngle = atan2f(dy, dx);
            r32 l2 = dx*dx + dy*dy;
            r32 angDif = fabsf(GetNormalizedAngDiff(angle, mAngle));
            if(angDif < dAngle/2.0 && l2 > radius*radius*0.8)
            {
                hit = 1;
            }

            // Check if selected. Only right mouse button.
            if(IsKeyActionJustReleased(gui->appState, ACTION_MOUSE_BUTTON_RIGHT) && hit)
            {
                result = itemIdx;
            }

            char *text = va_arg(args, char*);
            Vec2 dims = GetStringSize(assets->fontRenderer, text, vec2(0,0)).dims;
            Push2DTextColored(renderGroup, 
                    fontRenderer, 
                    vec2(gui->radialMenuPos.x+c*renderRadius-dims.x/2, gui->radialMenuPos.y+s*renderRadius+dims.y/2), 
                    text,
                    hit ? vec4(1,1,0,1) : vec4(1,1,1,1));
        }
    }
    if(!gui->isRadialMenuActive && isActive)
    {
        // Just activated.
        gui->radialTimer = 0.0;
        gui->radialMenuPos = center;
        gui->isRadialMenuActive = 1;
    }
    if(!isActive)
    {
        gui->isRadialMenuActive = 0;
    }
    return result;
}

int
DoRadialMenu(Gui *gui, Vec2 center, b32 isActive, int nItems, ...)
{
    va_list args;
    va_start(args, nItems);
    int result = DoRadialMenuV(gui, center, isActive, nItems, args);
    va_end(args);
    return result;
}

void
GuiUpdate(Gui *gui)
{
    GuiDefaultParameters(gui);
    if(gui->isRadialMenuActive && gui->radialTimer < 1.0)
    {
        gui->radialTimer+=(1.0/10.0);   // Sixth of a second.
        if(gui->radialTimer > 1.0)
        {
            gui->radialTimer = 1.0;
        }
    }
    ExecuteAndFlushRenderGroup(renderTools->worldRenderGroup, 
            assets, renderTools->worldShader);
    ExecuteAndFlushRenderGroup(renderTools->screenRenderGroup, 
            assets, renderTools->screenShader);
    gui->prevActive = gui->active;
    gui->prevHot = gui->hot;
    gui->active = 0;
    gui->hot = 0;
    gui->isMouseJustReleased = IsKeyActionJustReleased(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    gui->isMouseDown = IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
}

void
InitGui(Gui *gui, 
        MemoryArena *arena,
        AppState *appState, 
        Camera2D *camera,
        Assets *assets)
{
    gui->appState = appState;

    gui->renderTools = PushStruct(arena, BasicRenderTools);
    InitRenderTools(arena, gui->renderTools, assets, camera, appState->screenCamera);

    gui->defaultColor = vec4(0.6, 0.3, 0.3, 1.0);
    gui->hitColor = vec4(0.8, 0.5, 0.5, 1.0);
    gui->pressedColor = vec4(0.4, 0.2, 0.2, 1.0);
}

#undef GuiDefaultParameters

