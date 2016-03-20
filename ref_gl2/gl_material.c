#include "gl_local.h"

// Shader array
typedef struct shader_s
{
    char pathname[MAX_QPATH];
    GLenum type;
    GLuint shader;
} shader_t;

#define MAX_SHADERS 2

static shader_t s_shaders[MAX_SHADERS];
static int s_num_shaders;

// Material array
#define MAX_MATERIALS 1

static material_t s_materials[MAX_MATERIALS];
static int s_num_materials;

material_t g_default_material;

// Global materials
material_t *g_generic_material;

static int MaterialDesc_Compare(materialdesc_t const *a, materialdesc_t const *b)
{
    if(a->type != b->type)
        return 1;

    return 0;
}

static GLuint Shader_Load(const char *pathname, GLenum type)
{
    const GLchar *src;
    GLuint shader;

    for(;;)
    {
        GLint compiled;
        GLint src_len = ri.FS_LoadFile((char *)pathname, (void**)&src);
        if(!src)
            break;

        shader = glCreateShader(type);
        if (shader == 0)
            break;

        // Load the shader source
        glShaderSource(shader, 1, &src, &src_len);

        // Compile the shader
        glCompileShader(shader);

        // Check the compile status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if (compiled == GL_FALSE)
        {
            GLint info_len = 0;
            glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);

            if (info_len > 1)
            {
                char* info_log = malloc(sizeof(char) * info_len);
                glGetShaderInfoLog(shader, info_len, NULL, info_log);
                ri.Con_Printf(PRINT_DEVELOPER, "Error compiling shader:\n%s\n", info_log);
                free(info_log);
            }

            glDeleteShader(shader);
            shader = 0;
        }

        break;
    }

    if(src)
        ri.FS_FreeFile((void *)src);

    if(shader == 0)
        ri.Sys_Error(ERR_DROP, "Failed to load %s", pathname);

    return shader;
}

static shader_t *Shader_Find(const char *pathname, GLenum type)
{
    int i;
    shader_t *shd;

    for(i = 0, shd = s_shaders; i < s_num_shaders; ++i, ++shd)
    {
        if(shd->type != type)
            continue;

        if(strncmp(shd->pathname, pathname, sizeof(shd->pathname)) == 0)
            return shd;
    }

    if(i == s_num_shaders)
    {
        if(s_num_shaders == MAX_SHADERS)
            ri.Sys_Error(ERR_DROP, "MAX_SHADERS");

        s_num_shaders++;
    }

    shd = &s_shaders[i];
    shd->shader = Shader_Load(pathname, type);

    return shd;
}

static void Shader_Destroy(shader_t *shd)
{
    if(shd->shader)
    {
        glDeleteShader(shd->shader);
        shd->shader = 0;
    }
}

static void Material_Destroy(material_t *mat)
{
    if(mat->program)
    {
        glDeleteProgram(mat->program);
        mat->program = 0;
    }
}

static void MaterialGeneric_Load(material_t *mat)
{
    for(;;)
    {
        shader_t *vs, *fs;
        GLint linked;

        vs = Shader_Find("ref_gl2/generic_vs.glsl", GL_VERTEX_SHADER);
        if(vs == NULL)
            break;

        fs = Shader_Find("ref_gl2/generic_fs.glsl", GL_FRAGMENT_SHADER);
        if(fs == NULL)
            break;

        //<todo move this to its own function
        mat->program = glCreateProgram();
        if(mat->program == 0)
            break;

        glAttachShader(mat->program, vs->shader);
        glAttachShader(mat->program, fs->shader);

        // Link program
        glLinkProgram(mat->program);

        // Check the link status
        glGetProgramiv(mat->program, GL_LINK_STATUS, &linked);

        if (linked == GL_FALSE)
        {
            GLint info_len = 0;
            glGetProgramiv(mat->program, GL_INFO_LOG_LENGTH, &info_len);

            if (info_len > 1)
            {
                char* info_log = malloc(sizeof(char) * info_len);
                glGetProgramInfoLog(mat->program, info_len, NULL, info_log);
                ri.Con_Printf(PRINT_DEVELOPER, "Error linking program:\n%s\n", info_log);
                free(info_log);
            }

            glDeleteProgram(mat->program);
            mat->program = 0;
            break;
        }

        glUseProgram(mat->program);
        glUniform1i(glGetUniformLocation(mat->program, "diffuseTex"), 0);
        glUseProgram(0);
        break;
    }

    if(mat->program == 0)
        ri.Sys_Error(ERR_DROP, "MaterialGeneric_Load");
}

void Material_Init()
{
    // init mat_default (program 0, default state)

    // mat_generic
    {
        materialdesc_t desc;
        {
            desc.type = mt_generic;
        }
        g_generic_material = Material_Find(&desc);
    }
}

void Material_Shutdown()
{
    int i;
    material_t *mat;
    shader_t *shd;

    // Materials
    for(i = 0, mat = s_materials; i < s_num_materials; ++i, ++mat)
    {
        Material_Destroy(mat);
    }
    s_num_materials = 0;

    // Shaders
    for(i = 0, shd = s_shaders; i < s_num_shaders; ++i, ++shd)
    {
        Shader_Destroy(shd);
    }
    s_num_shaders = 0;

    g_generic_material = NULL;
}

material_t *Material_Find(materialdesc_t const* desc)
{
    int i;
    material_t *mat;

    for(i = 0, mat = s_materials; i < s_num_materials; ++i, ++mat)
    {
        if(MaterialDesc_Compare(&mat->desc, desc) == 0)
            return mat;

        if(mat->program == 0)
            break;
    }

    if(i == s_num_materials)
    {
        if(s_num_materials == MAX_MATERIALS)
            ri.Sys_Error(ERR_DROP, "MAX_MATERIALS");

        s_num_materials++;
    }

    mat = &s_materials[i];
    mat->desc = *desc;

    switch(desc->type)
    {
    case mt_generic:
        MaterialGeneric_Load(mat);
        break;
    }

    return mat;
}

void Material_SetCurrent(const material_t *mat)
{
    if(gl_state.current_material != mat)
    {
        glUseProgram(mat->program);

        gl_state.current_material = mat;
    }
}

void Material_Render(material_t const* mat, int num_verts, vertex_t const *verts, GLuint textures[num_texture_units])
{
    Material_SetCurrent(mat);

    GL_SelectTexture(GL_TEXTURE0);
    GL_Bind(textures[tu_diffuse]);

    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glVertexPointer(3, GL_FLOAT, sizeof(vertex_t), &verts[0].x);
    glTexCoordPointer(2, GL_FLOAT, sizeof(vertex_t), &verts[0].s);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, num_verts);

    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glDisableClientState(GL_VERTEX_ARRAY);
}
