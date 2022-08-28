
#define MAX_CONTEXT_STACK_SIZE 8

// Must be power of two
#define MAX_GUI_ANIMATIONS 256 

typedef U32 GuiId;

typedef struct
{
    B32 isActive;
    GuiId widgetId;

    R32 timeFactor;
    R32 timeStep;
} GuiAnimation;

typedef struct
{
    AppState *appState;
    BasicRenderTools *renderTools;

    B32 isMouseDown;
    B32 isMouseJustReleased;

    GuiId active;
    GuiId prevActive;
    GuiId hot;
    GuiId prevHot;
    GuiId mouseEnteredWidgetId;

    U32 contextStackDepth;
    GuiId contextStack[MAX_CONTEXT_STACK_SIZE];
    U32 nWidgetsInCurrentContext;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;

    B32 isRadialMenuActive;
    Vec2 radialMenuPos;
    R32 radialTimer;

    char textInput[128];
    GuiAnimation animations[MAX_GUI_ANIMATIONS];
} Gui;


