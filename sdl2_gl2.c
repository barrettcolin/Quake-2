#include "SDL.h"

#include "ref_gles2/gl_local.h"

static SDL_Window *window;
static SDL_GLContext GLcontext;

int GLimp_Init(void *hinstance, void *hWnd)
{
    return 1;
}

void GLimp_Shutdown(void)
{
    if(GLcontext)
    {
        SDL_GL_DeleteContext(GLcontext);
        GLcontext = 0;
    }

    if(window)
    {
        SDL_DestroyWindow(window);
        window = NULL;
    }
}

int GLimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
    int desiredWidth, desiredHeight, winWidth, winHeight;
    if(!ri.Vid_GetModeInfo(&desiredWidth, &desiredHeight, mode))
    {
        return rserr_invalid_mode;
    }

    //<todo.cb destroy window and context; might be able to do something nicer here
    GLimp_Shutdown();

    Uint32 flags = SDL_WINDOW_OPENGL;
    if(fullscreen)
    {
        flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
    }

    window = SDL_CreateWindow("Quake 2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desiredWidth, desiredHeight, flags);
    if(!window)
    {
        return rserr_unknown;
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    GLcontext = SDL_GL_CreateContext(window);
    if(!GLcontext)
    {
        SDL_DestroyWindow(window);
        window = NULL;
        return rserr_unknown;
    }

    SDL_GetWindowSize(window, &winWidth, &winHeight);

    ri.Vid_NewWindow(winWidth, winHeight);

    *pwidth = winWidth;
    *pheight = winHeight;
    return rserr_ok;
}

void GLimp_AppActivate(qboolean active)
{
}

void GLimp_BeginFrame(float camera_separation)
{
}

void GLimp_EndFrame(void)
{
    if(window)
    {
        SDL_GL_SwapWindow(window);
    }
}

void GLimp_EnableLogging(qboolean enable)
{
}

void GLimp_LogNewFrame()
{
}