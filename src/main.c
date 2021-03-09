#include "external_headers.h"
#include <time.h>

#define DebugOut(args...) printf("%20s%5d: ", __FILE__, __LINE__); printf(args); printf("\n");
#define Assert(expr) if(!(expr)) {DebugOut("assert failed "#expr""); \
    *((int *)0)=0;}


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

#define Min(a, b) (a) < (b) ? (a) : (b)
#define Max(a, b) (a) < (b) ? (b) : (a)

#define CheckOpenglError() { GLenum err = glGetError(); \
    if(err) { DebugOut("err =%04x", err);Assert(0); }}

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
#include "app_state.h"
#include "render2d.h"
#include "texture_atlas.h"

#include "cool_memory.c"
#include "tim_math.c"
#include "linalg.c"
#include "shader.c"
#include "neural_net.c"
#include "app_state.c"
#include "render2d.c"
#include "texture_atlas.c"

#define MEM_TEST 0

#if MEM_TEST==1
#include "mem_test.cpp"
#endif

void
DoTests()
{
#if MEM_TEST==1
    DoMemoryTests();
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
    InitFontRenderer(&fontRenderer, "DejaVuSansMono.ttf");

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

#if 0
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
#endif

    MemoryArena *gameArena = CreateMemoryArena(128*1000*1000);

    DebugOut("game arena : %lu / %lu bytes used. %lu procent", 
            gameArena->used, gameArena->size, (gameArena->used*100)/gameArena->size);

    // Init spritebatch
    SpriteBatch *batch = PushStruct(gameArena, SpriteBatch);
    InitSpriteBatch(batch, 10000, gameArena);

    // Init simple textureatlas
    TextureAtlas *atlas = MakeDefaultTexture(gameArena, 256);

    // Make spritebatch shader
    Shader *spriteShader = PushStruct(gameArena, Shader);
    InitShader(spriteShader, "shaders/sprite.vert", "shaders/sprite.frag");
    LoadShader(spriteShader);

    // + Camera
    Camera2D *camera = PushStruct(gameArena, Camera2D);
    InitCamera2D(camera);
    camera->isYDown = 1;

    // Font/Gui Camera
    Camera2D *screenCamera = PushStruct(gameArena, Camera2D);
    InitCamera2D(screenCamera);
    screenCamera->isYDown = 0;

    // Chipmunk hello world
    cpVect gravity = cpv(0, -800);

    cpSpace *space = cpSpaceNew();
    cpSpaceSetGravity(space, gravity);

    cpVect groundFrom = cpv(-100, 20);
    cpVect groundTo = cpv(100, 0);

    cpShape *ground = cpSegmentShapeNew(cpSpaceGetStaticBody(space), 
            groundFrom,
            groundTo,
            0);
    cpShapeSetFriction(ground, 1);
    cpSpaceAddShape(space, ground);

    cpFloat radius = 10;
    cpFloat mass = 1;
    cpFloat moment = cpMomentForCircle(mass, 0, radius, cpvzero);
    
    cpBody *ballBody = cpSpaceAddBody(space, cpBodyNew(mass, moment));
    cpBodySetPosition(ballBody, cpv(0, 100));

    cpShape *ballShape = cpSpaceAddShape(space, cpCircleShapeNew(ballBody, radius, cpvzero));
    cpShapeSetFriction(ballShape, 0.7);

    b32 paused = 0;
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

        if(IsKeyActionJustDown(appState, ACTION_MOUSE_BUTTON_LEFT))
        {
            DebugOut("mx %d, my %d, nmx %f, nmy %f", 
                    appState->mx, appState->my,
                    appState->normalizedMX, appState->normalizedMY);
        }

        // Clear screen
        Vec4 clearColor = ARGBToVec4(0xffe0fffe);
        glClearColor(clearColor.x, clearColor.y, clearColor.z, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, appState->screenWidth, appState->screenHeight);

        // Update Camera
#if 0
        r32 camSpeed = 0.6;
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
#endif
        if(IsKeyActionJustDown(appState, ACTION_P))
        {
            paused = !paused;
            DebugOut(paused ? "Paused" : "Resumed");
        }
#if 0
        if(IsKeyActionJustDown(appState, ACTION_R))
        {
            UnloadShader(&simpleShader);
            LoadShader(&simpleShader);
            UnloadShader(&spriteShader);
            LoadShader(&spriteShader);
        }
#endif 

        glBindTexture(GL_TEXTURE_2D, atlas->textureHandle);
        AtlasRegion circleRegion = atlas->regions[0];
        AtlasRegion squareRegion = atlas->regions[1];
        // Draw some 2D
#if 0
        Mat3 transform = m3_translation_and_scale(
                vec2(-appState->screenWidth/2.0, -appState->screenHeight/2.0), 
                2.0/(appState->screenWidth), -2.0/(appState->screenHeight));
#endif

        // Do physics step
        cpVect ballPos = cpBodyGetPosition(ballBody);
        cpFloat ballAngle = cpBodyGetAngle(ballBody);
        cpSpaceStep(space, 1.0/60.0);

        UpdateCamera2D(camera, appState);
        int matLocation = glGetUniformLocation(spriteShader->program, "transform");
        glUseProgram(spriteShader->program);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&camera->transform);
        BeginSpritebatch(batch);
        local_persist r32 coolTime = 0.0;
        coolTime+=0.01;


        // Draw physics world.
        batch->colorState = vec4(0,0,0,1);
        PushRect2(batch, vec2(ballPos.x-radius, ballPos.y-radius),
                vec2(radius*2, radius*2),
                circleRegion.pos, 
                circleRegion.size);
        batch->colorState = vec4(0,1,0,1);

        PushLine2(batch, vec2(ballPos.x, ballPos.y), 
                vec2(ballPos.x+cosf(ballAngle)*radius, ballPos.y+sinf(ballAngle)*radius),
                2.0, squareRegion.pos, squareRegion.size);
        
        batch->colorState = vec4(1,0,0,1);
        PushLine2(batch, vec2(groundFrom.x, groundFrom.y), vec2(groundTo.x, groundTo.y), 2.0, squareRegion.pos, squareRegion.size);

        // Test
        PushOrientedLineRectangle2(batch,
                vec2(100*cosf(coolTime*3),300*sinf(coolTime*4)), 
                200,
                100,
                coolTime,
                3,
                &squareRegion);

        PushLine2(batch, vec2(0,0), vec2(appState->mx-appState->screenWidth/2, 
                    appState->my-appState->screenHeight/2), 9.0, squareRegion.pos, squareRegion.size);

        EndSpritebatch(batch);

        glBindTexture(GL_TEXTURE_2D, fontRenderer.font12Texture);
        FitCamera2DToScreen(screenCamera, appState);
        UpdateCamera2D(screenCamera, appState);
        BeginSpritebatch(batch);
        glUniformMatrix3fv(matLocation, 1, 0, (GLfloat *)&screenCamera->transform);

        DrawString2D(batch, &fontRenderer, vec2(20, 900), "heyy");

        EndSpritebatch(batch);

        // UI
        enum {EASY, HARD};
        if(nk_begin(ctx, "Cool Window", nk_rect(50, 50, 220, 220),
                NK_WINDOW_BORDER | NK_WINDOW_TITLE | NK_WINDOW_MOVABLE|NK_WINDOW_SCALABLE))
        {
            nk_layout_row_static(ctx, 30, 80, 1);
            local_persist int op = EASY;
            if(nk_option_label(ctx, "easy", op==EASY)) op=EASY;
            if(nk_option_label(ctx, "hard", op==HARD)) op=HARD;
        }
        nk_end(ctx);

        nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
        // End of UI

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

