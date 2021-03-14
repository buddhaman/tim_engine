
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
} RotaryMuscle;

typedef struct
{
    r32 internalClock;
    ui32 physicsGroup;
    // Eventually hashmap and store in world.
    int nBodyParts;
    int maxBodyParts;
    BodyPart *bodyParts;

    int nRotaryMuscles;
    int maxRotaryMuscles;
    RotaryMuscle *rotaryMuscles;

} Creature;

void
UpdateCreature(World *world, Creature *creature);

