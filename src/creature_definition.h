
#ifndef CREATURE_DEFINITION_H
#define CREATURE_DEFINITION_H

typedef struct
{
    ui32 id;
    Vec2 pos;
    r32 width;
    r32 height;
    r32 angle;
    r32 localAngle;

    // Connection to other bodypart
    ui32 connectionId;
    int xEdge;
    int yEdge;
    r32 offset;
    Vec2 pivotPoint;    // Calculated from above variables.
    r32 minAngle;
    r32 maxAngle;
} BodyPartDefinition;

#define MAX_BODYPARTS 32

typedef struct
{
    ui32 nBodyParts;
    BodyPartDefinition bodyParts[MAX_BODYPARTS];
} CreatureDefinition;

#endif
