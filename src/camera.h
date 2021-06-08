
typedef struct
{
    Vec2 pos;
    Vec2 size;
    r32 scale;
    b32 isYUp;
    Mat3 transform;
    b32 isDragging;

    Vec2 mousePos;  // World coordinates.
} Camera2D;

