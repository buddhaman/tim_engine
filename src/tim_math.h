
typedef struct
{
    Vec2 pos;
    int xEdge;
    int yEdge;
    R32 offset;     // Counterclockwise direction on edge.
} BoxEdgeLocation;

typedef struct
{
    union
    {
        struct
        {
            R32 x;
            R32 y;
            R32 width;
            R32 height;
        };
        struct
        {
            Vec2 pos;
            Vec2 dims;
        };
    };
} Rect2;

typedef struct
{
    Vec2 pos;
    Vec2 dims;
    R32 angle;
} OrientedBox;



