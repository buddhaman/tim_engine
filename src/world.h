
#ifndef WORLD_H
#define WORLD_H

typedef struct RigidBody RigidBody;
typedef struct FakeWorld FakeWorld;

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

typedef enum TrainingScenario TrainingScenario;

enum TrainingScenario
{
    TRAIN_DISTANCE_X,
    TRAIN_DISTANCE_TARGET,
    TRAIN_WALK_RIGHT
};

// For now most of the arrays are fixed size. World is destroyed after each generation.
struct FakeWorld
{
    MemoryArena *persistentMemory;
    MemoryArena *transientMemory;
    cpSpace *space;

    ui32 nGenes;
    CreatureDefinition def;
    EvolutionStrategies *strategies;

    ui32 physicsGroupCounter;

    TrainingScenario trainingType;
    Vec2 target;

    int nRigidBodies;
    int maxRigidBodies;
    RigidBody *rigidBodies;

    int nStaticBodies;
    RigidBody *staticBodies[16];

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

#endif

