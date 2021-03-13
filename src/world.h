
typedef struct RigidBody RigidBody;
typedef struct World World;

struct RigidBody
{
    cpBody *body;
    r32 width;
    r32 height;
    Vec2 pos;
};

#include "creature.h"

struct World
{
    MemoryArena *arena;
    cpSpace *space;

    ui32 physicsGroupCounter;
    int nRigidBodies;
    int maxRigidBodies;
    RigidBody *rigidBodies;

    int nCreatures;
    int maxCreatures;
    Creature *creatures;

    ui32 pointerArraySize;
    MemoryPool *pointerArrayPool;

    ui32 maxBodypartsPerCreature;
    MemoryPool *bodyPartPool;

    ui32 maxRotaryMusclesPerCreature;
    MemoryPool *rotaryMusclePool;
};
