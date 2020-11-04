#include "external_headers.h"
#include <time.h>

#define DebugOut(args...) printf(args); printf("\t\t %s:%d\n", __FILE__, __LINE__)
#define Assert(expr) if(!(expr)) {DebugOut("assert failed "#expr""); \
    *((int *)0)=0;}

#define local_persist static
#define internal static
#define global_variable static

typedef unsigned char ui8;
typedef unsigned short ui16;
typedef unsigned int ui32;
typedef unsigned long ui64;
typedef char i8;
typedef short i16;
typedef int i32;
typedef long i64;

typedef float r32;
typedef double r64;

typedef i32 b32;

#define MAX_VERTEX_MEMORY 512*1024
#define MAX_ELEMENT_MEMORY 128*1024

// Own files
#include "cool_memory.h"
#include "app_state.h"
#include "renderer.h"
#include "world.h"
#include "world_renderer.h"
#include "guy.h"

#include "cool_memory.c"
#include "app_state.c"
#include "renderer.c"
#include "world.c"
#include "world_renderer.c"
#include "guy.c"

// shaders
#include "shaderVert.h"
#include "shaderFrag.h"

// stb
#define STB_DS_IMPLEMENTATION
#include "stb_ds.h"

#define FRAMES_PER_SECOND 60

const char *
ReadEntireFile(char *path)
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

ui32
CreateAndCompileShaderSource(const char * const*source, GLenum shaderType)
{
    ui32 shader = glCreateShader(shaderType);
    glShaderSource(shader, 1, source, NULL);
    glCompileShader(shader);
    i32 succes;
    char infolog[512];
    glGetShaderiv(shader, GL_COMPILE_STATUS, &succes);
    if(!succes)
    {
        glGetShaderInfoLog(shader, 512, NULL, infolog);
        DebugOut("%s", infolog);
    }
    else
    {
        DebugOut("Shader %u compiled succesfully.", shader);
    }
    return shader;
}

ui32 
CreateAndLinkShaderProgram(ui32 vertexShader, ui32 fragmentShader)
{
    ui32 shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    i32 succes;
    char infolog[512];
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &succes);
    if(!succes)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infolog);
        DebugOut("%s", infolog);
    }
    else
    {
        DebugOut("Shader program %u linked succesfully.", shaderProgram);
    }
    return shaderProgram;
}

