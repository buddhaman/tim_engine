#include "external_headers.h"
#include <time.h>

// Special file.
#include "tim_types.h"
#include "math_2d.h"

// My own math typedefs.
typedef vec2_t Vec2;
typedef vec3_t Vec3;
typedef vec4_t Vec4;
typedef mat4_t Mat4;
typedef mat3_t Mat3;

#define MAX_VERTEX_MEMORY 512*1024
#define MAX_ELEMENT_MEMORY 128*1024

#define CheckOpenglError() { GLenum err = glGetError(); \
    if(err) { DebugOut("err = %04x", err);Assert(0); }}

char *
ReadEntireFile(const char *path)
{
    char *buffer = NULL;
    size_t stringSize, readSize;
    FILE *handler = fopen(path, "r");
    if(handler)
    {
        fseek(handler, 0, SEEK_END);
        stringSize = ftell(handler);
        rewind(handler);
        buffer = (char *)malloc(sizeof(char) * (stringSize + 1));
        readSize = fread(buffer, sizeof(char), stringSize, handler);
        buffer[stringSize] = 0;

        if(stringSize!=readSize)
        {
            free(buffer);
            buffer = NULL;
        }
        fclose(handler);
        return buffer;
    }
    else
    {
        DebugOut("Can't read file %s", path);
        return NULL;
    }
}

// Own files
#include "cool_memory.h"
#include "tim_math.h"
#include "linalg.h"
#include "shader.h"
#include "neural_net.h"
#include "render2d.h"
#include "texture_atlas.h"
#include "render_context.h"
#include "creature_definition.h"
#include "evolution_strategies.h"
#include "world.h"
#include "app_state.h"
#include "tim_ui.h"
#include "fake_world_screen.h"
#include "creature_editor.h"

#include "cool_memory.c"
#include "tim_math.c"
#include "linalg.c"
#include "shader.c"
#include "neural_net.c"
#include "render2d.c"
#include "texture_atlas.c"
#include "render_context.c"
#include "creature_definition.c"
#include "evolution_strategies.c"
#include "world.c"
#include "creature.c"
#include "app_state.c"
#include "tim_ui.c"
#include "fake_world_screen.c"
#include "creature_editor.c"

#define MEM_TEST 0
#define NEURALNET_TEST 0

#if MEM_TEST==1
#include "mem_test.cpp"
#endif
#if NEURALNET_TEST==1
#include "neuralnet_test.c"
#endif

void
DoTests()
{
#if MEM_TEST==1
    DoMemoryTests();
#endif
#if NEURALNET_TEST==1
    DoNeuralNetTests();
#endif
}

#define FRAMES_PER_SECOND 60

int 
main(int argc, char**argv)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)!=0)
    {
        DebugOut("SDL does not work\n");
    }

    srand(time(0));
    DoTests();

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
    SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);

    SDL_WindowFlags window_flags = 
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    int screen_width = 1280;
    int screen_height = 720;

    SDL_Window *window = SDL_CreateWindow("Cool", SDL_WINDOWPOS_CENTERED, 
            SDL_WINDOWPOS_CENTERED, screen_width, screen_height, window_flags);

    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    SDL_GL_MakeCurrent(window, gl_context);


    SDL_GL_SetSwapInterval(1);
    
    b32 err = gl3wInit() != 0;
    if(err)
    {
        DebugOut("Failed to initialize OpenGl Loader!\n");
    }
#if 0
    // Load bitmaps
    int width, height, n;
    unsigned char *image = stbi_load("l.png", &width, &height, &n, 4);
    DebugOut("Image loaded with %d components per pixel, (%dx%d)", n, width, height);
