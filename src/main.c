#include "external_headers.h"
#include <time.h>

#define DebugOut(args...) printf("%20s%5d: ", __FILE__, __LINE__); printf(args); printf("\n");
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

// Special file
#include "math_2d.h"

// User defined typedefs
typedef vec2_t Vec2;
typedef vec3_t Vec3;
typedef vec4_t Vec4;
typedef mat4_t Mat4;
typedef mat3_t Mat3;

typedef float r32;
typedef double r64;

typedef i32 b32;

#define MAX_VERTEX_MEMORY 512*1024
#define MAX_ELEMENT_MEMORY 128*1024

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
#include "app_state.h"
#include "texture_atlas.h"
#include "renderer.h"
#include "spritebatch.h"
#include "world.h"
#include "world_renderer.h"
#include "guy.h"

#include "cool_memory.c"
#include "app_state.c"
#include "texture_atlas.c"
#include "renderer.c"
#include "spritebatch.c"
#include "world.c"
#include "world_renderer.c"
#include "guy.c"

// shaders
#include "shaderVert.h"
#include "shaderFrag.h"

#define FRAMES_PER_SECOND 60

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

    FontRenderer fontRenderer;
    InitFontRenderer(&fontRenderer, "cool.ttf");

    Vec2 v2 = vec2(1,0);

    Mat3 mat = m3_translation(vec2(2,3));
    Vec2 v3 = m3_mul_pos(mat, v2);

    m3_fprintp(stdout, mat, 2, 2);
    DebugOut("v3 = %f, %f", v3.x, v3.y);

    // Load bitmaps
    int width, height, n;
    unsigned char *image = stbi_load("l.png", &width, &height, &n, 4);
    DebugOut("Image loaded with %d components per pixel, (%dx%d)", n, width, height);

    ui32 texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Setup shaders
    Shader simpleShader;
    InitShader(&simpleShader, "shaders/texture.vert", "shaders/texture.frag");
    LoadShader(&simpleShader);
    
    ui32 transformLocation = glGetUniformLocation(simpleShader.program, "transform");
    ui32 lightDirLocation = glGetUniformLocation(simpleShader.program, "lightDir");

    Shader spriteShader;
    InitShader(&spriteShader, "shaders/sprite.vert", "shaders/sprite.frag");
    LoadShader(&spriteShader);
    ui32 spriteTransformLocation = glGetUniformLocation(spriteShader.program, "transform");

    // Setup nuklear
    struct nk_context *ctx;
    ctx = nk_sdl_init(window);
    // TODO: Check if this is necessary for nuklear
#if 0
    struct nk_font_atlas *atlas;
    nk_sdl_font_stash_begin(&atlas);
    nk_sdl_font_stash_end();
#endif

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
    hmput(hash, 23.2, 'o');

    DebugOut("%c - ", hmget(hash, 11.5));

    // Camera
    Camera camera;
    InitCamera(&camera);
    camera.lookAt = vec3(10,10,0);

    MemoryArena *renderArena = CreateMemoryArena(128*1000*1000);

    TextureAtlas *atlas = MakeDefaultTexture(renderArena, 512);
    AtlasRegion *circleRegion = atlas->regions;
    AtlasRegion *squareRegion = atlas->regions+1;

    ui32 vertexAttributes = ATTR_POS3 | ATTR_COL3 | ATTR_TEX | ATTR_NORM3;
    Model *groundModel = PushStruct(renderArena, Model);
    Mesh *groundMesh = CreateMesh(renderArena, vertexAttributes, 100000);
    InitModel(renderArena, groundModel, vertexAttributes, 100000);

    Mesh *dynamicMesh = CreateMesh(renderArena, vertexAttributes, 100000);
    Model *dynamicModel = PushStruct(renderArena, Model);
    InitModel(renderArena, dynamicModel, vertexAttributes, 100000);

    ui32 spriteAttributes = ATTR_POS2 | ATTR_TEX;
    int spriteBatchSize = 10000;
    Mesh *spriteMesh = CreateMesh(renderArena, spriteAttributes, spriteBatchSize);
    Model *spriteModel = PushStruct(renderArena, Model);
    InitModel(renderArena, spriteModel, spriteAttributes, spriteBatchSize);

    DebugOut("render arena : %lu / %lu bytes used. %lu procent", 
            renderArena->used, renderArena->size, (renderArena->used*100)/renderArena->size);
    ClearMesh(groundMesh);

    PushHeightField(groundMesh, 0.31, 100, 100, squareRegion->pos, squareRegion->size);

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
        if(IsKeyActionJustDown(appState, ACTION_R))
        {
            UnloadShader(&simpleShader);
            LoadShader(&simpleShader);
            UnloadShader(&spriteShader);
            LoadShader(&spriteShader);
        }
        glUseProgram(simpleShader.program);
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

        glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        PushQuad(dynamicMesh, vec3(0,0,1), vec3(1,0,1), vec3(1,1,1), vec3(0,1,1),
                vec2(0,0), vec2(1,1));

        // Guys
        UpdateGuys(world);
        DrawGuys(dynamicMesh, world, squareRegion->pos, squareRegion->size, circleRegion->pos,
                circleRegion->size);

        SetModelFromMesh(dynamicModel, dynamicMesh, GL_DYNAMIC_DRAW);
        glDisable(GL_CULL_FACE);
        RenderModel(dynamicModel);
        ClearMesh(dynamicMesh);

        // Render spritebatch
        glDisable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        glBindTexture(GL_TEXTURE_2D, fontRenderer.font12Texture);
        Mat3 t3 = m3_translation_and_scale(vec2(-appState->screenWidth/2, -appState->screenHeight/2),
                2.0/appState->screenWidth, -2.0/appState->screenHeight);

        glUseProgram(spriteShader.program);
        glUniformMatrix3fv(spriteTransformLocation, 1, GL_FALSE, (GLfloat*)&t3);

        ClearMesh(spriteMesh);
        char str[256];
        strcpy(str, "Welcome 2 tims engine R u Happy with d Resultt?!! :DDDDDD");
        int l = strlen(str);
        local_persist ui8 counter = 0;
        if(RandomFloat(0, 1) < 0.1) counter++;
        if(counter >= l) counter = 0;
        str[counter] = 0;
        DrawString2D(spriteMesh, &fontRenderer, vec2(100, 100), str);
        //DrawString2D(spriteMesh, &fontRenderer, vec2(-xo, -yo), "What is this game?");
        SetModelFromMesh(spriteModel, spriteMesh, GL_DYNAMIC_DRAW);
        RenderModel(spriteModel);

        // Draw rest of spritebatch
        ClearMesh(spriteMesh);
        PushRect2(spriteMesh, vec2(200, 209), 
                vec2(200+sinf(time)*100, 200+sinf(time)*100), vec2(0,0), vec2(1, 1));
        PushRect2(spriteMesh, vec2(400, 209), 
                vec2(200+sinf(time)*100, 200+sinf(time)*100), vec2(0,0), vec2(1, 1));
        SetModelFromMesh(spriteModel, spriteMesh, GL_DYNAMIC_DRAW);
        glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
        RenderModel(spriteModel);

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

