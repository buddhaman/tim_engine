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
    I32 screenWidth;
    I32 screenHeight;
    R32 ratio;
    I32 mx;
    I32 my;
    I32 dx;
    I32 dy;
    I32 mousePressedAtX;
    I32 mousePressedAtY;
    I32 mouseScrollY;
    R32 normalizedMX;
    R32 normalizedMY;
    Vec4 clearColor;
    ScreenType currentScreen;
    Camera2D *screenCamera;

    B32 isActionDown[NUM_KEY_ACTIONS];
    B32 wasActionDown[NUM_KEY_ACTIONS];

    char textInput[128];
};

void
StartFakeWorld(AppState *appState, 
        CreatureDefinition *definition, 
        Assets *assets, 
        U32 nGenes, 
        R32 dev, 
        R32 learningRate);

