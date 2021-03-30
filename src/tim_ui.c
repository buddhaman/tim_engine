
internal inline void
GuiPushCircle(Gui *gui, Vec2 pos, r32 radius, Vec4 color)
{
    SpriteBatch *batch = gui->renderContext->batch;
    batch->colorState = color;
    PushCircle2(batch, pos, radius, gui->renderContext->defaultAtlas->regions);
}

b32
DoDragButtonRotation(Gui *gui, 
        Vec2 *pos, 
        r32 radius, 
        Vec2 center, 
        r32 angle, 
        r32 length,
        b32 isDragging)
{
    return 1;
}

b32
DoDragButtonAlongAxis(Gui *gui, Vec2 *pos, r32 radius, Vec2 axisOffset, Vec2 axis, b32 isDragging)
{
    Vec4 color = gui->defaultColor;
    Vec2 mousePos = gui->camera->mousePos;
    b32 hit = CirclePointIntersect(*pos, radius, mousePos);
    if(isDragging)
    {
        r32 dotPos = v2_dot(axis, mousePos);
        r32 dotOffset = v2_dot(axis, axisOffset);
        r32 diff = Max(1, dotPos-dotOffset);
        *pos = v2_add(axisOffset, v2_muls(axis, diff));
        color = gui->pressedColor;
        GuiPushCircle(gui, *pos, radius, color);
        return IsKeyActionDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
    }
    else
    {
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
        GuiPushCircle(gui, *pos, radius, color);
        return hit && IsKeyActionJustDown(gui->appState, ACTION_MOUSE_BUTTON_LEFT);
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
