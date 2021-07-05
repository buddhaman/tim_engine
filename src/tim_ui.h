
typedef struct
{
    char *text;
    Vec2 pos;
    Vec4 color;
} GuiLabel;

typedef struct
{
    AppState *appState;
    // TODO: Clean all this. This is messy.
    Camera2D *camera;
    Assets *assets;
    RenderGroup *screenRenderGroup;
    RenderGroup *worldRenderGroup;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;

    b32 isRadialMenuActive;
    Vec2 radialMenuPos;
    r32 radialTimer;
    
    char *labelBuffer;
    size_t labelBufferSize;
    size_t maxLabelBufferSize;

    ui32 nLabels;
    ui32 maxLabels;
    GuiLabel *labels;
} Gui;

