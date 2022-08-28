
typedef enum
{
    EDIT_PHASE_BODY,
    EDIT_PHASE_BRAIN,
} EditPhase;

typedef enum
{
    EDIT_CREATURE_NONE,
    EDIT_CREATURE_BODYPART,
    EDIT_ADD_BODYPART_PLACE,
    EDIT_CREATURE_DRAW,
} EditCreatureState;

typedef enum
{
    CREATURE_TOOL_SELECT,
    CREATURE_TOOL_BRUSH,
} EditCreatureTool;

#define MAX_EDITOR_PARTICLES 256
typedef struct
{
    CreatureDefinition *creatureDefinition;
    BasicRenderTools *renderTools;

    EditPhase editPhase;

    Gui *gui;
    B32 isInputCaptured;
    
    U32 idCounter;
    U32 selectedId;
    U32 rightSelectedId;

    B32 isDimSnapEnabled;
    R32 dimSnapResolution;
    B32 isAngleSnapEnabled;
    R32 angleSnapResolution;
    B32 isEdgeSnapEnabled;
    U32 edgeSnapDivisions;

    // Simulation settings
    U32 nGenes;
    R32 learningRate;
    R32 deviation;

    // Bodypart editing info
    EditCreatureState editState;
    EditCreatureState prevEditState;        //TODO: Is this a hack? 
    BoxEdgeLocation bodyPartLocation;
    BodyPartDefinition *attachTo;

    B32 isCreatureColorPickerVisible;

    struct nk_colorf creatureSolidColor;
    B32 isSolidColorPickerVisible;

    struct nk_colorf brushColor;
    B32 isBrushColorPickerVisible;

    B32 canMoveCameraWithMouse;
    B32 canScrollCameraWithMouse;

    B32 isErasing;
    R32 brushSize;
    B32 drawBrushInScreenCenter;
    B32 hasLastBrushStroke;
    Vec2 lastBrushStroke;
    B32 isMouseInDrawArea;

    B32 showSaveScreen;

    B32 showLoadScreen;
    U32 nCreatureFiles;
    CreatureDefinitionFile creatureFiles[MAX_CREATURE_FILES];

    Vec2 gravity;
    int nParticles;
    Particle particles[MAX_EDITOR_PARTICLES];

    BodyPartDefinition *animateBodyPart;
    R32 animationTime;

} CreatureEditorScreen;

