
typedef struct
{
    cpBody *body;
    r32 width;
    r32 height;
    Vec2 pos;
} RigidBody;

typedef struct
{
    cpSpace *space;

    int nRigidBodies;
    int maxRigidBodies;
    RigidBody *rigidBodies;
} World;
