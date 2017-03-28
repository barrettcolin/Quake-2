#include "SDL.h"

#include "ref_gles2/gl_local.h"

void (GL_APIENTRY *qglActiveTexture)(GLenum texture);
void (GL_APIENTRY *qglAttachShader)(GLuint program, GLuint shader);
void (GL_APIENTRY *qglBindBuffer)(GLenum target, GLuint buffer);
void (GL_APIENTRY *qglBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
void (GL_APIENTRY *qglBindTexture)(GLenum target, GLuint texture);
void (GL_APIENTRY *qglBlendFunc)(GLenum sfactor, GLenum dfactor);
void (GL_APIENTRY *qglBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
void (GL_APIENTRY *qglClear)(GLbitfield mask);
void (GL_APIENTRY *qglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
void (GL_APIENTRY *qglCompileShader)(GLuint shader);
GLuint(GL_APIENTRY *qglCreateProgram)(void);
GLuint(GL_APIENTRY *qglCreateShader)(GLenum type);
void (GL_APIENTRY *qglCullFace)(GLenum mode);
void (GL_APIENTRY *qglDeleteBuffers)(GLsizei n, const GLuint* buffers);
void (GL_APIENTRY *qglDeleteProgram)(GLuint program);
void (GL_APIENTRY *qglDeleteShader)(GLuint shader);
void (GL_APIENTRY *qglDeleteTextures)(GLsizei n, const GLuint* textures);
void (GL_APIENTRY *qglDepthFunc)(GLenum func);
void (GL_APIENTRY *qglDepthMask)(GLboolean flag);
void (GL_APIENTRY *qglDepthRangef)(GLclampf zNear, GLclampf zFar);
void (GL_APIENTRY *qglDisable)(GLenum cap);
void (GL_APIENTRY *qglDisableVertexAttribArray)(GLuint index);
void (GL_APIENTRY *qglDrawArrays)(GLenum mode, GLint first, GLsizei count);
void (GL_APIENTRY *qglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
void (GL_APIENTRY *qglEnable)(GLenum cap);
void (GL_APIENTRY *qglEnableVertexAttribArray)(GLuint index);
void (GL_APIENTRY *qglGenBuffers)(GLsizei n, GLuint* buffers);
void (GL_APIENTRY *qglGenTextures)(GLsizei n, GLuint* textures);
GLenum(GL_APIENTRY *qglGetError)(void);
void (GL_APIENTRY *qglGetProgramiv)(GLuint program, GLenum pname, GLint* params);
void (GL_APIENTRY *qglGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
void (GL_APIENTRY *qglGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
void (GL_APIENTRY *qglGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
const GLubyte* (GL_APIENTRY *qglGetString)(GLenum name);
GLint(GL_APIENTRY *qglGetUniformLocation)(GLuint program, const GLchar* name);
GLboolean(GL_APIENTRY *qglIsBuffer)(GLuint texture);
GLboolean(GL_APIENTRY *qglIsTexture)(GLuint buffer);
void (GL_APIENTRY *qglLinkProgram)(GLuint program);
void (GL_APIENTRY *qglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
void (GL_APIENTRY *qglScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
void (GL_APIENTRY *qglShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
void (GL_APIENTRY *qglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
void (GL_APIENTRY *qglTexParameteri)(GLenum target, GLenum pname, GLint param);
void (GL_APIENTRY *qglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
void (GL_APIENTRY *qglUniform1i)(GLint location, GLint x);
void (GL_APIENTRY *qglUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
void (GL_APIENTRY *qglUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
void (GL_APIENTRY *qglUseProgram)(GLuint program);
void (GL_APIENTRY *qglVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
void (GL_APIENTRY *qglViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

static SDL_Window *window;
static SDL_GLContext GLcontext;

int GLimp_Init(void *hinstance, void *hWnd)
{
    //<note.cb may pick up d3dcompiler_47.dll from somewhere other than working dir
    SDL_SetHint(SDL_HINT_VIDEO_WIN_D3DCOMPILER, "d3dcompiler_47.dll");
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

    //<note.cb must be set before SDL_CreateWindow on Windows
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow("Quake 2", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, desiredWidth, desiredHeight, flags);
    if(!window)
    {
        ri.Con_Printf(PRINT_ALL, "(%s) SDL_CreateWindow: %s\n", SDL_GetCurrentVideoDriver(), SDL_GetError());
        return rserr_unknown;
    }

    GLcontext = SDL_GL_CreateContext(window);
    if(!GLcontext)
    {
        ri.Con_Printf(PRINT_ALL, "(%s) SDL_GL_CreateContext: %s\n", SDL_GetCurrentVideoDriver(), SDL_GetError());
        SDL_DestroyWindow(window);
        window = NULL;
        return rserr_unknown;
    }

    // QGL
    qglActiveTexture = SDL_GL_GetProcAddress("glActiveTexture");
    qglAttachShader = SDL_GL_GetProcAddress("glAttachShader");
    qglBindBuffer = SDL_GL_GetProcAddress("glBindBuffer");
    qglBindAttribLocation = SDL_GL_GetProcAddress("glBindAttribLocation");
    qglBindTexture = SDL_GL_GetProcAddress("glBindTexture");
    qglBlendFunc = SDL_GL_GetProcAddress("glBlendFunc");
    qglBufferData = SDL_GL_GetProcAddress("glBufferData");
    qglClear = SDL_GL_GetProcAddress("glClear");
    qglClearColor = SDL_GL_GetProcAddress("glClearColor");
    qglCompileShader = SDL_GL_GetProcAddress("glCompileShader");
    qglCreateProgram = SDL_GL_GetProcAddress("glCreateProgram");
    qglCreateShader = SDL_GL_GetProcAddress("glCreateShader");
    qglCullFace = SDL_GL_GetProcAddress("glCullFace");
    qglDeleteBuffers = SDL_GL_GetProcAddress("glDeleteBuffers");
    qglDeleteProgram = SDL_GL_GetProcAddress("glDeleteProgram");
    qglDeleteShader = SDL_GL_GetProcAddress("glDeleteShader");
    qglDeleteTextures = SDL_GL_GetProcAddress("glDeleteTextures");
    qglDepthFunc = SDL_GL_GetProcAddress("glDepthFunc");
    qglDepthMask = SDL_GL_GetProcAddress("glDepthMask");
    qglDepthRangef = SDL_GL_GetProcAddress("glDepthRangef");
    qglDisable = SDL_GL_GetProcAddress("glDisable");
    qglDisableVertexAttribArray = SDL_GL_GetProcAddress("glDisableVertexAttribArray");
    qglDrawArrays = SDL_GL_GetProcAddress("glDrawArrays");
    qglDrawElements = SDL_GL_GetProcAddress("glDrawElements");
    qglEnable = SDL_GL_GetProcAddress("glEnable");
    qglEnableVertexAttribArray = SDL_GL_GetProcAddress("glEnableVertexAttribArray");
    qglGenBuffers = SDL_GL_GetProcAddress("glGenBuffers");
    qglGenTextures = SDL_GL_GetProcAddress("glGenTextures");
    qglGetError = SDL_GL_GetProcAddress("glGetError");
    qglGetProgramiv = SDL_GL_GetProcAddress("glGetProgramiv");
    qglGetProgramInfoLog = SDL_GL_GetProcAddress("glGetProgramInfoLog");
    qglGetShaderiv = SDL_GL_GetProcAddress("glGetShaderiv");
    qglGetShaderInfoLog = SDL_GL_GetProcAddress("glGetShaderInfoLog");
    qglGetString = SDL_GL_GetProcAddress("glGetString");
    qglGetUniformLocation = SDL_GL_GetProcAddress("glGetUniformLocation");
    qglIsBuffer = SDL_GL_GetProcAddress("glIsBuffer");
    qglIsTexture = SDL_GL_GetProcAddress("glIsTexture");
    qglLinkProgram = SDL_GL_GetProcAddress("glLinkProgram");
    qglReadPixels = SDL_GL_GetProcAddress("glReadPixels");
    qglScissor = SDL_GL_GetProcAddress("glScissor");
    qglShaderSource = SDL_GL_GetProcAddress("glShaderSource");
    qglTexImage2D = SDL_GL_GetProcAddress("glTexImage2D");
    qglTexParameteri = SDL_GL_GetProcAddress("glTexParameteri");
    qglTexSubImage2D = SDL_GL_GetProcAddress("glTexSubImage2D");
    qglUniform1i = SDL_GL_GetProcAddress("glUniform1i");
    qglUniform4f = SDL_GL_GetProcAddress("glUniform4f");
    qglUniformMatrix4fv = SDL_GL_GetProcAddress("glUniformMatrix4fv");
    qglUseProgram = SDL_GL_GetProcAddress("glUseProgram");
    qglVertexAttribPointer = SDL_GL_GetProcAddress("glVertexAttribPointer");
    qglViewport = SDL_GL_GetProcAddress("glViewport");

    SDL_GetWindowSize(window, &winWidth, &winHeight);

    ri.Vid_NewWindow(desiredWidth, desiredHeight);

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
