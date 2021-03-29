
typedef struct
{
    FakeWorld *world;
    MemoryArena *evolutionArena;

    ui32 generation;
    ui32 tick;
    ui32 ticksPerGeneration;
    ui32 stepsPerFrame;
    r32 avgFitness;

    Camera2D *camera;
} FakeWorldScreen;
