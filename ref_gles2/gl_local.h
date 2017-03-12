/*
Copyright (C) 1997-2001 Id Software, Inc.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
// disable data conversion warnings

#if 0
#pragma warning(disable : 4244)     // MIPS
#pragma warning(disable : 4136)     // X86
#pragma warning(disable : 4051)     // ALPHA
#endif

#include <ctype.h>
#include <math.h>
#include <stdio.h>

#include "SDL_opengles2.h"

// QGL
extern void (GL_APIENTRY *qglActiveTexture)(GLenum texture);
extern void (GL_APIENTRY *qglAttachShader)(GLuint program, GLuint shader);
extern void (GL_APIENTRY *qglBindBuffer)(GLenum target, GLuint buffer);
extern void (GL_APIENTRY *qglBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
extern void (GL_APIENTRY *qglBindTexture)(GLenum target, GLuint texture);
extern void (GL_APIENTRY *qglBlendFunc)(GLenum sfactor, GLenum dfactor);
extern void (GL_APIENTRY *qglBufferData)(GLenum target, GLsizeiptr size, const GLvoid* data, GLenum usage);
extern void (GL_APIENTRY *qglClear)(GLbitfield mask);
extern void (GL_APIENTRY *qglClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void (GL_APIENTRY *qglCompileShader)(GLuint shader);
extern GLuint (GL_APIENTRY *qglCreateProgram)(void);
extern GLuint (GL_APIENTRY *qglCreateShader)(GLenum type);
extern void (GL_APIENTRY *qglCullFace)(GLenum mode);
extern void (GL_APIENTRY *qglDeleteBuffers)(GLsizei n, const GLuint* buffers);
extern void (GL_APIENTRY *qglDeleteProgram)(GLuint program);
extern void (GL_APIENTRY *qglDeleteShader)(GLuint shader);
extern void (GL_APIENTRY *qglDeleteTextures)(GLsizei n, const GLuint* textures);
extern void (GL_APIENTRY *qglDepthFunc)(GLenum func);
extern void (GL_APIENTRY *qglDepthMask)(GLboolean flag);
extern void (GL_APIENTRY *qglDepthRangef)(GLclampf zNear, GLclampf zFar);
extern void (GL_APIENTRY *qglDisable)(GLenum cap);
extern void (GL_APIENTRY *qglDisableVertexAttribArray)(GLuint index);
extern void (GL_APIENTRY *qglDrawArrays)(GLenum mode, GLint first, GLsizei count);
extern void (GL_APIENTRY *qglDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
extern void (GL_APIENTRY *qglEnable)(GLenum cap);
extern void (GL_APIENTRY *qglEnableVertexAttribArray)(GLuint index);
extern void (GL_APIENTRY *qglGenBuffers)(GLsizei n, GLuint* buffers);
extern GLenum (GL_APIENTRY *qglGetError)(void);
extern void (GL_APIENTRY *qglGetProgramiv)(GLuint program, GLenum pname, GLint* params);
extern void (GL_APIENTRY *qglGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern void (GL_APIENTRY *qglGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
extern void (GL_APIENTRY *qglGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
extern const GLubyte* (GL_APIENTRY *qglGetString)(GLenum name);
extern GLint (GL_APIENTRY *qglGetUniformLocation)(GLuint program, const GLchar* name);
extern void (GL_APIENTRY *qglLinkProgram)(GLuint program);
extern void (GL_APIENTRY *qglReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
extern void (GL_APIENTRY *qglScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
extern void (GL_APIENTRY *qglShaderSource)(GLuint shader, GLsizei count, const GLchar* const* string, const GLint* length);
extern void (GL_APIENTRY *qglTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid* pixels);
extern void (GL_APIENTRY *qglTexParameteri)(GLenum target, GLenum pname, GLint param);
extern void (GL_APIENTRY *qglTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
extern void (GL_APIENTRY *qglUniform1i)(GLint location, GLint x);
extern void (GL_APIENTRY *qglUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void (GL_APIENTRY *qglUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
extern void (GL_APIENTRY *qglUseProgram)(GLuint program);
extern void (GL_APIENTRY *qglVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
extern void (GL_APIENTRY *qglViewport)(GLint x, GLint y, GLsizei width, GLsizei height);

#include "../client/ref.h"


#define	REF_VERSION	"GLES2 0.01"

// up / down
#define	PITCH	0

// left / right
#define	YAW		1

// fall over
#define	ROLL	2


typedef struct
{
	unsigned		width, height;			// coordinates from main game
} viddef_t;

extern	viddef_t	vid;


/*

  skins will be outline flood filled and mip mapped
  pics and sprites with alpha will be outline flood filled
  pic won't be mip mapped

  model skin
  sprite frame
  wall texture
  pic

*/

