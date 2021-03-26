
typedef struct RigidBody RigidBody;
typedef struct FakeWorld FakeWorld;
typedef struct FakeWorldParams FakeWorldParams;

struct RigidBody
{
    cpBody *body;
    cpShape *shape;

    r32 width;
    r32 height;
    Vec2 pos;

    r32 drag;
};

#include "creature.h"

// For now most of the arrays are fixed size. World is destroyed after each generation.
struct FakeWorld
{
    MemoryArena *persistentMemory;
    MemoryArena *transientMemory;
    cpSpace *space;

    ui32 inputSize;
    ui32 outputSize;
    ui32 hiddenSize;
    ui32 nGenes;
    ui32 geneSize;
    CreatureDefinition def;
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