int 
main(int argc, char**argv)
{
    if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER)!=0)
    {
        DebugOut("Does not work\n");
    }

    srand(time(0));

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

    SDL_WindowFlags window_flags = 
        (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
    i32 screen_width = 1280;
    i32 screen_height = 720;

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

    // Setup shaders
    const char *const solidFragSource = (const char *const)shaders_solid_frag;
    const char *const solidVertSource = (const char *const)shaders_solid_vert;
    
    ui32 fragmentShader = CreateAndCompileShaderSource(&solidFragSource, GL_FRAGMENT_SHADER);
    ui32 vertexShader = CreateAndCompileShaderSource(&solidVertSource, GL_VERTEX_SHADER);
    ui32 simpleShader = CreateAndLinkShaderProgram(fragmentShader, vertexShader);

    ui32 transformLocation = glGetUniformLocation(simpleShader, "transform");
    ui32 lightDirLocation = glGetUniformLocation(simpleShader, "lightDir");

    // Setup nuklear
    struct nk_context *ctx;
    ctx = nk_sdl_init(window);
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();

    // Creating appstate
    AppState *appState = (AppState *)malloc(sizeof(AppState));
    *appState = (AppState){};

    r32 time = 0.0;
    r32 deltaTime = 0.0;
    r32 timeSinceLastFrame = 0.0;
    r32 updateTime = 1.0/FRAMES_PER_SECOND;
    ui32 frameStart = SDL_GetTicks(); 
    // Timing
    b32 done = 0;
    ui32 frameCounter = 0;

    // Array test
    int *array = NULL;
    arrput(array, 2);
    arrput(array, 3);
    for(int i = 0; i < arrlen(array); i++)
    {
        DebugOut("%d", array[i]);
    }
    // Hashmap test
    struct {float key; char value; } *hash = NULL;
    hmput(hash, 10.5, 'h');
    hmput(hash, 11.5, 'e');
    hmput(hash, 12.5, 'l');
    hmput(hash, 22.5, 'l');
    hmput(hash, 22.2, 'o');

    DebugOut("%c - ", hmget(hash, 11.5));

    // Camera
    Camera camera;
    InitCamera(&camera);
    camera.lookAt = vec3(10,10,0);

    MemoryArena *renderArena = CreateMemoryArena(128*1000*1000);

    Model *groundModel = PushStruct(renderArena, Model);
    Mesh *groundMesh = CreateMesh(renderArena, 10000);
    InitModel(renderArena, groundModel,10000);

    Mesh *dynamicMesh = CreateMesh(renderArena, 100000);
    Model *dynamicModel = PushStruct(renderArena, Model);
    InitModel(renderArena, dynamicModel, 100000);

    DebugOut("render arena : %lu / %lu bytes used. %lu procent", 
            renderArena->used, renderArena->size, (renderArena->used*100)/renderArena->size);
    ClearMesh(groundMesh);

    PushHeightField(groundMesh, 1, 20, 20);

    World *world = PushStruct(renderArena, World);
    InitWorld(renderArena, world, 1000, 1000, 100);

    AddGuy(world, vec3(0,0,0));

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

        // Build nice environment
        SetModelFromMesh(groundModel, groundMesh, GL_STREAM_DRAW);

        // Clear screen
        Vec3 clearColor = ARGBToVec3(0xffe0fffe);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, appState->screenWidth, appState->screenHeight);

        // My rendering
        Vec3 lightDir = v3_norm(vec3(-1,1,-1));
        
        // Update Camera
        glUseProgram(simpleShader);
        r32 camSpeed = 0.2;
        r32 zoomSpeed = 0.99;
        if(IsKeyActionDown(appState, ACTION_Z)) { camera.spherical.z*=zoomSpeed; }
        if(IsKeyActionDown(appState, ACTION_X)) { camera.spherical.z/=zoomSpeed; }
        if(IsKeyActionDown(appState, ACTION_UP)) { camera.lookAt.y+=camSpeed; }
        if(IsKeyActionDown(appState, ACTION_DOWN)) { camera.lookAt.y-=camSpeed; }
        if(IsKeyActionDown(appState, ACTION_LEFT)) { camera.lookAt.x-=camSpeed; }
        if(IsKeyActionDown(appState, ACTION_RIGHT)) { camera.lookAt.x+=camSpeed; }
        if(IsKeyActionDown(appState, ACTION_Q))
        {
            camera.spherical.y-=0.1;
            if(camera.spherical.y < 0.1) camera.spherical.y = 0.1;
        }
        if(IsKeyActionDown(appState, ACTION_E))
        {
            camera.spherical.y+=0.1;
            if(camera.spherical.y > M_PI/2-0.1) camera.spherical.y = M_PI/2-0.1;
        }
        if(IsKeyActionDown(appState, ACTION_R))
        {
        }
        // Update loop pos
        UpdateCamera(&camera, appState->screenWidth, appState->screenHeight);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Set uniforms
        glUniformMatrix4fv(transformLocation, 1, GL_FALSE, (GLfloat*)&camera.transform);
        glUniform3fv(lightDirLocation, 1, (GLfloat*)&lightDir);

        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);
        RenderModel(groundModel);

        // Update entire world physics step
        DoPhysicsStep(world);

        // Guys
        UpdateGuys(world);
        DrawGuys(dynamicMesh, world);

        // Render dynamic model
        SetModelFromMesh(dynamicModel, dynamicMesh, GL_DYNAMIC_DRAW);
        glDisable(GL_CULL_FACE);
        RenderModel(dynamicModel);
        ClearMesh(dynamicMesh);

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

