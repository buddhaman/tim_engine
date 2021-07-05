
typedef struct RenderGroup RenderGroup;
typedef struct RenderCommand RenderCommand;

typedef enum RenderCommandType RenderCommandType;
enum RenderCommandType
{
    RENDER_2D_RECT,
    RENDER_2D_ORIENTED_RECT,
    RENDER_2D_LINE,
    RENDER_2D_SEMICIRCLE,
};

// For now explicitly create commands for geometric shapes.
struct RenderCommand
{
    RenderCommandType type;

    union
    {
        Vec2 from;
        Vec2 pos;
    };

    union
    {
        Vec2 to;
        Vec2 dims;
        Vec2 range;     // Angle range in combination with pos.
    };

    union
    {
        r32 angle;
        r32 radius; 
    };

    ui32 nPoints;
    r32 lineWidth;
    Vec4 color;

    ui32 textureHandle;
    Vec2 uvPos;
    Vec2 uvDims;
};

struct RenderGroup 
{
    RenderCommand *commands;
    ui32 nCommands;
    ui32 maxCommands;
};

