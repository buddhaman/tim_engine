
typedef enum
{
    EDIT_CREATURE_NONE,
    EDIT_CREATURE_WIDTH,
    EDIT_CREATURE_HEIGHT,
    EDIT_CREATURE_ROTATION,
    EDIT_ADD_BODYPART_FIND_EDGE,
    EDIT_ADD_BODYPART_PLACE,
} EditCreatureState;

typedef struct
{
    CreatureDefinition *creatureDefinition;
    Camera2D *camera;
    Gui *gui;

    ui32 idCounter;
    BodyPartDefinition *selectedBodyPart;

    // Bodypart editing info
    EditCreatureState editState;
    BoxEdgeLocation bodyPartLocation;
    BodyPartDefinition *attachTo;
} CreatureEditorScreen;

