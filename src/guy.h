#ifndef GUY_H
#define GUY_H

typedef struct Guy Guy;

typedef struct
{
    r32 unit;
    Body *body;
    Guy *guy;
    Verlet *handle;
    Verlet *tip;
} Sword;

struct Guy
{
    Vec3 pos;
    r32 orientation;
    r32 unit;
    Body *body;

    Sword *sword;

    Verlet *head;
    Verlet *lFoot;
    Verlet *rFoot;
    Verlet *lHand;
    Verlet *rHand;
};

#endif
