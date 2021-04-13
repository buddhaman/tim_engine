
#ifndef FAKE_WORLD_SCREEN_H
#define FAKE_WORLD_SCREEN_H

typedef struct
{
    FakeWorld *world;
    MemoryArena *evolutionArena;

    b32 isPaused;
    ui32 stepsPerFrame;

    ui32 generation;
    ui32 tick;
    ui32 ticksPerGeneration;
    r32 avgFitness;

    Camera2D *camera;
    
    BodyPart *hitBodyPart;
    Creature *selectedCreature;
    BodyPart *selectedBodyPart;

    b32 isGuiInputCaptured;

    b32 isPopulationVisible;
    b32 isDragVisible;

} FakeWorldScreen;

#endif