typedef enum 
{
	it_skin,
	it_sprite,
	it_wall,
	it_pic,
	it_sky
} imagetype_t;

typedef struct image_s
{
	char	name[MAX_QPATH];			// game path, including extension
	imagetype_t	type;
	int		width, height;				// source image
	int		upload_width, upload_height;	// after power of two and picmip
	int		registration_sequence;		// 0 = free
	struct msurface_s	*texturechain;	// for sort-by-texture world drawing
	int		texnum;						// gl texture binding
	float	sl, tl, sh, th;				// 0,0 - 1,1 unless part of the scrap
	qboolean	scrap;
	qboolean	has_alpha;

	qboolean paletted;
} image_t;

#define	TEXNUM_LIGHTMAPS	1024
#define	TEXNUM_SCRAPS		1152
#define	TEXNUM_IMAGES		1153

#define		MAX_GLTEXTURES	1024

//===================================================================

typedef enum
{
	rserr_ok,

	rserr_invalid_fullscreen,
	rserr_invalid_mode,

	rserr_unknown
} rserr_t;

#include "gl_model.h"

void GL_BeginRendering (int *x, int *y, int *width, int *height);
void GL_EndRendering (void);

void GL_SetDefaultState( void );
void GL_UpdateSwapInterval( void );

extern	float	gldepthmin, gldepthmax;

typedef struct
{
	float	x, y, z;
	float	s, t;
	float	r, g, b;
} glvert_t;


#define	MAX_LBM_HEIGHT		480

#define BACKFACE_EPSILON	0.01


//====================================================

extern	image_t		gltextures[MAX_GLTEXTURES];
extern	int			numgltextures;


extern	image_t		*r_notexture;
extern	image_t		*r_particletexture;
extern	entity_t	*currententity;
extern	model_t		*currentmodel;
extern	int			r_visframecount;
extern	int			r_framecount;
extern	cplane_t	frustum[4];
extern	int			c_brush_polys, c_alias_polys;


extern	int			gl_filter_min, gl_filter_max;

//
// view origin
//
extern	vec3_t	vup;
extern	vec3_t	vpn;
extern	vec3_t	vright;
extern	vec3_t	r_origin;

//
// screen size info
//
extern	refdef_t	r_newrefdef;
extern	int		r_viewcluster, r_viewcluster2, r_oldviewcluster, r_oldviewcluster2;

extern	cvar_t	*r_norefresh;
extern	cvar_t	*r_lefthand;
extern	cvar_t	*r_drawentities;
extern	cvar_t	*r_drawworld;
extern	cvar_t	*r_speeds;
extern	cvar_t	*r_fullbright;
extern	cvar_t	*r_novis;
extern	cvar_t	*r_nocull;
extern	cvar_t	*r_lerpmodels;

extern	cvar_t	*r_lightlevel;	// FIXME: This is a HACK to get the client's light level

extern cvar_t	*gl_vertex_arrays;

extern cvar_t	*gl_ext_swapinterval;
extern cvar_t	*gl_ext_palettedtexture;
extern cvar_t	*gl_ext_multitexture;
extern cvar_t	*gl_ext_pointparameters;
extern cvar_t	*gl_ext_compiled_vertex_array;

extern cvar_t	*gl_particle_min_size;
extern cvar_t	*gl_particle_max_size;
extern cvar_t	*gl_particle_size;
extern cvar_t	*gl_particle_att_a;
extern cvar_t	*gl_particle_att_b;
extern cvar_t	*gl_particle_att_c;

