#ifndef GUY_H
#define GUY_H


typedef struct Guy Guy;

typedef enum
{
    BT_SUCCESS,
    BT_FAIL,
    BT_PROCESSING
}BTNodeResult;

typedef enum
{
    BT_SEQUENCE,
    BT_SELECTOR,
    BT_LEAF,
} BTNodeType;

#define BTNODE_UPDATE_FUNCTION(name) BTNodeResult name(void *args)
typedef BTNODE_UPDATE_FUNCTION(btnode_update);

typedef struct BTNode BTNode;
struct BTNode
{
    BTNodeType type;
    btnode_update *update;
    int nChildren;
    BTNode *children;
};

typedef struct
{
    r32 unit;
    Body *body;
    Guy *guy;
    Verlet *handle;
    Verlet *tip;
} Sword;

struct Guy
{
    Vec3 pos;
    r32 orientation;
    r32 unit;
    Body *body;
    BTNode *behavior;

    Sword *sword;

    Verlet *head;
    Verlet *lFoot;
    Verlet *rFoot;
    Verlet *lHand;
    Verlet *rHand;
};

#endif
