#include "SDL.h"

#include "client/client.h"
#include "client/snd_loc.h"
#include "game/game.h"

// Sys
unsigned sys_frame_time;
qboolean stdin_active = true;

game_export_t *GetGameAPI(game_import_t *import);

#define PRINT_MSG_SIZE 4096

void Sys_Init(void)
{
    SDL_Init(SDL_INIT_EVERYTHING);
}

void Sys_Quit(void)
{
    SDL_Quit();

    exit(0);
}

void Sys_Error(char *error, ...)
{
    va_list args;
    char msg[PRINT_MSG_SIZE];

    va_start(args, error);
    vsnprintf(msg, sizeof(msg), error, args);
    va_end(args);

    fputs(msg, stderr);

    exit(1);
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

static qboolean mouseActive;
static Sint32 mouseRelX;
static Sint32 mouseRelY;

static int IN_SDL2_key(SDL_Keycode key)
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

static int IN_SDL2_button(int button)
{
    switch(button)
    {
    case SDL_BUTTON_LEFT: return K_MOUSE1;
    case SDL_BUTTON_RIGHT: return K_MOUSE2;
    case SDL_BUTTON_MIDDLE: return K_MOUSE3;
    default: return 0;
    }
}

void IN_Init(void)
{
    in_joystick = Cvar_Get("in_joystick", "0", CVAR_ARCHIVE);

    SDL_SetRelativeMouseMode(SDL_TRUE);
    mouseActive = true;
}

void IN_Shutdown(void)
{
}

void IN_Frame(void)
{
    qboolean isFullscreen = (Cvar_VariableValue("vid_fullscreen") != 0);
    if(!isFullscreen && (cl.refresh_prepped == false || cls.key_dest == key_console || cls.key_dest == key_menu))
    {
        SDL_SetRelativeMouseMode(SDL_FALSE);
        mouseActive = false;
        return;
    }

    SDL_SetRelativeMouseMode(SDL_TRUE);
    mouseActive = true;
}

void IN_Move(usercmd_t *cmd)
{
    float yawDelta, pitchDelta;

    if(!mouseActive)
    {
        return;
    }

    yawDelta = sensitivity->value * mouseRelX * m_yaw->value;
    pitchDelta = sensitivity->value * mouseRelY * m_pitch->value;

    cl.viewangles[YAW] -= yawDelta;
    cl.viewangles[PITCH] += pitchDelta;
}

void IN_Commands(void)
{
}

// SNDDMA
static int snd_inited;

static int dma_sample_bytes, dma_pos, dma_size;
static unsigned char *dma_buffer;

static unsigned next_power_of_two(unsigned n)
{
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;

    return n;
}

static void paint_audio(void *unused, Uint8 *stream, int num_bytes)
{
    int current_byte = dma_pos * dma_sample_bytes;
    int num_bytes_to_end_of_buffer = dma_size - current_byte;

    int bytes1 = (num_bytes < num_bytes_to_end_of_buffer)
        ? num_bytes
        : num_bytes_to_end_of_buffer;

    memcpy(stream, dma.buffer + current_byte, bytes1);
    dma_pos += (bytes1 / dma_sample_bytes);

    if (bytes1 < num_bytes)
    {
        int bytes2 = num_bytes - bytes1;
        memcpy(stream + bytes1, dma.buffer, bytes2);
        dma_pos = (bytes2 / dma_sample_bytes);
    }
}

qboolean SNDDMA_Init(void)
{
    SDL_AudioSpec desired = { 0 }, obtained = { 0 };

    // desired.freq
    if (s_khz->value == 44)
        desired.freq = 44100;
    else if (s_khz->value == 22)
        desired.freq = 22050;
    else
        desired.freq = 11025;

    // desired.format
    if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
        desired.format = AUDIO_S16MSB;
    else
        desired.format = AUDIO_S16LSB;

    desired.channels = 2;
    desired.samples = 512;
    desired.callback = paint_audio;

    if (SDL_OpenAudio(&desired, &obtained) < 0)
    {
        Com_Printf("Couldn't open SDL audio: %s\n", SDL_GetError());
        return 0;
    }

    memset(&dma, 0, sizeof(dma));
    dma.samplebits = (obtained.format & 0xFF);
    dma.speed = obtained.freq;
    dma.channels = obtained.channels;
    dma.samples = next_power_of_two(dma.speed * dma.channels);
    dma.submission_chunk = 1;

    dma_sample_bytes = dma.samplebits / 8;
    dma_size = (dma.samples * dma_sample_bytes);
    dma.buffer = dma_buffer = calloc(1, dma_size);

    SDL_PauseAudio(0);

    snd_inited = 1;
    return true;
}

int SNDDMA_GetDMAPos(void)
{
    return dma_pos;
}

void SNDDMA_Shutdown(void)
{
    if (snd_inited)
    {
        SDL_PauseAudio(1);
        SDL_CloseAudio();

        free(dma_buffer);
        dma_size = dma_pos = 0;

        snd_inited = 0;
    }
}

void SNDDMA_BeginPainting(void)
{
    SDL_LockAudio();
}

void SNDDMA_Submit(void)
{
    SDL_UnlockAudio();
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

// VID
viddef_t viddef;
refexport_t	re;

//<todo.cb multiply defined in gl_rmain when statically linked, though should have same value
cvar_t *vid_ref;
cvar_t *vid_fullscreen;

refexport_t GetRefAPI (refimport_t rimp);

static qboolean reflib_active;

typedef struct vidmode_s
{
    int width, height;
} vidmode_t;

static vidmode_t vid_modes[] =
{
    { 320, 240 },
    { 400, 300 },
    { 512, 384 },
    { 640, 480 },
    { 800, 600 },
    { 960, 720 },
    { 1024, 768 },
    { 1152, 864 },
    { 1280, 960 },
    { 1600, 1200 },

    // 1080p
    { 720, 540 },
    { 1440, 1080 },
};

#define VID_NUM_MODES (sizeof(vid_modes) / sizeof(vid_modes[0]))

static void VID_Restart_f(void)
{
    vid_ref->modified = true;
}

void VID_Printf(int print_level, char *fmt, ...)
{
    char msg[PRINT_MSG_SIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    if(print_level == PRINT_ALL)
    {
        Com_Printf("%s", msg);
    }
    else
    {
        Com_DPrintf("%s", msg);
    }
}

void VID_Error(int err_level, char *fmt, ...)
{
    char msg[PRINT_MSG_SIZE];
    va_list args;

    va_start(args, fmt);
    vsnprintf(msg, sizeof(msg), fmt, args);
    va_end(args);

    Com_Error(err_level, "%s", msg);
}

qboolean VID_GetModeInfo(int *width, int *height, int mode)
{
    if(mode < 0 || mode >= VID_NUM_MODES)
        return false;

    *width = vid_modes[mode].width;
    *height = vid_modes[mode].height;
    return true;
}

void VID_NewWindow(int width, int height)
{
    viddef.width = width;
    viddef.height = height;
}

void VID_Init(void)
{
    vid_ref = Cvar_Get("vid_ref", "soft", CVAR_ARCHIVE);
    vid_fullscreen = Cvar_Get("vid_fullscreen", "0", CVAR_ARCHIVE);

    Cmd_AddCommand("vid_restart", VID_Restart_f);

    VID_CheckChanges();
}

void VID_Shutdown(void)
{
    if(reflib_active)
    {
        if (re.Shutdown)
        {
            re.Shutdown();
        }

        reflib_active = false;
    }
}

void VID_CheckChanges(void)
{
    if(vid_ref->modified)
    {
        S_StopAllSounds();

        vid_ref->modified = false;
        vid_fullscreen->modified = true;
        cl.refresh_prepped = false;

        VID_Shutdown();

        refimport_t	ri;
        {
            ri.Cmd_AddCommand = Cmd_AddCommand;
            ri.Cmd_RemoveCommand = Cmd_RemoveCommand;
            ri.Cmd_Argc = Cmd_Argc;
            ri.Cmd_Argv = Cmd_Argv;
            ri.Cmd_ExecuteText = Cbuf_ExecuteText;
            ri.Con_Printf = VID_Printf;
            ri.Sys_Error = VID_Error;
            ri.FS_LoadFile = FS_LoadFile;
            ri.FS_FreeFile = FS_FreeFile;
            ri.FS_Gamedir = FS_Gamedir;
            ri.Cvar_Get = Cvar_Get;
            ri.Cvar_Set = Cvar_Set;
            ri.Cvar_SetValue = Cvar_SetValue;
            ri.Vid_GetModeInfo = VID_GetModeInfo;
            ri.Vid_NewWindow = VID_NewWindow;
            ri.Vid_MenuInit = VID_MenuInit;
        }

        re = GetRefAPI(ri);

        if (re.api_version != API_VERSION)
            Com_Error (ERR_FATAL, "Re has incompatible api_version");

        // call the init function
        if (re.Init (NULL, NULL) == -1)
            Com_Error (ERR_FATAL, "Couldn't start refresh");

        reflib_active = true;
    }
}

void VID_MenuInit(void)
{
}

void VID_MenuDraw(void)
{
}

const char *VID_MenuKey(int k)
{
    return NULL;
}

// main
int main(int argc, char **argv)
{
    Qcommon_Init(argc, argv);
    {
        int prevTime = Sys_Milliseconds();

        for(;;)
        {
            int deltaTime, currTime = Sys_Milliseconds();

            mouseRelX = mouseRelY = 0;

            SDL_Event event;
            while (SDL_PollEvent(&event))
            {
                switch(event.type)
                {
                case SDL_KEYDOWN:
                case SDL_KEYUP:
                    Key_Event(IN_SDL2_key(event.key.keysym.sym), event.type == SDL_KEYDOWN, currTime);
                    break;

                case SDL_MOUSEBUTTONDOWN:
                case SDL_MOUSEBUTTONUP:
                    Key_Event(IN_SDL2_button(event.button.button), event.type == SDL_MOUSEBUTTONDOWN, currTime);
                    break;

                case SDL_MOUSEMOTION:
                    mouseRelX = event.motion.xrel;
                    mouseRelY = event.motion.yrel;
                    break;

                case SDL_QUIT:
                    Com_Quit();
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
