
// NUKLEAR STUFF

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
NKEditFloatPropertyWithTooltip(struct nk_context *ctx, 
        char *label, 
        char *tooltip,
        r32 min, 
        r32 *value, 
        r32 max, 
        r32 stepSize, 
        r32 incPerPixel)
{
    if(nk_widget_is_hovered(ctx))
    {
        nk_tooltip(ctx, tooltip);
    }
    return NKEditFloatProperty(ctx, label, min, value, max, stepSize, incPerPixel);
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

internal inline void
GuiPushCircle(Gui *gui, Vec2 pos, r32 radius, Vec4 color)
{
    RenderGroup *renderGroup = gui->worldRenderGroup;
    Assets *assets = gui->assets;
    Push2DCircleColored(renderGroup, pos, radius, assets->defaultAtlas->regions, color);
}

b32
DoCircularButtonOnPress(Gui *gui, Vec2 pos, r32 radius)
{
    Vec4 color;
    b32 hit = CirclePointIntersect(pos, radius, gui->camera->mousePos);
    if(hit)
    {
        if(IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT))
        {
            color = gui->pressedColor;
        }
        else
        {
            color = gui->hitColor;
        }
    }
    else
    {
        color = gui->defaultColor;
    }
    GuiPushCircle(gui, pos, radius, color);
    return hit & IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
}

b32
DoAngleLengthButton(Gui *gui,
        Vec2 center,
        r32 *angle,
        r32 *length,
        r32 radius,
        b32 isDragging)
{
    Vec2 mousePos = gui->camera->mousePos;
    Vec2 pos = v2_add(center, v2_polar(*angle, *length));
    if(isDragging)
    {
        *angle = atan2f(mousePos.y-center.y, mousePos.x-center.x);
        *length = v2_dist(center, mousePos);
        GuiPushCircle(gui, pos, radius, gui->pressedColor);
        return IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    }
    else
    {
        return DoCircularButtonOnPress(gui, pos, radius);
    }
}

b32
DoRotationDragButton(Gui *gui, 
        Vec2 center, 
        r32 *angle, 
        r32 length,
        r32 radius, 
        b32 isDragging)
{
    Vec2 mousePos = gui->camera->mousePos;
    Vec2 pos = v2_add(center, v2_polar(*angle, length));

    if(isDragging)
    {
        *angle  = atan2f(mousePos.y-center.y, mousePos.x-center.x);
        GuiPushCircle(gui, pos, radius, gui->pressedColor);
        return IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    }
    else
    {
        return DoCircularButtonOnPress(gui, pos, radius);
    }
}

b32
DoDragButtonAlongAxis(Gui *gui, Vec2 axisOrigin, Vec2 axis, r32 *length, r32 radius, b32 isDragging)
{
    Vec4 color = gui->defaultColor;
    Vec2 mousePos = gui->camera->mousePos;
    Vec2 pos = v2_add(axisOrigin, v2_muls(axis, *length));

    if(isDragging)
    {
        r32 dotPos = v2_dot(axis, mousePos);
        r32 dotOffset = v2_dot(axis, axisOrigin);
        *length = dotPos-dotOffset;
        color = gui->pressedColor;
        GuiPushCircle(gui, pos, radius, color);
        return IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    }
    else
    {
        return DoCircularButtonOnPress(gui, pos, radius);
    }
}

int
DoRadialMenuV(Gui *gui, Vec2 center, b32 isActive, int nItems, va_list args)
{   
    int result = -1;
    if(gui->isRadialMenuActive)
    {
        RenderGroup *renderGroup = gui->screenRenderGroup;
        FontRenderer *fontRenderer = gui->assets->fontRenderer;
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
            Vec2 dims = GetStringSize(gui->assets->fontRenderer, text);
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
GuiUpdate(Gui *gui, Camera2D *screenCamera, Camera2D *worldCamera)
{
    if(gui->isRadialMenuActive && gui->radialTimer < 1.0)
    {
        gui->radialTimer+=(1.0/10.0);   // Sixth of a second.
        if(gui->radialTimer > 1.0)
        {
            gui->radialTimer = 1.0;
        }
    }
    ExecuteAndFlushRenderGroup(gui->worldRenderGroup, 
            gui->assets, gui->worldShader);
    ExecuteAndFlushRenderGroup(gui->screenRenderGroup, 
            gui->assets, gui->screenShader);
}

void
InitGui(Gui *gui, 
        MemoryArena *arena,
        AppState *appState, 
        Camera2D *camera,
        Assets *assets,
        ShaderInstance *worldShader,
        ShaderInstance *screenShader)
{
    gui->appState = appState;
    gui->camera = camera;
    gui->assets = assets;
    gui->worldShader = worldShader;
    gui->screenShader = screenShader;
    gui->defaultColor = vec4(0.6, 0.3, 0.3, 1.0);
    gui->hitColor = vec4(0.8, 0.5, 0.5, 1.0);
    gui->pressedColor = vec4(0.4, 0.2, 0.2, 1.0);

    gui->worldRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, gui->worldRenderGroup, 1024);
    gui->screenRenderGroup = PushStruct(arena, RenderGroup);
    InitRenderGroup(arena, gui->screenRenderGroup, 1024);

    gui->labelBufferSize = 0;
    gui->maxLabelBufferSize = 4096;
    gui->labelBuffer = PushAndZeroArray(arena, char, gui->maxLabelBufferSize);

    gui->nLabels = 0;
    gui->maxLabels = 256;
    gui->labels = PushAndZeroArray(arena, GuiLabel, gui->maxLabels);
}