extern	cvar_t	*gl_bitdepth;
extern	cvar_t	*gl_mode;
extern	cvar_t	*gl_log;
extern	cvar_t	*gl_lightmap;
extern	cvar_t	*gl_shadows;
extern	cvar_t	*gl_dynamic;
extern  cvar_t  *gl_monolightmap;
extern	cvar_t	*gl_round_down;
extern	cvar_t	*gl_picmip;
extern	cvar_t	*gl_skymip;
extern	cvar_t	*gl_showtris;
extern	cvar_t	*gl_finish;
extern	cvar_t	*gl_ztrick;
extern	cvar_t	*gl_clear;
extern	cvar_t	*gl_cull;
extern	cvar_t	*gl_poly;
extern	cvar_t	*gl_texsort;
extern	cvar_t	*gl_polyblend;
extern	cvar_t	*gl_flashblend;
extern	cvar_t	*gl_lightmaptype;
extern	cvar_t	*gl_modulate;
extern	cvar_t	*gl_playermip;
extern	cvar_t	*gl_3dlabs_broken;
extern  cvar_t  *gl_driver;
extern	cvar_t	*gl_swapinterval;
extern	cvar_t	*gl_texturemode;
extern  cvar_t  *gl_saturatelighting;
extern  cvar_t  *gl_lockpvs;

extern	cvar_t	*vid_fullscreen;
extern	cvar_t	*vid_gamma;

extern	cvar_t		*intensity;

extern	int		c_visible_lightmaps;
extern	int		c_visible_textures;

void R_TranslatePlayerSkin (int playernum);
void GL_SelectTexture(GLenum texture);
void GL_Bind (int texnum);
void GL_MBind( GLenum target, int texnum );

void R_LightPoint (vec3_t p, vec3_t color);
void R_PushDlights (void);

//====================================================================

extern	model_t	*r_worldmodel;

extern	unsigned	d_8to24table[256];

extern	int		registration_sequence;


void V_AddBlend (float r, float g, float b, float a, float *v_blend);

qboolean 	R_Init( void *hinstance, void *hWnd );
void	R_Shutdown( void );

void R_RenderView (refdef_t *fd);
void GL_ScreenShot_f (void);
void R_DrawAliasModel (entity_t *e);
void R_DrawBrushModel (entity_t *e);
void R_DrawSpriteModel (entity_t *e);
void R_DrawBeam( entity_t *e );
void R_DrawWorld (void);
void R_RenderDlights (void);
void R_DrawAlphaSurfaces (void);
void R_RenderBrushPoly (msurface_t *fa);
void R_InitParticleTexture (void);
void Draw_InitLocal (void);
void GL_SubdivideSurface (msurface_t *fa);
qboolean R_CullBox (vec3_t mins, vec3_t maxs);
void R_RotateForEntity (entity_t *e);
void R_MarkLeaves (void);

glpoly_t *WaterWarpPolyVerts (glpoly_t *p);
void EmitWaterPolys (msurface_t *fa);
void R_AddSkySurface (msurface_t *fa);
void R_ClearSkyBox (void);
void R_DrawSkyBox (void);
void R_MarkLights (dlight_t *light, int bit, mnode_t *node);

#if 0
short LittleShort (short l);
short BigShort (short l);
int	LittleLong (int l);
float LittleFloat (float f);

char	*va(char *format, ...);
// does a varargs printf into a temp buffer
#endif

void COM_StripExtension (char *in, char *out);

void	Draw_GetPicSize (int *w, int *h, char *name);
void	Draw_Pic (int x, int y, char *name);
void	Draw_StretchPic (int x, int y, int w, int h, char *name);
void	Draw_Char (int x, int y, int c);
void	Draw_TileClear (int x, int y, int w, int h, char *name);
void	Draw_Fill (int x, int y, int w, int h, int c);
void	Draw_FadeScreen (void);
void	Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data);

void	R_BeginFrame( float camera_separation );
void	R_SwapBuffers( int );
void	R_SetPalette ( const unsigned char *palette);

int		Draw_GetPalette (void);

void GL_ResampleTexture (unsigned *in, int inwidth, int inheight, unsigned *out,  int outwidth, int outheight);

