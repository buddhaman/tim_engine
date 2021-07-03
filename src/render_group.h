
typedef struct RenderGroup RenderGroup;
typedef struct RenderCommand RenderCommand;

typedef enum RenderCommandType RenderCommandType;
enum RenderCommandType
{
    RENDER_2D_RECT,
    RENDER_2D_ORIENTED_RECT,
    RENDER_2D_LINE,
};

// For now explicitly create commands for geometric shapes.
struct RenderCommand
{
    RenderCommandType type;

    // TODO: Is this safe ? 
    union
    {
        Vec2 from;
        Vec2 pos;
    };

    union
    {
        Vec2 to;
        Vec2 dims;
    };

    // For oriented rectangle 
    Vec2 center;
    r32 thickness;
    Vec4 color;
    AtlasRegion *texture;
};

struct RenderGroup 
{
    RenderCommand *commands;
    ui32 nCommands;
    ui32 maxCommands;
};

