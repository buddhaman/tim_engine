
#define GuiDefaultParameters(gui)\
    BasicRenderTools *renderTools = gui->renderTools;\
    Camera2D *camera = renderTools->camera;(void)camera;\
    Camera2D *screenCamera = renderTools->screenCamera;(void)screenCamera;\
    Assets *assets = renderTools->assets;\
    AtlasRegion *circleRegion = assets->defaultAtlas->regions;(void)circleRegion;\
    AtlasRegion *squareRegion = assets->defaultAtlas->regions+1;(void)squareRegion;\
    FontRenderer *fontRenderer = assets->fontRenderer;(void)fontRenderer;

global_variable U32 default_hash_seed = 5381;

internal inline GuiId
GuiHash(const U8 *input, U32 lengthInBytes, U32 seed)
{
    U32 hash = seed;
    U32 c;
    for(U32 i = 0; i < lengthInBytes; i++)
    {
        c = *input++;
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

internal inline GuiId
GuiHashCString(char *string, U32 seed)
{
    U32 length = (U32)strlen(string);
    return GuiHash((U8*)string, length, seed);
}

internal inline GuiId
GuiHashI32(I32 value)
{
    return GuiHash((U8*)&value, 4, default_hash_seed);
}

internal inline GuiId
GuiHashTupleI32(I32 value1, I32 value2)
{
    I32 values[2] = {value1, value2};
    return GuiHash((U8 *)values, 8, default_hash_seed);
}

internal inline GuiId
GuiHashPointer(void *pointer)
{
    return GuiHash((U8 *)&pointer, sizeof(void*), default_hash_seed);
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
GuiGetAnimation(Gui *gui, GuiId id, B32 findNew)
{
    U32 mask = MAX_GUI_ANIMATIONS-1;   
    U32 startIdx = id & mask;
    for(U32 animationSlotOffset = 0;
            animationSlotOffset < MAX_GUI_ANIMATIONS;
            animationSlotOffset++)
    {
        U32 animationIdx = (animationSlotOffset+startIdx) & mask;
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
GuiTriggerAnimation(Gui *gui, GuiId id, R32 duration)
{
    R32 FPS = 60.0;     // TODO: Replace, read from somewhere
    GuiAnimation *animation = GuiGetAnimation(gui, id, 1);
    animation->timeFactor = 0;
    animation->timeStep = 1.0f/(duration*FPS);
    animation->isActive = 1;
}

internal inline R32
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

internal inline B32
GuiIsHot(Gui *gui, GuiId id)
{
    return gui->prevHot==id;
}

internal inline B32
GuiIsActive(Gui *gui, GuiId id)
{
    return gui->prevActive==id;
}

internal inline B32
GuiHasCapturedInput(Gui *gui)
{
    return !!gui->prevActive;
}

internal inline B32
GuiMouseEnteredWidget(Gui *gui, GuiId widgetId)
{
    return widgetId==gui->mouseEnteredWidgetId;
}

internal inline void
GuiPushWorldCircle(Gui *gui, Vec2 worldPos, R32 screenRadius, Vec4 color)
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
DoButtonLogic(Gui *gui, GuiId id, B32 hit)
{
    B32 isActive = GuiIsActive(gui, id);
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
    B32 isHot = GuiIsHot(gui, id);
    B32 isActive = GuiIsActive(gui, id);
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

B32
DoCircularButtonOnPress(Gui *gui, GuiId id, Vec2 pos, R32 screenRadius)
{
    GuiDefaultParameters(gui);
    R32 radius = camera->scale*screenRadius;
    B32 hit = CirclePointIntersect(pos, radius, camera->mousePos);
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
        R32 minWidth, 
        R32 minHeight, 
        Rect2 *bounds, 
        Vec2 *textPos)
{
    GuiDefaultParameters(gui);
    *bounds = GetStringSize(fontRenderer, text, pos);
    // text offset
    R32 dx = 0;
    R32 dy = pos.y-bounds->pos.y; 
    R32 H = 0;
    R32 W = 0;
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
    *textPos = V2(pos.x+dx, pos.y+dy);
}

void
GuiPushRoundedRect(Gui *gui, Vec2 pos, Vec2 dims, R32 radius, Vec4 color)
{
    GuiDefaultParameters(gui);

#if 1
    Push2DRectColored(renderTools->screenRenderGroup, 
            V2(pos.x, pos.y+radius), V2(dims.x, dims.y-radius*2),
            squareRegion, color);
    Push2DRectColored(renderTools->screenRenderGroup,
            V2(pos.x+radius, pos.y), V2(dims.x-radius*2, dims.y), 
            squareRegion, color);
#endif

    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+radius, pos.y+radius),
            radius, circleRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+dims.x-radius, pos.y+radius),
            radius, circleRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+dims.x-radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
}

void
GuiPushBottomRoundedRect(Gui *gui, Vec2 pos, Vec2 dims, R32 radius, Vec4 color)
{
    GuiDefaultParameters(gui);

    Push2DRectColored(renderTools->screenRenderGroup, pos, 
            V2(dims.x, dims.y-radius), squareRegion, color);
    Push2DRectColored(renderTools->screenRenderGroup,
            V2(pos.x+radius, pos.y+dims.y-radius*2), 
            V2(dims.x-radius*2, radius*2), squareRegion, color);

    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
    Push2DCircleColored(renderTools->screenRenderGroup, 
            V2(pos.x+dims.x-radius, pos.y+dims.y-radius),
            radius, circleRegion, color);
}

void
GuiPushRect(Gui *gui, Vec2 pos, Vec2 dims, Vec4 color)
{
    GuiDefaultParameters(gui);
    Push2DRectColored(renderTools->screenRenderGroup, pos, dims, squareRegion, color);
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
    Push2DTextColored(renderTools->screenRenderGroup, fontRenderer, textPos, text, V4(0,0,0,1));
    Push2DText(renderTools->screenRenderGroup, fontRenderer, V2Add(textPos, V2(2, -2)), text);
    return textRect;
}

B32
DoLabelButton(Gui *gui, char *label, Vec2 pos, Vec2 minDims)
{
    GuiDefaultParameters(gui);
    GuiId id = GuiGetNameHash(gui, label);

    //Vec4 color = GetDe
    Rect2 textRect = DoLabelWithBackground(gui, label, pos, minDims, gui->defaultColor);

    B32 hit = BoxPoint2Intersect(textRect.pos, textRect.dims, screenCamera->mousePos);
    DoButtonLogic(gui, id, hit);
    //Vec4 color = GetButtonColor(gui, id);

    //Push2DRectColored(renderTools->screenRenderGroup, textRect.pos, textRect.dims, squareRegion, color);
    //Push2DText(renderTools->screenRenderGroup, fontRenderer, 
    //V2(pos.x, pos.y+dy), label);
    ///Push2DCircleColored(renderTools->screenRenderGroup, pos, 1, circleRegion, V4(1,0,0,1));

    return gui->isMouseJustReleased && GuiIsActive(gui, id);
}

B32
DoTabBarRadioButton(Gui *gui, char *label, Vec2 pos, Vec2 minDims, B32 isEnabled)
{
    GuiDefaultParameters(gui);

    GuiId id = GuiGetNameHash(gui, label);
    B32 mouseEntered = GuiMouseEnteredWidget(gui, id);
    B32 isActive = GuiIsActive(gui, id);
    B32 isHot = GuiIsHot(gui, id);

    R32 shade = 0.5;
    R32 elevation = isActive ? 4.0 : 8.0;
    R32 hotFactor = 1.5f;
    R32 enabledFactor = 2.0f;
    R32 heightFactor = 1.0f;
    R32 animationDuration = 0.1;
    B32 justPressed = gui->isMouseJustReleased && isActive;

    if(!isEnabled)
    {
        if(mouseEntered)
        {
            GuiTriggerAnimation(gui, id, animationDuration);
        }
        if(isHot)
        {
            R32 animation = UpdateAnimation(gui, id);
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
        R32 animation = UpdateAnimation(gui, id);
        heightFactor = Lerp(hotFactor, enabledFactor, animation);
    }

    Vec4 color = isActive ? gui->pressedColor : (isHot ? gui->hitColor : gui->defaultColor);
    Vec4 bgColor = V4(color.x*shade, color.y*shade, color.z*shade, color.w);

    minDims.y*=heightFactor;

    GuiPushBottomRoundedRect(gui, pos, minDims, 10, bgColor);
    GuiPushBottomRoundedRect(gui, pos, V2(minDims.x, minDims.y-elevation), 10, color);

    B32 hit = BoxPoint2Intersect(pos, minDims, screenCamera->mousePos);

    DoLabel(gui, label, V2(pos.x, pos.y+minDims.y-32), V2(minDims.x, 16));
    DoButtonLogic(gui, id, hit);
    return justPressed;
}

void
DoTextField(Gui *gui, char *name, char *text, Vec2 pos, Vec2 minDims)
{
    GuiDefaultParameters(gui);

    Vec4 bgColor = gui->defaultColor;

    strcat(text, gui->textInput);
    GuiPushRoundedRect(gui, pos, minDims, 4, bgColor);
    DoLabel(gui, text, pos, V2(0,minDims.y));
}

// Returns true if dragging
B32
DoDragCircularButtonOnPress(Gui *gui, char *name, Vec2 pos, R32 screenRadius)
{
    B32 result = 0;
    GuiId id = GuiGetNameHash(gui, name);
    B32 isDragging = GuiIsActive(gui, id);
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

B32
DoAngleLengthButton(Gui *gui,
        char *name,
        Vec2 center,
        R32 *angle,
        R32 *length,
        R32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 pos = V2Add(center, V2Polar(*angle, *length));
    B32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);
    if(isDragging)
    {
        Vec2 mousePos = camera->mousePos;
        *angle = atan2f(mousePos.y-center.y, mousePos.x-center.x);
        *length = V2Dist(center, mousePos);
    }
    return isDragging;
}

B32
DoRotationDragButton(Gui *gui, 
        char *name,
        Vec2 center, 
        R32 *angle, 
        R32 length,
        R32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 mousePos = camera->mousePos;
    Vec2 pos = V2Add(center, V2Polar(*angle, length));
    B32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);

    if(isDragging)
    {
        *angle  = atan2f(mousePos.y-center.y, mousePos.x-center.x);
    }
    return isDragging;
}

B32
DoRotationDragButtonWithLine(Gui *gui,
        char *name,
        Vec2 center,
        R32 *angle,
        R32 length,
        R32 screenRadius,
        Vec4 lineColor)
{
    GuiDefaultParameters(gui);
    R32 lineWidth = 2.0 *camera->scale;
    Vec2 to = V2Add(center, V2Polar(*angle, length));
    Push2DLineColored(renderTools->worldRenderGroup, center, to, lineWidth, squareRegion, lineColor);
    return DoRotationDragButton(gui, name, center, angle, length, screenRadius);
}

B32
DoDragButtonAlongAxis(Gui *gui, 
        char *name, 
        Vec2 axisOrigin, 
        Vec2 axis, 
        R32 *length, 
        R32 screenRadius)
{
    GuiDefaultParameters(gui);
    Vec2 mousePos = camera->mousePos;
    Vec2 pos = V2Add(axisOrigin, V2MulS(axis, *length));

    B32 isDragging = DoDragCircularButtonOnPress(gui, name, pos, screenRadius);
    if(isDragging)
    {
        R32 dotPos = V2Dot(axis, mousePos);
        R32 dotOffset = V2Dot(axis, axisOrigin);
        *length = dotPos-dotOffset;
    }
    return isDragging;
}

int
DoRadialMenuV(Gui *gui, Vec2 center, B32 isActive, int nItems, va_list args)
{   
    GuiDefaultParameters(gui);
    int result = -1;
    if(gui->isRadialMenuActive)
    {
        RenderGroup *renderGroup = renderTools->screenRenderGroup;
        FontRenderer *fontRenderer = renderTools->assets->fontRenderer;
        R32 dAngle = 2*M_PI/nItems;
        for(U32 itemIdx = 0;
                itemIdx < nItems;
                itemIdx++)
        {
            R32 angle = itemIdx*dAngle-M_PI/2.0;
            R32 radius = 120.0;
            R32 renderRadius = radius * gui->radialTimer;
            R32 c = cosf(angle);
            R32 s = sinf(angle);

            B32 hit = 0;
            R32 dx = gui->appState->screenCamera->mousePos.x - gui->radialMenuPos.x;
            R32 dy = gui->appState->screenCamera->mousePos.y - gui->radialMenuPos.y;
            R32 mAngle = atan2f(dy, dx);
            R32 l2 = dx*dx + dy*dy;
            R32 angDif = fabsf(GetNormalizedAngDiff(angle, mAngle));
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
            Vec2 dims = GetStringSize(assets->fontRenderer, text, V2(0,0)).dims;
            Push2DTextColored(renderGroup, 
                    fontRenderer, 
                    V2(gui->radialMenuPos.x+c*renderRadius-dims.x/2, gui->radialMenuPos.y+s*renderRadius+dims.y/2), 
                    text,
                    hit ? V4(1,1,0,1) : V4(1,1,1,1));
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
DoRadialMenu(Gui *gui, Vec2 center, B32 isActive, int nItems, ...)
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
    strcpy(gui->textInput, gui->appState->textInput);

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

    Vec4 col = RGBAToVec4(0xEBA487FF);
    R32 s = 1.0f;
    s = 1.0f; gui->defaultColor = V4(s*col.r, s*col.g, s*col.b, 1.0f);
    s = 1.0f/0.9f;; gui->hitColor = V4(s*col.r, s*col.g, s*col.b, 1.0f);
    s = 0.6f; gui->pressedColor = V4(s*col.r, s*col.g, s*col.b, 1.0f);
}

#undef GuiDefaultParameters