struct image_s *R_RegisterSkin (char *name);

void LoadPCX (char *filename, byte **pic, byte **palette, int *width, int *height);
image_t *GL_LoadPic (char *name, byte *pic, int width, int height, imagetype_t type, int bits);
image_t	*GL_FindImage (char *name, imagetype_t type);
void	GL_TextureMode( char *string );
void	GL_ImageList_f (void);

void	GL_SetTexturePalette( unsigned palette[256] );

void	GL_InitImages (void);
void	GL_ShutdownImages (void);

void	GL_FreeUnusedImages (void);

/*
** GL extension emulation functions
*/
void GL_DrawParticles( int n, const particle_t particles[], const unsigned colortable[768] );

/*
** GL config stuff
*/
#define GL_RENDERER_VOODOO		0x00000001
#define GL_RENDERER_VOODOO2   	0x00000002
#define GL_RENDERER_VOODOO_RUSH	0x00000004
#define GL_RENDERER_BANSHEE		0x00000008
#define		GL_RENDERER_3DFX		0x0000000F

#define GL_RENDERER_PCX1		0x00000010
#define GL_RENDERER_PCX2		0x00000020
#define GL_RENDERER_PMX			0x00000040
#define		GL_RENDERER_POWERVR		0x00000070

#define GL_RENDERER_PERMEDIA2	0x00000100
#define GL_RENDERER_GLINT_MX	0x00000200
#define GL_RENDERER_GLINT_TX	0x00000400
#define GL_RENDERER_3DLABS_MISC	0x00000800
#define		GL_RENDERER_3DLABS	0x00000F00

#define GL_RENDERER_REALIZM		0x00001000
#define GL_RENDERER_REALIZM2	0x00002000
#define		GL_RENDERER_INTERGRAPH	0x00003000

#define GL_RENDERER_3DPRO		0x00004000
#define GL_RENDERER_REAL3D		0x00008000
#define GL_RENDERER_RIVA128		0x00010000
#define GL_RENDERER_DYPIC		0x00020000

#define GL_RENDERER_V1000		0x00040000
#define GL_RENDERER_V2100		0x00080000
#define GL_RENDERER_V2200		0x00100000
#define		GL_RENDERER_RENDITION	0x001C0000

#define GL_RENDERER_O2          0x00100000
#define GL_RENDERER_IMPACT      0x00200000
#define GL_RENDERER_RE			0x00400000
#define GL_RENDERER_IR			0x00800000
#define		GL_RENDERER_SGI			0x00F00000

#define GL_RENDERER_MCD			0x01000000
#define GL_RENDERER_OTHER		0x80000000

typedef struct
{
	int         renderer;
	const char *renderer_string;
	const char *vendor_string;
	const char *version_string;
	const char *extensions_string;
} glconfig_t;

typedef struct
{
	float inverse_intensity;
	qboolean fullscreen;

	int     prev_mode;

	unsigned char *d_16to8table;

	int lightmap_textures;

	int	currenttextures[2];
	int currenttmu;

	float camera_separation;
	qboolean stereo_enabled;

    GLfloat clip_from_view[16];
    GLfloat view_from_world[16]; // for rendering world alpha surfaces after everything else
} glstate_t;

extern glconfig_t  gl_config;
extern glstate_t   gl_state;

/*
====================================================================

IMPORTED FUNCTIONS

====================================================================
*/

extern	refimport_t	ri;


/*
====================================================================

IMPLEMENTATION SPECIFIC FUNCTIONS

====================================================================
*/

void		GLimp_BeginFrame( float camera_separation );
void		GLimp_EndFrame( void );
int 		GLimp_Init( void *hinstance, void *hWnd );
void		GLimp_Shutdown( void );
int     	GLimp_SetMode( int *pwidth, int *pheight, int mode, qboolean fullscreen );
void		GLimp_AppActivate( qboolean active );
void		GLimp_EnableLogging( qboolean enable );
void		GLimp_LogNewFrame( void );

// Shader material
typedef enum
{
    mt_unlit,
    mt_lightmapped,
    mt_vertexlit
} materialtype_t;