#endif

    // Setup nuklear
    struct nk_context *ctx;
    ctx = nk_sdl_init(window);

    struct nk_font_atlas *nkFontAtlas;
    nk_sdl_font_stash_begin(&nkFontAtlas);
    struct nk_font *font = nk_font_atlas_add_from_file(nkFontAtlas, "DejaVuSansMono.ttf", 16.0, 0);
    nk_sdl_font_stash_end();
    nk_style_set_font(ctx, &font->handle);

    // MemoryArena
    MemoryArena *gameArena = CreateMemoryArena(128*1000*1000);
    // Creating appstate
    AppState *appState = (AppState *)malloc(sizeof(AppState));
    *appState = (AppState){};

    appState->fakeWorldArena = CreateMemoryArena(128L*1000L*1000L);
    appState->clearColor = RGBAToVec4(0x35637cff);
    appState->currentScreen = SCREEN_CREATURE_EDITOR;
    // Font/Gui Camera
    appState->screenCamera = PushStruct(gameArena, Camera2D);
    InitCamera2D(appState->screenCamera);
    appState->screenCamera->isYUp = 0;

    r32 time = 0.0;
    r32 deltaTime = 0.0;
    r32 timeSinceLastFrame = 0.0;
    r32 updateTime = 1.0/FRAMES_PER_SECOND;
    ui32 frameStart = SDL_GetTicks(); 
    // Timing
    b32 done = 0;
    ui32 frameCounter = 0;

    RenderContext *renderContext = PushStruct(gameArena, RenderContext);
    InitRenderContext(renderContext, gameArena);

    CreatureEditorScreen *creatureEditorScreen = PushStruct(gameArena, CreatureEditorScreen);
    InitCreatureEditorScreen(appState, creatureEditorScreen, renderContext, gameArena);

    // Anti aliasing in opengl.
    glEnable(GL_MULTISAMPLE);

    // Handle evolution
    while(!done)
    {
        SDL_Event event;
        nk_input_begin(ctx);
        ResetKeyActions(appState);
        while(SDL_PollEvent(&event))
        {
            nk_sdl_handle_event(&event);
            switch(event.type)
            {

            case SDL_MOUSEBUTTONUP:
            {
                RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_LEFT, 0);
            } break;

            case SDL_MOUSEBUTTONDOWN:
            {
                RegisterKeyAction(appState, ACTION_MOUSE_BUTTON_LEFT, 1);
            } break;

            case SDL_KEYUP:
            {
                RegisterKeyAction(appState, MapKeyCodeToAction(event.key.keysym.sym), 0);
            } break;

            case SDL_KEYDOWN:
            {
                RegisterKeyAction(appState, MapKeyCodeToAction(event.key.keysym.sym), 1);
            } break;

            case SDL_QUIT:
            {
                done = 1;
            } break;

            default:
            {
                if(event.type == SDL_WINDOWEVENT
                        && event.window.event == SDL_WINDOWEVENT_CLOSE
                        && event.window.windowID == SDL_GetWindowID(window))
                {
                    done = 1;
                }
            } break;

            }
        }
        nk_input_end(ctx);
        // Set Appstate

        SDL_GetWindowSize(window, &appState->screenWidth, &appState->screenHeight);
        appState->ratio = (r32)appState->screenHeight / ((r32)appState->screenWidth);
        SDL_GetMouseState(&appState->mx, &appState->my);
        appState->normalizedMX = 2*((r32)appState->mx)/appState->screenWidth - 1.0;
        appState->normalizedMY = -2*((r32)appState->my)/appState->screenHeight + 1.0;

        FitCamera2DToScreen(appState->screenCamera, appState);
        UpdateCamera2D(appState->screenCamera, appState);

        // Clear screen
        Vec4 clearColor = appState->clearColor;
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, appState->screenWidth, appState->screenHeight);

        switch(appState->currentScreen)
        {

        case SCREEN_FAKE_WORLD:
        {
            UpdateFakeWorldScreen(appState, appState->fakeWorldScreen, renderContext, ctx);
        } break;

        case SCREEN_CREATURE_EDITOR:
        {
            UpdateCreatureEditorScreen(appState, creatureEditorScreen, renderContext, ctx);
        } break;

        }

        // Render UI
        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);

        // frame timing
        ui32 frameEnd = SDL_GetTicks();
#if 0
        if(frameEnd > (lastSecond+1)*1000)
        {
            // new second
            lastSecond = frameEnd/1000;
            DebugOut("Frames per second = %d\n", frameCounter);
            frameCounter = 0;
        }
#endif
        ui32 frameTicks = frameEnd-frameStart;
        frameStart=frameEnd;
        deltaTime=((r32)frameTicks)/1000.0;
        time+=deltaTime;
        timeSinceLastFrame+=deltaTime;
        if(timeSinceLastFrame < updateTime)
        {
            i32 waitForTicks = (i32)((updateTime-timeSinceLastFrame)*1000);
            if(waitForTicks > 0)
            {
                //DebugOut("this happened, waitForTicks = %d\n", waitForTicks);
                if(waitForTicks < 2*updateTime*1000)
                {
                    SDL_Delay(waitForTicks);
                    timeSinceLastFrame-=waitForTicks/1000.0;
                }
                else
                {
                    timeSinceLastFrame = 0;
                }
            }
        }
        //DebugOut("time = %f, deltaTime = %f, ticks = %d\n", time, deltaTime, frameTicks);
        SDL_GL_SwapWindow(window);
        frameCounter++;
    }
    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

