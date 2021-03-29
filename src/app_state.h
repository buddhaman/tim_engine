typedef enum 
{
    ACTION_UNKNOWN,
    ACTION_MOUSE_BUTTON_LEFT,
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
    NUM_KEY_ACTIONS
} KeyAction;

typedef enum
{
    SCREEN_CREATURE_EDITOR,
    SCREEN_FAKE_WORLD
} ScreenType;

typedef struct AppState AppState;
struct AppState
{
    i32 screenWidth;
    i32 screenHeight;
    r32 ratio;
    i32 mx;
    i32 my;
    r32 normalizedMX;
    r32 normalizedMY;
    Vec4 clearColor;
    ScreenType currentScreen;
    Camera2D *screenCamera;

    b32 isActionDown[NUM_KEY_ACTIONS];
    b32 wasActionDown[NUM_KEY_ACTIONS];
};
