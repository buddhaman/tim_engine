
typedef struct RotaryMuscle RotaryMuscle;
typedef struct BodyPart BodyPart;

struct RotaryMuscle
{
    BodyPart *bodyA;
    BodyPart *bodyB;
    R32 minAngle;
    R32 maxAngle;
    // Control by fixing rate and setting max force by cpConstraintSetMaxForce();
    cpConstraint *motor;
    cpConstraint *rotaryLimitConstraint;
    cpConstraint *pivotConstraint;
};

struct BodyPart
{
    RigidBody *body;
    RigidBody *parent;
    Vec4 color;
    BodyPartDefinition *def;
    RotaryMuscle *rotaryMuscle; // New bodyparts always have this muscle but cannot always control it.
};

typedef struct
{
    U32 physicsGroup;

    B32 drawSolidColor;
    Vec3 solidColor;

    R32 internalClock;
    U32 nInternalClocks;
    R32 phases[4];
    R32 frequencies[4];

    int nBodyParts;
    BodyPart *bodyParts;

    int nRotaryMuscles;
    RotaryMuscle *rotaryMuscles;

    MinimalGatedUnit *brain;
} Creature;

Creature *
AddCreature(FakeWorld *world, Vec2 pos, CreatureDefinition *def, MinimalGatedUnit *brain);

void
DestroyCreature(FakeWorld *world, Creature *creature);

void
UpdateCreature(FakeWorld *world, Creature *creature);

