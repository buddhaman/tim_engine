
#ifndef CREATURE_DEFINITION_H
#define CREATURE_DEFINITION_H

#define MAX_BODYPARTS 16
#define CREATURE_TEX_SIZE 2048
#define MAX_CREATURE_NAME_LENGTH 64
#define MAX_CREATURE_FILES 64
#define CREATURE_FOLDER_NAME "crdefs"

typedef struct
{
    ui32 id;
    ui16 degree;
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

    b32 hasDragOutput;
    b32 hasRotaryMuscleOutput;
    b32 hasAbsoluteXPositionInput;      
    b32 hasAbsoluteYPositionInput;       
    b32 hasAngleTowardsTargetInput;
    b32 hasAbsoluteAngleInput;

    ui32 absoluteXPositionInputIdx;
    ui32 absoluteYPositionInputIdx;
    ui32 angleTowardsTargetInputIdx;
    ui32 absoluteAngleInputIdx;

    ui32 dragOutputIdx;
    ui32 rotaryMuscleOutputIdx;
    //b32 hasTargetOrientationInput;

    r32 rotaryMuscleStrength;   //TODO: implement. Not used yet.

    // Cosmetic
    Vec2 uvPos;
    Vec2 uvDims;
    int texGridX;
    int texGridY;
    r32 texScale;
} BodyPartDefinition;

typedef struct
{
    ui32 nBodyParts;
    ui32 nInputs;
    ui32 nOutputs;
    ui32 nHidden;
    ui32 geneSize;
    ui32 nInternalClocks;
    r32 textureOverhang;
    b32 drawSolidColor;
    Vec3 solidColor;

    // Texture atlas
    ui32 creatureTextureGridDivs;
    b32 isTextureSquareOccupied[16];

    BodyPartDefinition bodyParts[MAX_BODYPARTS];
    BodyPartDefinition *drawOrder[MAX_BODYPARTS];
    char name[MAX_CREATURE_NAME_LENGTH];
} CreatureDefinition;

#endif

