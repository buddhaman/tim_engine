
typedef struct RigidBody RigidBody;
typedef struct FakeWorld FakeWorld;
typedef struct FakeWorldParams FakeWorldParams;

struct RigidBody
{
    cpBody *body;

    r32 width;
    r32 height;
    Vec2 pos;

    r32 drag;
};

#include "creature.h"

// For now most of the arrays are fixed size. World is destroyed after each generation.
struct FakeWorld
{
    MemoryArena *arena;
    cpSpace *space;

    ui32 nGenes;
    ui32 geneSize;
    EvolutionStrategies *strategies;

    ui32 physicsGroupCounter;

    int nRigidBodies;
    int maxRigidBodies;
    RigidBody *rigidBodies;

    int nCreatures;
    int maxCreatures;
    Creature *creatures;

    int nBodyParts;
    int maxBodyParts;
    BodyPart *bodyParts;

    int nRotaryMuscles;
    int maxRotaryMuscles;
    RotaryMuscle *rotaryMuscles;
};

