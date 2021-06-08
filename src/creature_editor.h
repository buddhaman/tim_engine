
typedef enum
{
    EDIT_CREATURE_NONE,
    EDIT_CREATURE_WIDTH,
    EDIT_CREATURE_HEIGHT,
    EDIT_CREATURE_ROTATION,
    EDIT_CREATURE_ROTATION_AND_LENGTH,
    EDIT_CREATURE_MIN_ANGLE,
    EDIT_CREATURE_MAX_ANGLE,
    EDIT_ADD_BODYPART_PLACE,
    EDIT_CREATURE_DRAW,
} EditCreatureState;

typedef enum
{
    CREATURE_TOOL_SELECT,
    CREATURE_TOOL_BRUSH,
} EditCreatureTool;

typedef struct
{
    CreatureDefinition *creatureDefinition;
    Camera2D *camera;
    Gui *gui;
    b32 isInputCaptured;
    
    ui32 idCounter;
    ui32 selectedId;
    ui32 rightSelectedId;

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

    b32 isCreatureColorPickerVisible;

    struct nk_colorf creatureSolidColor;
    b32 isSolidColorPickerVisible;

    struct nk_colorf brushColor;
    b32 isBrushColorPickerVisible;

    b32 canMoveCameraWithMouse;

    b32 isErasing;
    r32 brushSize;
    b32 drawBrushInScreenCenter;
    b32 hasLastBrushStroke;
    Vec2 lastBrushStroke;
    b32 isMouseInDrawArea;

    b32 showSaveScreen;

    b32 showLoadScreen;
    ui32 nCreatureFiles;
    CreatureDefinitionFile creatureFiles[MAX_CREATURE_FILES];

} CreatureEditorScreen;

