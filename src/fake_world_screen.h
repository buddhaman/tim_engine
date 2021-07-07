
#ifndef FAKE_WORLD_SCREEN_H
#define FAKE_WORLD_SCREEN_H

typedef struct
{
    FakeWorld *world;
    MemoryArena *evolutionArena;
    BasicRenderTools *renderTools;

    FrameBuffer *frameBuffer;

    b32 isPaused;
    ui32 stepsPerFrame;
    b32 isInputCaptured;

    ui32 generation;
    ui32 tick;
    ui32 ticksPerGeneration;
    
    r32 avgFitness;
    Vec2 target;

    BodyPart *hitBodyPart;
    Creature *selectedCreature;
    BodyPart *selectedBodyPart;

    b32 isGuiInputCaptured;

    b32 isPopulationVisible;
    b32 isDragVisible;

} FakeWorldScreen;

void 
InitFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        MemoryArena *arena, 
        Assets *assets,
        CreatureDefinition *def,
        ui32 nGenes,
        r32 dev,
        r32 learningRate);

#endif

