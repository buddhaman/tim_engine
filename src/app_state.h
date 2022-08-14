typedef enum 
{
    ACTION_UNKNOWN = 0,
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

typedef struct AppState AppState;

#include "fake_world_screen.h"

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

    char textInput[128];
};

void
StartFakeWorld(AppState *appState, 
        CreatureDefinition *definition, 
        Assets *assets, 
        ui32 nGenes, 
        r32 dev, 
        r32 learningRate);

