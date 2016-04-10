#include "gl_local.h"

static const int MAX_VERTEX_ATTRIBUTES = 3;

// Material array
typedef struct material_s
{
    materialdesc_t desc;

    GLuint num_vertex_attributes;

    // GL state
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint program;

    GLint clip_from_view_location;
    GLint view_from_world_location;
    GLint world_from_model_location;
    GLint diffuse_color_location;
} material_t;

#define MAX_MATERIALS 16

static material_t s_materials[MAX_MATERIALS];
static int s_num_materials;

static material_t *s_current_material;

static material_t s_default_material;

//<todo needed for ff pipeline glue
material_t *g_default_material = &s_default_material;

GLfloat const g_identity_matrix[16] =
{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1,
};

static int MaterialDesc_Compare(materialdesc_t const *a, materialdesc_t const *b)
{
    if(a->type != b->type)
        return 1;

    if(a->blend != b->blend)
        return 1;

    if(a->alpha_test_66 != b->alpha_test_66)
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
        char const *defines = mat->desc.alpha_test_66 ? "#define ALPHA_TEST_66\n" : "";

        if(Material_CreateProgram(mat, "ref_gl2/unlit", defines) != 0)
            break;

        mat->num_vertex_attributes = 2;
        glBindAttribLocation(mat->program, 0, "a_vPosition");
        glBindAttribLocation(mat->program, 1, "a_vTexCoord");

        if(Material_LinkProgram(mat) != 0)
            break;

        glUseProgram(mat->program);
        mat->clip_from_view_location = glGetUniformLocation(mat->program, "mClipFromView");
        mat->view_from_world_location = glGetUniformLocation(mat->program, "mViewFromWorld");
        mat->world_from_model_location = glGetUniformLocation(mat->program, "mWorldFromModel");
        mat->diffuse_color_location = glGetUniformLocation(mat->program, "vDiffuseColor");
        glUniform1i(glGetUniformLocation(mat->program, "sDiffuse"), 0);
        glUseProgram(0);
        return;
    }

    Material_Destroy(mat);
    ri.Sys_Error(ERR_DROP, "MaterialUnlit_Create");
}

static void MaterialLightmapped_Create(material_t *mat)
{
    for(;;)
    {
        char const *defines = "";

        if(Material_CreateProgram(mat, "ref_gl2/lightmapped", defines) != 0)
            break;

        mat->num_vertex_attributes = 3;
        glBindAttribLocation(mat->program, 0, "a_vPosition");
        glBindAttribLocation(mat->program, 1, "a_vTexCoord0");
        glBindAttribLocation(mat->program, 2, "a_vTexCoord1");

        if(Material_LinkProgram(mat) != 0)
            break;

        glUseProgram(mat->program);
        mat->clip_from_view_location = glGetUniformLocation(mat->program, "mClipFromView");
        mat->view_from_world_location = glGetUniformLocation(mat->program, "mViewFromWorld");
        mat->world_from_model_location = glGetUniformLocation(mat->program, "mWorldFromModel");
        mat->diffuse_color_location = glGetUniformLocation(mat->program, "vDiffuseColor");
        glUniform1i(glGetUniformLocation(mat->program, "sDiffuse"), 0);
        glUniform1i(glGetUniformLocation(mat->program, "sLightmap"), 1);
        glUseProgram(0);
        return;
    }

    Material_Destroy(mat);
    ri.Sys_Error(ERR_DROP, "MaterialLightmapped_Create");
}

static void MaterialVertexlit_Create(material_t *mat)
{
    for(;;)
    {
        char const *defines = "";

        if(Material_CreateProgram(mat, "ref_gl2/vertexlit", defines) != 0)
            break;

        mat->num_vertex_attributes = 3;
        glBindAttribLocation(mat->program, 0, "a_vPosition");
        glBindAttribLocation(mat->program, 1, "a_vColor");
        glBindAttribLocation(mat->program, 2, "a_vTexCoord");

        if(Material_LinkProgram(mat) != 0)
            break;

        glUseProgram(mat->program);
        mat->clip_from_view_location = glGetUniformLocation(mat->program, "mClipFromView");
        mat->view_from_world_location = glGetUniformLocation(mat->program, "mViewFromWorld");
        mat->world_from_model_location = glGetUniformLocation(mat->program, "mWorldFromModel");
        glUniform1i(glGetUniformLocation(mat->program, "sDiffuse"), 0);
        glUseProgram(0);
        return;
    }

    Material_Destroy(mat);
    ri.Sys_Error(ERR_DROP, "MaterialVertexlit_Create");
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

    case mt_lightmapped:
        MaterialLightmapped_Create(mat);
        break;

    case mt_vertexlit:
        MaterialVertexlit_Create(mat);
        break;
    }

    return mat;
}

