
typedef ui32 GuiId;

typedef struct
{
    AppState *appState;
    BasicRenderTools *renderTools;

    b32 isMouseDown;
    b32 isMouseJustReleased;

    GuiId active;
    GuiId prevActive;
    GuiId hot;
    GuiId prevHot;
    GuiId nWidgetsInCurrentContext;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;

    b32 isRadialMenuActive;
    Vec2 radialMenuPos;
    r32 radialTimer;
} Gui;


