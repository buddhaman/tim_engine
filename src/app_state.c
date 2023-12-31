KeyAction
MapKeyCodeToAction(SDL_Keycode code)
{
    switch(code)
    {
        case SDLK_w: { return ACTION_UP; } break;
        case SDLK_a: { return ACTION_LEFT; } break;
        case SDLK_s: { return ACTION_DOWN; } break;
        case SDLK_d: { return ACTION_RIGHT; } break;
        case SDLK_z: { return ACTION_Z; } break;
        case SDLK_x: { return ACTION_X; } break;
        case SDLK_e: { return ACTION_E; } break;
        case SDLK_q: { return ACTION_Q; } break;
        case SDLK_r: { return ACTION_R; } break;
        case SDLK_p: { return ACTION_P; } break;
        case SDLK_UP: { return ACTION_UP; } break;
        case SDLK_DOWN: { return ACTION_DOWN; } break;
        case SDLK_LEFT: { return ACTION_LEFT; } break;
        case SDLK_RIGHT: { return ACTION_RIGHT; } break;
        case SDLK_ESCAPE: { return ACTION_ESCAPE; } break;
        case SDLK_BACKSPACE: { return ACTION_DELETE; } break;
        case SDLK_DELETE: { return ACTION_DELETE; } break;
        default: { return ACTION_UNKNOWN; } break;
    }
}

void
RegisterTextInput(AppState *appState, char *text)
{
    strcpy(appState->textInput, text);
}

void
RegisterKeyAction(AppState *appState, KeyAction action, B32 down)
{
    appState->isActionDown[action] = down;
}

void
AppStateInputBegin(AppState *appState)
{
    appState->mouseScrollY = 0;
    memcpy(appState->wasActionDown, appState->isActionDown, sizeof(appState->isActionDown));
    memset(appState->textInput, 0, sizeof(appState->textInput));
}

void
AppStateInputEnd(AppState *appState)
{
}

void
HandleSDLEvent(AppState *appState, SDL_Event *event)
{
    switch(event->type)
    {

    case SDL_MOUSEBUTTONUP:
    case SDL_MOUSEBUTTONDOWN:
    {
        KeyAction mouseButton;
        switch(event->button.button)
        {
        case SDL_BUTTON_LEFT: mouseButton = ACTION_MOUSE_BUTTON_LEFT; break;
        case SDL_BUTTON_RIGHT: mouseButton = ACTION_MOUSE_BUTTON_RIGHT; break;
        default: mouseButton = ACTION_UNKNOWN;
        }
        if(mouseButton)
        {
            B32 down = event->type==SDL_MOUSEBUTTONDOWN;
            appState->isActionDown[mouseButton] = down;
        }
    } break;

    }
}

B32
IsKeyActionDown(AppState *appState, KeyAction action)
{
    return appState->isActionDown[action];
}

B32
IsKeyActionJustDown(AppState *appState, KeyAction action)
{
    return appState->isActionDown[action] && !appState->wasActionDown[action];
}

B32
IsKeyActionJustReleased(AppState *appState, KeyAction action)
{
    return !appState->isActionDown[action] && appState->wasActionDown[action];
}

void
StartFakeWorld(AppState *appState, CreatureDefinition *definition, Assets *assets, U32 nGenes, R32 dev, R32 learningRate)
{
    MemoryArena *arena = appState->fakeWorldArena;
    ClearArena(arena);
    appState->fakeWorldScreen = PushStruct(arena, FakeWorldScreen);
    InitFakeWorldScreen(appState, appState->fakeWorldScreen, arena, assets, definition, nGenes, dev, learningRate);
    appState->currentScreen = SCREEN_FAKE_WORLD;
}

