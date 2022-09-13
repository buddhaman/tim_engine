
#ifndef WORLD_H
#define WORLD_H

typedef struct RigidBody RigidBody;
typedef struct FakeWorld FakeWorld;

struct RigidBody
{
    cpBody *body;
    cpShape *shape;

    R32 width;
    R32 height;
    Vec2 pos;

    R32 drag;
};

#include "creature.h"

typedef enum TrainingScenario TrainingScenario;

enum TrainingScenario
{
    TRAIN_DISTANCE_X,
    TRAIN_DISTANCE_TARGET,
    TRAIN_WALK_RIGHT
};

typedef struct StaticPlatform StaticPlatform;
struct StaticPlatform
{
    RigidBody *body;
    Rect2 bounds;
};

typedef struct Grass Grass;
struct Grass
{
    Vec2 pos;
    R32 width;
    R32 topHeight;
    R32 ovalHeight;
};

typedef struct TallGrass TallGrass;
struct TallGrass
{
    Vec2 from;
    int nBlades;
    Vec2 to[6];
};

typedef struct Bush Bush;
struct Bush
{
    int nLeafs;
    Vec2 leafs[16];
    R32 r[16];
};

// For now most of the arrays are fixed size.
struct FakeWorld
{
    MemoryArena *persistentMemory;
    MemoryArena *transientMemory;
    cpSpace *space;

    Vec2 origin;
    Vec2 size;

    U32 nGenes;
    CreatureDefinition def;
    EvolutionStrategies *strategies;

    U32 physicsGroupCounter;

    TrainingScenario trainingType;
    Vec2 target;

    int nRigidBodies;
    int maxRigidBodies;
    RigidBody *rigidBodies;

    int nStaticBodies;
    int maxStaticBodies;
    RigidBody *staticBodies;

    int nStaticPlatforms;
    int maxStaticPlatforms;
    StaticPlatform *staticPlatforms;

    int nGrass;
    int maxGrass;
    Grass *grass;

    int nTallGrass;
    int maxTallGrass;
    TallGrass *tallGrass;

    int nBushes;
    int maxBushes;
    Bush *bushes;

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

