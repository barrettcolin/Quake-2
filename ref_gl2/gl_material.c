#include "gl_local.h"

// Material array
typedef struct material_s
{
    materialdesc_t desc;

	// GL state
    qboolean enable_client_state_vertex_array;
    qboolean enable_client_state_texture_coord_array;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;
} material_t;

#define MAX_MATERIALS 16

static material_t s_materials[MAX_MATERIALS];
static int s_num_materials;

static material_t *s_current_material;

static material_t s_default_material;

//<todo needed for ff pipeline glue
material_t *g_default_material = &s_default_material;

static int MaterialDesc_Compare(materialdesc_t const *a, materialdesc_t const *b)
{
    if(a->type != b->type)
        return 1;

    if(a->blend != b->blend)
        return 1;

    return 0;
}

static void Material_Destroy(material_t *mat)
{
    if(mat->program)
    {
        glDeleteProgram(mat->program);
        mat->program = 0;
    }

    if(mat->fragment_shader)
    {
        glDeleteShader(mat->fragment_shader);
        mat->fragment_shader = 0;
    }

    if(mat->vertex_shader)
    {
        glDeleteShader(mat->vertex_shader);
        mat->vertex_shader = 0;
    }
}

static GLuint Material_LoadShader(GLenum type, char const *pathname, char const *defines)
{
    GLuint shader;

    for(;;)
    {
        GLint compiled;
        GLchar const *strs[2] = { defines };
        GLint str_lens[2] = { strlen(defines) };

        shader = glCreateShader(type);
        if (shader == 0)
            break;

        str_lens[1] = ri.FS_LoadFile((char *)pathname, (void**)&strs[1]);
        if(!strs[1])
            break;

        // Load the shader source
        glShaderSource(shader, 2, strs, str_lens);

        ri.FS_FreeFile((void *)strs[1]);

        // Compile the shader
        glCompileShader(shader);

        // Check the compile status
        glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

        if(compiled == GL_FALSE)
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

            break;
        }

        return shader;
    }

    if(shader)
    {
        glDeleteShader(shader);
        shader = 0;
    }

    return shader;
}

static int Material_CreateProgram(material_t *mat, char const *pathname, char const *defines)
{
    char shader_name_buf[MAX_QPATH];

    snprintf(shader_name_buf, sizeof(shader_name_buf), "%s_vs.glsl", pathname);
    mat->vertex_shader = Material_LoadShader(GL_VERTEX_SHADER, shader_name_buf, defines);
    if(mat->vertex_shader == 0)
        return -1;

    snprintf(shader_name_buf, sizeof(shader_name_buf), "%s_fs.glsl", pathname);
    mat->fragment_shader = Material_LoadShader(GL_FRAGMENT_SHADER, shader_name_buf, defines);
    if(mat->fragment_shader == 0)
        return -1;

    mat->program = glCreateProgram();
    if(mat->program == 0)
        return -1;

    glAttachShader(mat->program, mat->vertex_shader);
    glAttachShader(mat->program, mat->fragment_shader);
    return 0;
}

static int Material_LinkProgram(material_t *mat)
{
    GLint linked;

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

        return -1;
    }

    return 0;
}

static void MaterialUnlit_Create(material_t *mat)
{
    for(;;)
    {
        char const *defines;

        switch(mat->desc.blend)
        {
        case mb_alpha_test_66:
            defines = "#define ALPHA_TEST_66\n";
            break;

        default:
            defines = "";
            break;
        }

        if(Material_CreateProgram(mat, "ref_gl2/unlit", defines) != 0)
            break;

        //<todo Bind attributes here

        if(Material_LinkProgram(mat) != 0)
            break;

        glUseProgram(mat->program);
        glUniform1i(glGetUniformLocation(mat->program, "diffuseTex"), 0);

        mat->enable_client_state_vertex_array = true;
        mat->enable_client_state_texture_coord_array = true;
        return;
    }

    Material_Destroy(mat);
    ri.Sys_Error(ERR_DROP, "MaterialUnlit_Create");
}

void Material_Init()
{
    // init s_default_material (program 0, default state)

    s_current_material = &s_default_material;
}

void Material_Shutdown()
{
    int i;
    material_t *mat;

    // Materials
    for(i = 0, mat = s_materials; i < s_num_materials; ++i, ++mat)
    {
        Material_Destroy(mat);
    }
    s_num_materials = 0;
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
    case mt_unlit:
        MaterialUnlit_Create(mat);
        break;
    }

    return mat;
}

void Material_SetCurrent(material_t *mat)
{
    if(s_current_material != mat)
    {
        if(s_current_material->enable_client_state_vertex_array != mat->enable_client_state_vertex_array)
        {
            if(mat->enable_client_state_vertex_array)
            {
                glEnableClientState(GL_VERTEX_ARRAY);
            }
            else
            {
                glDisableClientState(GL_VERTEX_ARRAY);
            }
        }

        if(s_current_material->enable_client_state_texture_coord_array != mat->enable_client_state_texture_coord_array)
        {
            if(mat->enable_client_state_texture_coord_array)
            {
                glEnableClientState(GL_TEXTURE_COORD_ARRAY);
            }
            else
            {
                glDisableClientState(GL_TEXTURE_COORD_ARRAY);
            }
        }

        glUseProgram(mat->program);

        s_current_material = mat;
    }
}

void Material_Render(material_t *mat, void const *data, GLuint textures[num_texture_units])
{
    Material_SetCurrent(mat);
}
