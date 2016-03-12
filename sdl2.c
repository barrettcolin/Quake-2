#include "SDL.h"

#include "qcommon/qcommon.h"
#include "game/game.h"

unsigned sys_frame_time;
qboolean stdin_active = true;
cvar_t *in_joystick;

void Sys_Init(void)
{

}

void Sys_Quit(void)
{

}

void Sys_Error(char *error, ...)
{

}

void Sys_AppActivate(void)
{

}

void Sys_SendKeyEvents(void)
{

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
            deltaTime = currTime - prevTime;

            if(deltaTime == 0)
                continue;

            Qcommon_Frame(deltaTime);

            prevTime = currTime;
        }
    }

    return 0;
}
