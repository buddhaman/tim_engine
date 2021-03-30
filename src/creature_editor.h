
typedef enum
{
    EDIT_CREATURE_NONE,
    EDIT_CREATURE_WIDTH,
    EDIT_CREATURE_HEIGHT,
} EditCreatureState;

typedef struct
{
    CreatureDefinition *creatureDefinition;
    Camera2D *camera;
    Gui *gui;

    BodyPartDefinition *selectedBodyPart;

    // Bodypart editing info
    EditCreatureState editState;
    Vec2 widthDragPos;
    Vec2 heightDragPos;

} CreatureEditorScreen;

