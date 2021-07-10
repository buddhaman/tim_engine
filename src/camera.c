
void 
InitCamera2D(Camera2D *camera)
{
    camera->pos = vec2(0,0);
    camera->scale = 1.0;
}

void
UpdateCamera2D(Camera2D *camera, AppState *appState)
{
    Vec2 size = vec2(camera->scale*appState->screenWidth, camera->scale*appState->screenHeight);
    camera->transform = m3_translation_and_scale(
            vec2(-camera->pos.x, -camera->pos.y),
            2.0/size.x, (-1.0+2*camera->isYUp)*2.0/size.y);
    camera->size = size;

    r32 ny = ((r32)appState->my)/((r32)appState->screenHeight) - 0.5;// In [-0.5, 0.5]
    // Mouse location.
    camera->mousePos = vec2(
            appState->mx*camera->scale+camera->pos.x-camera->size.x/2.0, 
            (1.0-2.0*camera->isYUp)*camera->size.y*ny + camera->pos.y);
}

void
FitCamera2DToScreen(Camera2D *camera, AppState *appState)
{
    camera->scale = 1;
    camera->pos = vec2(appState->screenWidth/2.0, appState->screenHeight/2.0);
}

internal inline Vec2
CameraToScreenPos(Camera2D *camera, AppState *appState, Vec2 pos)
{
    r32 dx = pos.x-camera->pos.x;
    r32 dy = pos.y-camera->pos.y;

    dx = dx/camera->size.x+0.5;
    dy = (1-2*camera->isYUp)*dy/camera->size.y+0.5;
    return vec2(appState->screenWidth*dx, appState->screenHeight*dy);
}

void
UpdateCameraKeyMovementInput(AppState *appState, Camera2D *camera)
{
    r32 zoomSpeed = 0.98;
    r32 camSpeed = 8.0 * camera->scale;
    if(IsKeyActionDown(appState, ACTION_Z)) { camera->scale*=zoomSpeed; }
    if(IsKeyActionDown(appState, ACTION_X)) { camera->scale/=zoomSpeed; }
    if(IsKeyActionDown(appState, ACTION_UP)) { camera->pos.y+=camSpeed; }
    if(IsKeyActionDown(appState, ACTION_DOWN)) { camera->pos.y-=camSpeed; }
    if(IsKeyActionDown(appState, ACTION_LEFT)) { camera->pos.x-=camSpeed; }
    if(IsKeyActionDown(appState, ACTION_RIGHT)) { camera->pos.x+=camSpeed; }
}

void
UpdateCameraScrollInput(AppState *appState, Camera2D *camera)
{
    if(appState->mouseScrollY)
    {
        r32 zoomFactor = pow(0.9, appState->mouseScrollY);
        camera->scale*=zoomFactor;
    }
}

void
UpdateCameraDragInput(AppState *appState, Camera2D *camera)
{
    if(camera->isDragging)
    {
        camera->pos.x-=appState->dx*camera->scale;
        camera->pos.y+=appState->dy*camera->scale;
    }
    else if(IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT))
    {
        camera->isDragging = 1;
    }
}

void
CameraStopDragging(Camera2D *camera)
{
    camera->isDragging = 0;
}

