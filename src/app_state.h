typedef enum 
{
    ACTION_UNKNOWN,
    ACTION_MOUSE_BUTTON_LEFT,
    ACTION_MOUSE_BUTTON_RIGHT,
    ACTION_RIGHT,
    ACTION_UP,
    ACTION_LEFT,
    ACTION_DOWN,
    ACTION_Z,
    ACTION_X,
    ACTION_Q,
    ACTION_E,
    ACTION_R,
    ACTION_P, 
    ACTION_ESCAPE,
    ACTION_DELETE,
    NUM_KEY_ACTIONS
} KeyAction;

typedef enum
{
    SCREEN_CREATURE_EDITOR,
    SCREEN_FAKE_WORLD
} ScreenType;

#include "fake_world_screen.h"

typedef struct AppState AppState;
struct AppState
{
    MemoryArena *fakeWorldArena;
    FakeWorldScreen *fakeWorldScreen;
    i32 screenWidth;
    i32 screenHeight;
    r32 ratio;
    i32 mx;
    i32 my;
    i32 dx;
    i32 dy;
    i32 mouseScrollY;
    r32 normalizedMX;
    r32 normalizedMY;
    Vec4 clearColor;
    ScreenType currentScreen;
    Camera2D *screenCamera;

    b32 isActionDown[NUM_KEY_ACTIONS];
    b32 wasActionDown[NUM_KEY_ACTIONS];
};

void 
InitFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        MemoryArena *arena, 
        CreatureDefinition *def,
        ui32 nGenes,
        r32 dev,
        r32 learningRate);

void
StartFakeWorld(AppState *appState, CreatureDefinition *definition, ui32 nGenes, r32 dev, r32 learningRate);

