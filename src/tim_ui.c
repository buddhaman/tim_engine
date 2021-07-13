
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

b32
GuiMouseEnteredWidget(Gui *gui, GuiId id)
{
    return gui->mouseEnteredWidget==id;
}

GuiAnimation *
GuiGetAnimation(Gui *gui, GuiId id, b32 findActive)
{
    for(ui32 hashTry = 0;
            hashTry < MAX_GUI_ANIMATIONS;
            hashTry++)
    {
        ui32 hashMask = MAX_GUI_ANIMATIONS-1;
        ui32 animationHashIndex = id & hashMask;
        GuiAnimation *animation = gui->animations+animationHashIndex;
        if(!animation->isActive)
        {
            if(findActive)
            {
                return NULL;
            }
            else
            {
                return animation;
            }
        }
        else
        {
            if(animation->id==id)
            {
                return animation;
            }
        }
    }
    Assert(0);
    return NULL;
}

GuiAnimation *
TriggerAnimation(Gui *gui, GuiId id, r32 durationInSeconds)
{
    r32 fps = 60.0;
    // TODO: solve hash collisions.
    GuiAnimation *animation = GuiGetAnimation(gui, id, 0);
    animation->id = id;
    animation->t = 0;
    animation->updateStep = 1.0/(fps*durationInSeconds);
    animation->isActive = 1;
    return animation;
}

r32
UpdateAnimation(Gui *gui, GuiId id)
{
    GuiAnimation *animation = GuiGetAnimation(gui, id, 1);
    if(animation)
    {
        Assert(animation->isActive);
        animation->t+=animation->updateStep;
        if(animation->t >= 1.0) 
        {
            animation->isActive = 0;
            animation->t = 1.0;
        }
        return animation->t;
    }
    else
    {
        return 1.0;
    }
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

b32 
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
    return gui->isMouseJustReleased && GuiIsActive(gui, id);
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
DoTabBarRadioButton(Gui *gui, char *label, Vec2 pos, Vec2 minDims, b32 selected)
{
    GuiDefaultParameters(gui);
    GuiId id = GuiGetNameHash(gui, label);

    b32 hot = GuiIsHot(gui, id);
    b32 active = GuiIsActive(gui, id);
    r32 shade = 0.5;
    r32 elevation = active ? 6 : 8;
    Vec4 color = GetButtonColor(gui, id);

    Vec4 bgColor = vec4(color.x*shade, color.y*shade, color.z*shade, color.w);

    r32 heightFactor = 1.0;
    if(!selected && GuiMouseEnteredWidget(gui, id))
    {
        //DebugOut("Triggering animation");
        TriggerAnimation(gui, id, 0.15);
    }
    r32 t = UpdateAnimation(gui, id);
    if(!selected && hot)
    {
        heightFactor = (1.0-t) + 1.5*t;
    }

    minDims.y = selected ? minDims.y*2 : (hot ? minDims.y*heightFactor : minDims.y);
    b32 hit = BoxPoint2Intersect(pos, minDims, screenCamera->mousePos);

    GuiPushBottomRoundedRect(gui, pos, minDims, 10, bgColor);
    GuiPushBottomRoundedRect(gui, pos, vec2(minDims.x, minDims.y-elevation), 10, color);

    DoLabel(gui, label, vec2(pos.x, pos.y+minDims.y-elevation-24), vec2(minDims.x, 24));

    return DoButtonLogic(gui, id, hit);
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
    if(gui->prevHot!=gui->hot)
    {
        gui->mouseEnteredWidget = gui->hot;
    }
    else
    {
        gui->mouseEnteredWidget = 0;
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

    gui->nAnimations = 0;
}

#undef GuiDefaultParameters

