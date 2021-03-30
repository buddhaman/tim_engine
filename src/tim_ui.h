
typedef struct
{
    AppState *appState;
    Camera2D *camera;
    RenderContext *renderContext;

    b32 hasFocus;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;
} Gui;

