#include "SDL.h"

#include "client/client.h"
#include "game/game.h"

unsigned sys_frame_time;
qboolean stdin_active = true;

// Sys
void Sys_Init(void)
{

}

void Sys_Quit(void)
{
    //<todo.cb SDL_QUIT
    exit(0);
}

void Sys_Error(char *error, ...)
{

}

void Sys_AppActivate(void)
{

}

void Sys_SendKeyEvents(void)
{
    sys_frame_time = Sys_Milliseconds();
}

char *Sys_GetClipboardData(void)
{
    return NULL;
}

game_export_t *GetGameAPI(game_import_t *import);

void *Sys_GetGameAPI(void *parms)
{
    return GetGameAPI(parms);
}

void Sys_UnloadGame(void)
{

}

char *Sys_ConsoleInput(void)
{
    return NULL;
}

void Sys_ConsoleOutput(char *string)
{

}

// IN
cvar_t *in_joystick;

void IN_Init(void)
{
}

void IN_Shutdown(void)
{
}

void IN_Commands(void)
{
}

void IN_Frame(void)
{
}

void IN_Move(usercmd_t *cmd)
{
}

void IN_Activate(qboolean active)
{
}

void IN_ActivateMouse(void)
{
}

void IN_DeactivateMouse(void)
{
}

// CDAudio
int CDAudio_Init(void)
{
    return 0;
}

void CDAudio_Shutdown(void)
{
}

void CDAudio_Play(int track, qboolean looping)
{
}


void CDAudio_Stop(void)
{
}


void CDAudio_Resume(void)
{
}


void CDAudio_Update(void)
{
}

static int quakeKey(SDL_Keycode key)
{
    switch (key)
    {
    case SDLK_TAB: return K_TAB;
    case SDLK_RETURN: return K_ENTER;
    case SDLK_ESCAPE: return K_ESCAPE;
    case SDLK_SPACE: return K_SPACE;

    case SDLK_BACKSPACE: return K_BACKSPACE;
    case SDLK_UP: return K_UPARROW;
    case SDLK_DOWN: return K_DOWNARROW;
    case SDLK_LEFT: return K_LEFTARROW;
    case SDLK_RIGHT: return K_RIGHTARROW;

    case SDLK_LALT:
    case SDLK_RALT: return K_ALT;
    case SDLK_LCTRL:
    case SDLK_RCTRL: return K_CTRL;
    case SDLK_LSHIFT:
    case SDLK_RSHIFT: return K_SHIFT;

    case SDLK_F1:
    case SDLK_F2:
    case SDLK_F3:
    case SDLK_F4:
    case SDLK_F5:
    case SDLK_F6:
    case SDLK_F7:
    case SDLK_F8:
    case SDLK_F9:
    case SDLK_F10:
    case SDLK_F11: return (key - SDLK_F1) + K_F1;

    case SDLK_INSERT: return K_INS;
    case SDLK_DELETE: return K_DEL;
    case SDLK_PAGEUP: return K_PGUP;
    case SDLK_PAGEDOWN: return K_PGDN;
    case SDLK_HOME: return K_HOME;
    case SDLK_END: return K_END;

        //< SDL keys not exposed in Urho
    case SDLK_BACKQUOTE: return '~';
    case SDLK_MINUS: return '-';
    case SDLK_LEFTBRACKET: return '[';
    case SDLK_RIGHTBRACKET: return ']';
    case SDLK_SEMICOLON: return ';';
    case SDLK_COMMA: return ',';
    case SDLK_PERIOD: return '.';

    case SDLK_0:
    case SDLK_1:
    case SDLK_2:
    case SDLK_3:
    case SDLK_4:
    case SDLK_5:
    case SDLK_6:
    case SDLK_7:
    case SDLK_8:
    case SDLK_9: return (key - SDLK_0) + '0';

    default: return (key - SDLK_a) + 'a';
    }
}

int main(int argc, char **argv)
{
    Qcommon_Init(argc, argv);
    {
        int prevTime, currTime;
        prevTime = Sys_Milliseconds();

        for(;;)
        {
            int deltaTime;
            currTime = Sys_Milliseconds();

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                case SDL_KEYDOWN:
                    Key_Event(quakeKey(event.key.keysym.sym), true, currTime);
                    break;

                case SDL_KEYUP:
                    Key_Event(quakeKey(event.key.keysym.sym), false, currTime);
                    break;
                }
            }

            deltaTime = currTime - prevTime;

            if(deltaTime == 0)
                continue;

            Qcommon_Frame(deltaTime);

            prevTime = currTime;
        }
    }

    return 0;
}