void Material_SetCurrent(material_t *mat)
{
    GLuint i;

    if(s_current_material != mat)
    {
        for(i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
        {
            if(i < mat->num_vertex_attributes)
            {
                glEnableVertexAttribArray(i);
            }
            else
            {
                glDisableVertexAttribArray(i);
            }
        }

        if(s_current_material->desc.blend != mat->desc.blend)
        {
            if(mat->desc.blend != mb_opaque)
            {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
            else
            {
                glDisable(GL_BLEND);
            }
        }

        glUseProgram(mat->program);

        s_current_material = mat;
    }
}

void Material_SetClipFromView(material_id mat, GLfloat const clip_from_view[16])
{
    glUniformMatrix4fv(mat->clip_from_view_location, 1, GL_FALSE, clip_from_view);
}

void Material_SetViewFromWorld(material_id mat, GLfloat const view_from_world[16])
{
    glUniformMatrix4fv(mat->view_from_world_location, 1, GL_FALSE, view_from_world);
}

void Material_SetWorldFromModel(material_id mat, GLfloat const world_from_model[16])
{
    glUniformMatrix4fv(mat->world_from_model_location, 1, GL_FALSE, world_from_model);
}

void Material_SetDiffuseColor(material_id mat, GLfloat r, GLfloat g, GLfloat b, GLfloat a)
{
    glUniform4f(mat->diffuse_color_location, r, g, b, a);
}

void Material_Render(material_t *mat, void const *data, GLuint textures[num_texture_units])
{
    Material_SetCurrent(mat);
}

void Matrix_Perspective(GLfloat camera_separation, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat matrix_out[16])
{
    GLfloat xmin, xmax, ymin, ymax, near2, zDiff;
    ymax = zNear * tan(fovy * M_PI / 360.0);
    ymin = -ymax;

    xmin = ymin * aspect;
    xmax = ymax * aspect;

    xmin += -( 2.0f * camera_separation ) / zNear;
    xmax += -( 2.0f * camera_separation ) / zNear;

    near2 = 2.0f * zNear;
    zDiff = zFar - zNear;

    // col0
    matrix_out[0] = near2 / (xmax - xmin);
    matrix_out[1] = 0;
    matrix_out[2] = 0;
    matrix_out[3] = 0;

    // col1
    matrix_out[4] = 0;
    matrix_out[5] = near2 / (ymax - ymin);
    matrix_out[6] = 0;
    matrix_out[7] = 0;

    // col2
    matrix_out[8] = 0;
    matrix_out[9] = 0;
    matrix_out[10] = -(zFar + zNear) / zDiff;
    matrix_out[11] = -1;

    // col3
    matrix_out[12] = 0;
    matrix_out[13] = 0;
    matrix_out[14] = -(2.0f * zFar * zNear) / zDiff;
    matrix_out[15] = 0;
}

void Matrix_Orthographic(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal, GLfloat matrix_out[16])
{
    GLfloat const right_minus_left = right - left;
    GLfloat const top_minus_bottom = top - bottom;
    GLfloat const far_minus_near = farVal - nearVal;

    // col0
    matrix_out[0] = 2.0f / right_minus_left;
    matrix_out[1] = 0;
    matrix_out[2] = 0;
    matrix_out[3] = 0;

    // col1
    matrix_out[4] = 0;
    matrix_out[5] = 2.0f / top_minus_bottom;
    matrix_out[6] = 0;
    matrix_out[7] = 0;

    // col2
    matrix_out[8] = 0;
    matrix_out[9] = 0;
    matrix_out[10] = -2.0f / far_minus_near;
    matrix_out[11] = 0;

    // col3
    matrix_out[12] = -(right + left) / right_minus_left;
    matrix_out[13] = -(top + bottom) / top_minus_bottom;
    matrix_out[14] = -(farVal + nearVal) / far_minus_near;
    matrix_out[15] = 1;
}

void Matrix_FromAnglesOrigin(vec3_t const angles, vec3_t const origin, GLfloat matrix_out[16])
{
    float const DEG_TO_RAD = M_PI / 180.0f;

    float const sy = sin(angles[YAW] * DEG_TO_RAD);
    float const cy = cos(angles[YAW] * DEG_TO_RAD);
    float const sp = sin(angles[PITCH] * DEG_TO_RAD);
    float const cp = cos(angles[PITCH] * DEG_TO_RAD);
    float const sr = sin(angles[ROLL] * DEG_TO_RAD);
    float const cr = cos(angles[ROLL] * DEG_TO_RAD);

    float const cycp = cy * cp;
    float const sycp = sy * cp;
    float const cysr = cy * sr;
    float const sysr = sy * sr;
    float const spcr = sp * cr;

    // col0
    matrix_out[0] = cycp;
    matrix_out[1] = sycp;
    matrix_out[2] = -sp;
    matrix_out[3] = 0;

    // col1
    matrix_out[4] = cysr * sp - sy * cr;
    matrix_out[5] = cy * cr + sysr * sp;
    matrix_out[6] = cp * sr;
    matrix_out[7] = 0;

    // col2
    matrix_out[8] = sysr + cy * spcr;
    matrix_out[9] = sy * spcr - cysr;
    matrix_out[10] = cp * cr;
    matrix_out[11] = 0;

    // col3
    matrix_out[12] = origin[0];
    matrix_out[13] = origin[1];
    matrix_out[14] = origin[2];
    matrix_out[15] = 1;
}

void Matrix_InverseFromAnglesOrigin(vec3_t const angles, vec3_t const origin, GLfloat matrix_out[16])
{
    float const DEG_TO_RAD = M_PI / 180.0f;

    float const sy = sin(angles[YAW] * DEG_TO_RAD);
    float const cy = cos(angles[YAW] * DEG_TO_RAD);
    float const sp = sin(angles[PITCH] * DEG_TO_RAD);
    float const cp = cos(angles[PITCH] * DEG_TO_RAD);
    float const sr = sin(angles[ROLL] * DEG_TO_RAD);
    float const cr = cos(angles[ROLL] * DEG_TO_RAD);

    float const cycp = cy * cp;
    float const sycp = sy * cp;
    float const cysr = cy * sr;
    float const sysr = sy * sr;
    float const spcr = sp * cr;

    // col0
    matrix_out[0] = cycp;
    matrix_out[1] = cysr * sp - sy * cr;
    matrix_out[2] = sysr + cy * spcr;
    matrix_out[3] = 0;

    // col1
    matrix_out[4] = sycp;
    matrix_out[5] = cy * cr + sysr * sp;
    matrix_out[6] = sy * spcr - cysr;
    matrix_out[7] = 0;

    // col2
    matrix_out[8] = -sp;
    matrix_out[9] = cp * sr;
    matrix_out[10] = cp * cr;
    matrix_out[11] = 0;

    // col3
    matrix_out[12] = -matrix_out[0] * origin[0] - matrix_out[4] * origin[1] - matrix_out[8] * origin[2];
    matrix_out[13] = -matrix_out[1] * origin[0] - matrix_out[5] * origin[1] - matrix_out[9] * origin[2];
    matrix_out[14] = -matrix_out[2] * origin[0] - matrix_out[6] * origin[1] - matrix_out[10] * origin[2];
    matrix_out[15] = 1;
}

void Matrix_FromAxisAngleOrigin(vec3_t const axis, float angle, vec3_t const origin, GLfloat matrix_out[16])
{
    float const DEG_TO_RAD = M_PI / 180.0f;

    float const c = cos(angle * DEG_TO_RAD);
    float const c1 = 1.0f - c;
    float const s = sin(angle * DEG_TO_RAD);
    {
        vec3_t normalized_axis; VectorCopy(axis, normalized_axis);
        VectorNormalize(normalized_axis);

        float const x = normalized_axis[0];
        float const y = normalized_axis[1];
        float const z = normalized_axis[2];

        // col0
        matrix_out[0] = x * x * c1 + c;
        matrix_out[1] = y * x * c1 + z * s;
        matrix_out[2] = x * z * c1 - y * s;
        matrix_out[3] = 0;

        // col1
        matrix_out[4] = x * y * c1 - z * s;
        matrix_out[5] = y * y * c1 + c;
        matrix_out[6] = y * z * c1 + x * s;
        matrix_out[7] = 0;

        // col2
        matrix_out[8] = x * z * c1 + y * s;
        matrix_out[9] = y * z * c1 - x * s;
        matrix_out[10] = z * z * c1 + c;
        matrix_out[11] = 0;

        // col3
        matrix_out[12] = origin[0];
        matrix_out[13] = origin[1];
        matrix_out[14] = origin[2];
        matrix_out[15] = 1;
    }
}
