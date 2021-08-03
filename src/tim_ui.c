
#define GuiDefaultParameters(gui)\
    BasicRenderTools *renderTools = gui->renderTools;\
    Camera2D *camera = renderTools->camera;(void)camera;\
    Camera2D *screenCamera = renderTools->screenCamera;(void)screenCamera;\
    Assets *assets = renderTools->assets;\
    AtlasRegion *circleRegion = assets->defaultAtlas->regions;(void)circleRegion;\
    AtlasRegion *squareRegion = assets->defaultAtlas->regions+1;(void)squareRegion;\
    FontRenderer *fontRenderer = assets->fontRenderer;(void)fontRenderer;

global_variable ui32 default_hash_seed = 5381;

internal inline GuiId
GuiHash(const ui8 *input, ui32 lengthInBytes, ui32 seed)
{
    ui32 hash = seed;
    ui32 c;
    for(ui32 i = 0; i < lengthInBytes; i++)
    {
        c = *input++;
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

internal inline GuiId
GuiHashCString(char *string, ui32 seed)
{
    ui32 length = (ui32)strlen(string);
    return GuiHash((ui8*)string, length, seed);
}

internal inline GuiId
GuiHashI32(i32 value)
{
    return GuiHash((ui8*)&value, 4, default_hash_seed);
}

internal inline GuiId
GuiHashTupleI32(i32 value1, i32 value2)
{
    i32 values[2] = {value1, value2};
    return GuiHash((ui8 *)values, 8, default_hash_seed);
}

internal inline GuiId
GuiHashPointer(void *pointer)
{
    return GuiHash((ui8 *)&pointer, sizeof(void*), default_hash_seed);
}

internal inline GuiId
GuiGetCurrentContextId(Gui *gui)
{
    if(gui->contextStackDepth)
    {
        return gui->contextStack[gui->contextStackDepth-1];
    }
    else
    {
        return 0;
    }
}

internal inline GuiId
GuiGetNameHash(Gui *gui, char *name)
{
    GuiId currentContextId = GuiGetCurrentContextId(gui);
    if(currentContextId)
    {
        return GuiHashCString(name, currentContextId);
    }
    else
    {
        return GuiHashCString(name, default_hash_seed);
    }
}

internal inline GuiAnimation*
GuiGetAnimation(Gui *gui, GuiId id, b32 findNew)
{
    ui32 mask = MAX_GUI_ANIMATIONS-1;   
    ui32 startIdx = id & mask;
    for(ui32 animationSlotOffset = 0;
            animationSlotOffset < MAX_GUI_ANIMATIONS;
            animationSlotOffset++)
    {
        ui32 animationIdx = (animationSlotOffset+startIdx) & mask;
        GuiAnimation *animation = gui->animations+animationIdx;
        if(animation->widgetId==id)
        {
            return animation;
        }
        else if(findNew && !animation->isActive)
        {
            return animation;
        }
        return animation;
    }
    return NULL;
}

internal inline void
GuiTriggerAnimation(Gui *gui, GuiId id, r32 duration)
{
    r32 FPS = 60.0;     // TODO: Replace, read from somewhere
    GuiAnimation *animation = GuiGetAnimation(gui, id, 1);
    animation->timeFactor = 0;
    animation->timeStep = 1.0f/(duration*FPS);
    animation->isActive = 1;
}

internal inline r32
UpdateAnimation(Gui *gui, GuiId id)
{
    GuiAnimation *animation = GuiGetAnimation(gui, id, 0);

    if(animation && animation->isActive)
    {
        animation->timeFactor+=animation->timeStep;
        if(animation->timeFactor >= 1.0)
        {
            animation->timeFactor = 1.0;
            animation->isActive = 0;
        }
        return animation->timeFactor;
    }
    else
    {
        return 1.0;
    }
}

internal inline b32
GuiIsHot(Gui *gui, GuiId id)
{
    return gui->prevHot==id;
}

internal inline b32
GuiIsActive(Gui *gui, GuiId id)
{
    return gui->prevActive==id;
}

internal inline b32
GuiHasCapturedInput(Gui *gui)
{
    return !!gui->prevActive;
}

internal inline b32
GuiMouseEnteredWidget(Gui *gui, GuiId widgetId)
{
    return widgetId==gui->mouseEnteredWidgetId;
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

void
GuiEndContext(Gui *gui)
{
    Assert(gui->contextStackDepth > 0);
    gui->contextStackDepth--;
}

void
GuiBeginContext(Gui *gui, char *name)
{
    Assert(gui->contextStackDepth < MAX_CONTEXT_STACK_SIZE);
    GuiId contextId = GuiGetNameHash(gui, name);
    gui->contextStack[gui->contextStackDepth++] = contextId;
}

void
GetLabelLayout(Gui *gui, 
        char *text, Vec2 pos, 
        r32 minWidth, 
        r32 minHeight, 
        Rect2 *bounds, 
        Vec2 *textPos)
{
    GuiDefaultParameters(gui);
    *bounds = GetStringSize(fontRenderer, text, pos);
    // text offset
    r32 dx = 0;
    r32 dy = pos.y-bounds->pos.y; 
    r32 H = 0;
    r32 W = 0;
    if(minHeight > bounds->dims.y)
    {
        H = minHeight-bounds->dims.y;
    }
    if(minWidth > bounds->dims.x)
    {
        W = minWidth-bounds->dims.x;
    }
    dx+=W/2;
    dy+=H/2;
    bounds->width+=W;
    bounds->height+=H;
    bounds->pos.y = pos.y;
    *textPos = vec2(pos.x+dx, pos.y+dy);
}

void
GuiPushBottomRoundedRect(Gui *gui, Vec2 pos, Vec2 dims, r32 radius, Vec4 color)
{
    GuiDefaultParameters(gui);
    Push2DRectColored(renderTools->screenRenderGroup, pos, 
            vec2(dims.x, dims.y-radius), squareRegion, color);
    Push2DRectColored(renderTools->screenRenderGroup,
            vec2(pos.x+radius, pos.y+dims.y-radius*2), 
            vec2(dims.x-radius*2, radius*2), squareRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            vec2(pos.x+radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            vec2(pos.x+dims.x-radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
}

Rect2
DoLabel(Gui *gui, char *text, Vec2 pos, Vec2 minDims)
{
    GuiDefaultParameters(gui);
    Rect2 textRect;
    Vec2 textPos;
    GetLabelLayout(gui, text, pos, minDims.x, minDims.y, &textRect, &textPos);
    Push2DText(renderTools->screenRenderGroup, fontRenderer, textPos, text);
    return textRect;
}

Rect2
DoLabelWithBackground(Gui *gui, char *text, Vec2 pos, Vec2 minDims, Vec4 backgroundColor)
{
    GuiDefaultParameters(gui);
    Rect2 textRect;
    Vec2 textPos;
    GetLabelLayout(gui, text, pos, minDims.x, minDims.y, &textRect, &textPos);
    Push2DRectColored(renderTools->screenRenderGroup, textRect.pos, textRect.dims, squareRegion, backgroundColor);
    Push2DTextColored(renderTools->screenRenderGroup, fontRenderer, textPos, text, vec4(0,0,0,1));
    Push2DText(renderTools->screenRenderGroup, fontRenderer, v2_add(textPos, vec2(2, -2)), text);
    return textRect;
}

b32
DoLabelButton(Gui *gui, char *label, Vec2 pos, Vec2 minDims)
{
    GuiDefaultParameters(gui);
    GuiId id = GuiGetNameHash(gui, label);

    //Vec4 color = GetDe
    Rect2 textRect = DoLabelWithBackground(gui, label, pos, minDims, gui->defaultColor);

    b32 hit = BoxPoint2Intersect(textRect.pos, textRect.dims, screenCamera->mousePos);
    DoButtonLogic(gui, id, hit);
    //Vec4 color = GetButtonColor(gui, id);

    //Push2DRectColored(renderTools->screenRenderGroup, textRect.pos, textRect.dims, squareRegion, color);
    //Push2DText(renderTools->screenRenderGroup, fontRenderer, 
    //vec2(pos.x, pos.y+dy), label);
    ///Push2DCircleColored(renderTools->screenRenderGroup, pos, 1, circleRegion, vec4(1,0,0,1));

    return gui->isMouseJustReleased && GuiIsActive(gui, id);
}

b32
DoTabBarRadioButton(Gui *gui, char *label, Vec2 pos, Vec2 minDims, b32 isEnabled)
{
    GuiDefaultParameters(gui);

    GuiId id = GuiGetNameHash(gui, label);
    b32 mouseEntered = GuiMouseEnteredWidget(gui, id);
    b32 isActive = GuiIsActive(gui, id);
    b32 isHot = GuiIsHot(gui, id);

    r32 shade = 0.5;
    r32 elevation = isActive ? 4.0 : 8.0;
    r32 hotFactor = 1.5f;
    r32 enabledFactor = 2.0f;
    r32 heightFactor = 1.0f;
    r32 animationDuration = 0.1;
    b32 justPressed = gui->isMouseJustReleased && isActive;

    if(!isEnabled)
    {
        if(mouseEntered)
        {
            GuiTriggerAnimation(gui, id, animationDuration);
        }
        if(isHot)
        {
            r32 animation = UpdateAnimation(gui, id);
            heightFactor = Lerp(1.0, hotFactor, animation);
        }
        // Trigger animation if just enabled
        if(justPressed)
        {
            GuiTriggerAnimation(gui, id, animationDuration);
        }
    }
    else
    {
        r32 animation = UpdateAnimation(gui, id);
        heightFactor = Lerp(hotFactor, enabledFactor, animation);
    }

    Vec4 color = isActive ? gui->pressedColor : (isHot ? gui->hitColor : gui->defaultColor);
    Vec4 bgColor = vec4(color.x*shade, color.y*shade, color.z*shade, color.w);

    minDims.y*=heightFactor;

    GuiPushBottomRoundedRect(gui, pos, minDims, 10, bgColor);
    GuiPushBottomRoundedRect(gui, pos, vec2(minDims.x, minDims.y-elevation), 10, color);

    b32 hit = BoxPoint2Intersect(pos, minDims, screenCamera->mousePos);

    DoLabel(gui, label, vec2(pos.x, pos.y+minDims.y-32), vec2(minDims.x, 16));
    DoButtonLogic(gui, id, hit);
    return justPressed;
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
    if(gui->prevHot != gui->hot)
    {
        gui->mouseEnteredWidgetId = gui->hot;
    }
    else
    {
        gui->mouseEnteredWidgetId = 0;
    }
    gui->prevActive = gui->active;
    gui->prevHot = gui->hot;
    gui->active = 0;
    gui->hot = 0;
    gui->isMouseJustReleased = IsKeyActionJustReleased(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    gui->isMouseDown = IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);


    Assert(gui->contextStackDepth==0);
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

