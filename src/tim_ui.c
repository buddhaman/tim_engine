
internal inline void
GuiPushCircle(Gui *gui, Vec2 pos, r32 radius, Vec4 color)
{
    SpriteBatch *batch = gui->renderContext->batch;
    batch->colorState = color;
    PushCircle2(batch, pos, radius, gui->renderContext->defaultAtlas->regions);
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
    return hit & IsKeyActionJustDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
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

void
InitGui(Gui *gui, 
        AppState *appState, 
        Camera2D *camera,
        RenderContext *renderContext)
{
    gui->appState = appState;
    gui->camera = camera;
    gui->renderContext = renderContext;
    gui->defaultColor = vec4(0.6, 0.3, 0.3, 1.0);
    gui->hitColor = vec4(0.8, 0.5, 0.5, 1.0);
    gui->pressedColor = vec4(0.4, 0.2, 0.2, 1.0);
}