typedef enum
{
    mb_zero,
    mb_one,
    mb_src_alpha,
    mb_one_minus_src_alpha,

    num_materialblend
} materialblend_t;

typedef struct materialdesc_s
{
    struct
    {
        unsigned int type : 4;
        unsigned int use_diffuse_color : 1;
        unsigned int use_alpha_test66 : 1;
        unsigned int : 2; // padding; todo- depth_mask (0 for transparent ents), cull_face (switched for lefthand)
        unsigned int src_blend : 4;
        unsigned int dst_blend : 4;
    } flags;
} materialdesc_t;

typedef struct material_s *material_id;

typedef enum
{
    tu_diffuse,

    num_texture_units
} textureunit_t;

void Material_Init();

void Material_Shutdown();

material_id Material_Find(materialdesc_t const *desc);

void Material_SetCurrent(material_id mat);

void Material_SetClipFromView(material_id mat, GLfloat const clip_from_view[16]);

void Material_SetViewFromWorld(material_id mat, GLfloat const view_from_world[16]);

void Material_SetWorldFromModel(material_id mat, GLfloat const world_from_model[16]);

void Material_SetDiffuseColor(material_id mat, GLfloat r, GLfloat g, GLfloat b, GLfloat a);

void Material_Render(material_id mat, void const *data, GLuint textures[num_texture_units]);

extern material_id g_lightmapped_material;
extern material_id g_lightmapped_alpha_material; // for RF_TRANSLUCENT brushmodels
extern material_id g_unlit_material; // for opaque SURF_DRAWTURB (same material as opaque Draw_*)
extern material_id g_unlit_alpha_material; // alpha surface, opaque SURF_DRAWTURB on RF_TRANSLUCENT brushmodels
extern material_id g_vertexlit_material; // for alias model entities
extern material_id g_vertexlit_alpha_material; // for RF_TRANSLUCENT alias model entities

//<todo reset state for ff pipeline
extern material_id g_default_material;

extern GLfloat const g_identity_matrix[16];

// calculate GL perspective projection
void Matrix_Perspective(GLfloat camera_separation, GLfloat fovy, GLfloat aspect, GLfloat zNear, GLfloat zFar, GLfloat matrix_out[16]);

// calculate GL orthographic projection
void Matrix_Orthographic(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat nearVal, GLfloat farVal, GLfloat matrix_out[16]);

// calculate T * Ry * Rp * Rr
void Matrix_FromAnglesOrigin(vec3_t const angles, vec3_t const origin, GLfloat matrix_out[16]);

// calculate inv(Rr) * inv(Rp) * inv(Ry) * inv(T)
void Matrix_InverseFromAnglesOrigin(vec3_t const angles, vec3_t const origin, GLfloat matrix_out[16]);

// calculate T * Ra
void Matrix_FromAxisAngleOrigin(vec3_t const axis, float angle, vec3_t const origin, GLfloat matrix_out[16]);

// VertexBuffer
GLuint VertexBuffer_Create();

void VertexBuffer_Destroy(GLuint *name);

typedef enum
{
    GLMDL_BUFFER_POSITIONS,
    GLMDL_BUFFER_TEXCOORDS,
    GLMDL_BUFFER_COLORS,
    GLMDL_BUFFER_TRIANGLES,

    NUM_GLMDL_BUFFERS
} glmdl_buffer_t;

typedef struct
{
    int framesize; // byte size of each frame
    int num_skins;
    int num_xyz;
    int num_st;
    int num_frames;
    int num_tris; // number of indexed tris (indices in range [0, num_verts - 1])
    int ofs_skins; // each skin is a MAX_SKINNAME string
    int ofs_st; // byte offset from start for stverts (float, float)
    int ofs_frames; // offset for first frame
    int ofs_tris; // offset for indexed tris (short, short, short)
    int ofs_xyz_from_st_indices; // byte offet to (num_st - num_xyz) indices mapping st to xyz
    GLuint buffers[NUM_GLMDL_BUFFERS];
} glmdl_t;

typedef struct
{
    GLfloat s, t;
} glstvert_t;

typedef struct
{
    short a, b, c;
} gltriangle_t;

#include "../Remotery/lib/Remotery.h"
