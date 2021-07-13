
#define MAX_CONTEXT_STACK_SIZE 8

typedef ui32 GuiId;

typedef struct
{
    GuiId id;
    r32 t;          // Between [0, 1].
    r32 updateStep;
    b32 isActive;
} GuiAnimation;     

#define MAX_GUI_ANIMATIONS 256

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
    GuiId mouseEnteredWidget;

    ui32 contextStackDepth;
    GuiId contextStack[MAX_CONTEXT_STACK_SIZE];
    ui32 nWidgetsInCurrentContext;

    Vec4 defaultColor;
    Vec4 hitColor;
    Vec4 pressedColor;

    b32 isRadialMenuActive;
    Vec2 radialMenuPos;
    r32 radialTimer;

    ui32 nAnimations;
    GuiAnimation animations[MAX_GUI_ANIMATIONS];
} Gui;


