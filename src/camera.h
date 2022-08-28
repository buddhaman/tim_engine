
typedef struct
{
    Vec2 pos;
    Vec2 size;
    R32 scale;
    B32 isYUp;
    Mat3 transform;
    B32 isDragging;

    Vec2 mousePos;  // World coordinates.
} Camera2D;

