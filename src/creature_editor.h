
typedef enum
{
    EDIT_CREATURE_NONE,
    EDIT_CREATURE_WIDTH,
    EDIT_CREATURE_HEIGHT,
    EDIT_CREATURE_ROTATION,
    EDIT_CREATURE_ROTATION_AND_LENGTH,
    EDIT_CREATURE_MIN_ANGLE,
    EDIT_CREATURE_MAX_ANGLE,
    EDIT_ADD_BODYPART_FIND_EDGE,
    EDIT_ADD_BODYPART_PLACE,
    EDIT_CREATURE_DRAW,
} EditCreatureState;

typedef struct
{
    CreatureDefinition *creatureDefinition;
    Camera2D *camera;
    Gui *gui;
    b32 isInputCaptured;

    ui32 idCounter;
    ui32 selectedId;

    b32 isDimSnapEnabled;
    r32 dimSnapResolution;
    b32 isAngleSnapEnabled;
    r32 angleSnapResolution;
    b32 isEdgeSnapEnabled;
    ui32 edgeSnapDivisions;

    // Simulation settings
    ui32 nGenes;
    r32 learningRate;
    r32 deviation;

    // Bodypart editing info
    EditCreatureState editState;
    EditCreatureState prevEditState;        //TODO: Is this a hack? 
    BoxEdgeLocation bodyPartLocation;
    BodyPartDefinition *attachTo;

    // Texture atlas
    ui32 creatureTextureGridDivs;
    b32 isTextureSquareOccupied[16];
    struct nk_colorf brushColor;
    r32 brushSize;
} CreatureEditorScreen;

