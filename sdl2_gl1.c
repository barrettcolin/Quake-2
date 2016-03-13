#include "SDL.h"

#include "ref_gl/gl_local.h"

static SDL_Window *window;
static SDL_GLContext GLcontext;

int GLimp_Init(void *hinstance, void *hWnd)
{
    return 1;
}

void GLimp_Shutdown(void)
{
    SDL_GL_DeleteContext(GLcontext);
    SDL_DestroyWindow(window);
}

int GLimp_SetMode(int *pwidth, int *pheight, int mode, qboolean fullscreen)
{
    if(!ri.Vid_GetModeInfo(pwidth, pheight, mode))
    {
        return rserr_invalid_mode;
    }

    SDL_GL_DeleteContext(GLcontext);
    if(window)
        SDL_DestroyWindow(window);

    window = SDL_CreateWindow("Quake 2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, *pwidth, *pheight, SDL_WINDOW_OPENGL);

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    GLcontext = SDL_GL_CreateContext(window);

    ri.Vid_NewWindow(*pwidth, *pheight);

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
        SDL_GL_SwapWindow(window);
}
