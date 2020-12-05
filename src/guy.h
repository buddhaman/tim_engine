#ifndef GUY_H
#define GUY_H

typedef struct
{
    Vec3 pos;
    r32 orientation;
    r32 unit;
    Body *body;

    Verlet *head;
    Verlet *lFoot;
    Verlet *rFoot;
} Guy;

#endif
