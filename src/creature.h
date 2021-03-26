
typedef struct
{
    RigidBody *body;
    Vec4 color;
} BodyPart;

typedef struct
{
    BodyPart *bodyA;
    BodyPart *bodyB;
    r32 minAngle;
    r32 maxAngle;
    // Control by fixing rate and setting max force by cpConstraintSetMaxForce();
    cpConstraint *motor;
    cpConstraint *rotaryLimitConstraint;
    cpConstraint *pivotConstraint;
} RotaryMuscle;

typedef struct
{
    Vec2 pivotPoint;
    ui32 bodyPartIdx0;
    ui32 bodyPartIdx1;
    r32 minAngle;
    r32 maxAngle;
} RotaryMuscleDefinition;

typedef struct
{
    Vec2 pos;
    r32 width;
    r32 height;
    r32 angle;
} BodyPartDefinition;

typedef struct
{
    ui32 nBodyParts;
    BodyPartDefinition bodyParts[32];
    ui32 nRotaryMuscles;
    RotaryMuscleDefinition rotaryMuscles[32];
} CreatureDefinition;

typedef struct
{
    r32 internalClock;
    ui32 physicsGroup;

    int nBodyParts;
    BodyPart *bodyParts;

    int nRotaryMuscles;
    RotaryMuscle *rotaryMuscles;

    MinimalGatedUnit *brain;
} Creature;

Creature *
AddCreature(FakeWorld *world, Vec2 pos, CreatureDefinition *def, MinimalGatedUnit *brain);

void
DestroyCreature(Creature *creature);

void
UpdateCreature(FakeWorld *world, Creature *creature);

