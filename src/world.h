
typedef struct
{
    Vec3 pos;
    Vec3 oldPos;
} Verlet;

typedef struct 
{
    Verlet *a;
    Verlet *b;
    r32 r;
} Constraint;

typedef struct
{
    Verlet *particles;
    int nParticles;
    
    Constraint *constraints;
    int nConstraints;
} Body;

#include "guy.h"

typedef struct
{
    Verlet *particles;
    int maxParticles;
    int nParticles;

    Constraint *constraints;
    int maxConstraints;
    int nConstraints;

    Body *bodies;
    int nBodies;
    int maxBodies;

    Guy *guys;
    int nGuys;
    int maxGuys;
} World;


