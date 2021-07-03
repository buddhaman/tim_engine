
void 
ExecuteRenderGroup(RenderGroup *renderGroup, RenderContext *context)
{
    SpriteBatch *batch = context->batch;
    for(ui32 commandIdx = 0;
            commandIdx < renderGroup->nCommands;
            commandIdx++)
    {
        RenderCommand *command = renderGroup->commands+commandIdx;

        switch(command->type)
        {

        case RENDER_2D_RECT:
        {
            PushRect2(batch,
                    command->pos,
                    command->dims,
                    command->texture.pos, 
                    command->texture.size);
        } break;

        default:
        {
            DebugOut("RENDERCOMMAND NOT IMPLEMENTED YET");
        } break;

        }
    }
}

void
InitRenderGroup(MemoryArena *arena, RenderGroup *renderGroup)
{
    renderGroup->nCommands = 0;
    renderGroup->maxCommands = 10000;
    renderGroup->commands = PushAndZeroArray(arena, RenderCommand, renderGroup->maxCommands);
}


