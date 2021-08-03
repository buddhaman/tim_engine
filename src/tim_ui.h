
#define MAX_CONTEXT_STACK_SIZE 8

// Must be power of two
#define MAX_GUI_ANIMATIONS 256 

typedef ui32 GuiId;

typedef struct
{
    b32 isActive;
    GuiId widgetId;

    r32 timeFactor;
    r32 timeStep;
} GuiAnimation;

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
    GuiId mouseEnteredWidgetId;

    ui32 contextStackDepth;
    GuiId contextStack[MAX_CONTEXT_STACK_SIZE];
    ui32 nWidgetsInCurrentContext;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;

    b32 isRadialMenuActive;
    Vec2 radialMenuPos;
    r32 radialTimer;

    GuiAnimation animations[MAX_GUI_ANIMATIONS];
} Gui;


