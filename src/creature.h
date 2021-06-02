
typedef struct RotaryMuscle RotaryMuscle;
typedef struct BodyPart BodyPart;

struct RotaryMuscle
{
    BodyPart *bodyA;
    BodyPart *bodyB;
    r32 minAngle;
    r32 maxAngle;
    // Control by fixing rate and setting max force by cpConstraintSetMaxForce();
    cpConstraint *motor;
    cpConstraint *rotaryLimitConstraint;
    cpConstraint *pivotConstraint;
};

struct BodyPart
{
    RigidBody *body;
    Vec4 color;
    BodyPartDefinition *def;
    RotaryMuscle *rotaryMuscle; // New bodyparts always have this muscle but cannot always control it.
};

typedef struct
{
    ui32 physicsGroup;

    b32 drawSolidColor;
    Vec3 solidColor;

    r32 internalClock;
    ui32 nInternalClocks;
    r32 phases[4];
    r32 frequencies[4];

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

