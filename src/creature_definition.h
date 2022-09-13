
#ifndef CREATURE_DEFINITION_H
#define CREATURE_DEFINITION_H

#define MAX_BODYPARTS 16
#define CREATURE_TEX_SIZE 2048
#define MAX_CREATURE_NAME_LENGTH 64
#define MAX_CREATURE_FILES 64
#define CREATURE_FOLDER_NAME "crdefs"

typedef struct CreatureIO CreatureIO;
struct CreatureIO 
{
    B32 activated;
    I32 index;
};

#define N_BODYPART_SENSORS 2
#define N_BODYPART_ACTUATORS 1

typedef struct
{
    // Sensors
    union 
    {
        CreatureIO sensors[N_BODYPART_SENSORS];
        struct
        {
            CreatureIO xOrientationSensor;
            CreatureIO yOrientationSensor;
        };
    };
    // Actuators
    union 
    {
        CreatureIO actuators[N_BODYPART_ACTUATORS];
        struct 
        {
            CreatureIO rotaryMuscleActuator;
        };
    };

    U32 id;
    U16 degree;
    Vec2 pos;
    R32 width;
    R32 height;
    R32 angle;
    R32 localAngle;

    // Connection to other bodypart
    U32 connectionId;
    int xEdge;
    int yEdge;
    R32 offset;
    Vec2 pivotPoint;    // Calculated from above variables.
    R32 minAngle;
    R32 maxAngle;

    // Cosmetic
    Vec2 uvPos;
    Vec2 uvDims;
    int texGridX;
    int texGridY;
    R32 texScale;
} BodyPartDefinition;

typedef struct
{
    U32 nBodyParts;
    U32 nInputs;
    U32 nOutputs;
    U32 nHidden;
    U32 geneSize;
    U32 nInternalClocks;
    R32 textureOverhang;
    B32 drawSolidColor;
    Vec3 solidColor;

    // Texture atlas
    U32 creatureTextureGridDivs;
    B32 isTextureSquareOccupied[16];

    BodyPartDefinition bodyParts[MAX_BODYPARTS];
    BodyPartDefinition *drawOrder[MAX_BODYPARTS];
    char name[MAX_CREATURE_NAME_LENGTH];
} CreatureDefinition;

#endif

