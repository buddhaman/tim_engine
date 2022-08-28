
#ifndef FAKE_WORLD_SCREEN_H
#define FAKE_WORLD_SCREEN_H

typedef struct
{
    FakeWorld *world;
    MemoryArena *evolutionArena;
    BasicRenderTools *renderTools;

    FrameBuffer *frameBuffer0;
    FrameBuffer *frameBuffer1;

    B32 isPaused;
    U32 stepsPerFrame;
    B32 isInputCaptured;

    U32 generation;
    U32 tick;
    U32 ticksPerGeneration;
    
    R32 avgFitness;
    Vec2 target;

    BodyPart *hitBodyPart;
    Creature *selectedCreature;
    BodyPart *selectedBodyPart;

    B32 isGuiInputCaptured;

    B32 isPopulationVisible;
    B32 isDragVisible;

} FakeWorldScreen;

void 
InitFakeWorldScreen(AppState *appState, 
        FakeWorldScreen *screen, 
        MemoryArena *arena, 
        Assets *assets,
        CreatureDefinition *def,
        U32 nGenes,
        R32 dev,
        R32 learningRate);

#endif

