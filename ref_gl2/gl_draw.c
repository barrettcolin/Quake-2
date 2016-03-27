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

// draw.c

#include "gl_local.h"

static image_t *draw_chars;

extern	qboolean	scrap_dirty;
void Scrap_Upload (void);

typedef struct draw_vertex_s
{
    float x, y, z;
    float s, t;
} draw_vertex_t;

static material_id s_draw_material;
static material_id s_draw_alpha_material;

/*
===============
Draw_InitLocal
===============
*/
void Draw_InitLocal (void)
{
    materialdesc_t desc = { 0 };

	// load console characters (don't bilerp characters)
	draw_chars = GL_FindImage ("pics/conchars.pcx", it_pic);
	GL_Bind( draw_chars->texnum );
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    desc.type = mt_unlit;
    desc.blend = mb_opaque;

    s_draw_material = Material_Find(&desc);

    desc.alpha_test_66 = true;

    s_draw_alpha_material = Material_Find(&desc);
}



/*
================
Draw_Char

Draws one 8*8 graphics character with 0 being transparent.
It can be clipped to the top of the screen to allow the console to be
smoothly scrolled off.
================
*/
void Draw_Char (int x, int y, int num)
{
	int				row, col;
	float			frow, fcol, size;
    draw_vertex_t verts[4];

	num &= 255;
	
	if ( (num&127) == 32 )
		return;		// space

	if (y <= -8)
		return;			// totally off screen

	row = num>>4;
	col = num&15;

	frow = row*0.0625;
	fcol = col*0.0625;
	size = 0.0625;

    verts[0].x = x;
    verts[0].y = y;
    verts[0].z = 0;
    verts[0].s = fcol;
    verts[0].t = frow;

    verts[1].x = x;
    verts[1].y = y + 8;
    verts[1].z = 0;
    verts[1].s = fcol;
    verts[1].t = frow + size;

    verts[2].x = x + 8;
    verts[2].y = y;
    verts[2].z = 0;
    verts[2].s = fcol + size;
    verts[2].t = frow;

    verts[3].x = x + 8;
    verts[3].y = y + 8;
    verts[3].z = 0;
    verts[3].s = fcol + size;
    verts[3].t = frow + size;

    Material_SetCurrent(s_draw_alpha_material);
    Material_SetClipFromView(s_draw_alpha_material, gl_state.clip_from_view);
    Material_SetViewFromWorld(s_draw_alpha_material, g_identity_matrix);
    Material_SetWorldFromModel(s_draw_alpha_material, g_identity_matrix);
    Material_SetDiffuseColor(s_draw_alpha_material, 1, 1, 1, 1);

    GL_MBind(GL_TEXTURE0, draw_chars->texnum);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].x);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].s);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/*
=============
Draw_FindPic
=============
*/
image_t	*Draw_FindPic (char *name)
{
	image_t *gl;
	char	fullname[MAX_QPATH];

	if (name[0] != '/' && name[0] != '\\')
	{
		Com_sprintf (fullname, sizeof(fullname), "pics/%s.pcx", name);
		gl = GL_FindImage (fullname, it_pic);
	}
	else
		gl = GL_FindImage (name+1, it_pic);

	return gl;
}

/*
=============
Draw_GetPicSize
=============
*/
void Draw_GetPicSize (int *w, int *h, char *pic)
{
	image_t *gl;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		*w = *h = -1;
		return;
	}
	*w = gl->width;
	*h = gl->height;
}

/*
=============
Draw_StretchPic
=============
*/
void Draw_StretchPic (int x, int y, int w, int h, char *pic)
{
	image_t *gl;
    draw_vertex_t verts[4];
    material_id mat;

	gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}

	if (scrap_dirty)
		Scrap_Upload ();

    verts[0].x = x;
    verts[0].y = y;
    verts[0].z = 0;
    verts[0].s = gl->sl;
    verts[0].t = gl->tl;

    verts[1].x = x;
    verts[1].y = y + h;
    verts[1].z = 0;
    verts[1].s = gl->sl;
    verts[1].t = gl->th;

    verts[2].x = x + w;
    verts[2].y = y;
    verts[2].z = 0;
    verts[2].s = gl->sh;
    verts[2].t = gl->tl;

    verts[3].x = x + w;
    verts[3].y = y + h;
    verts[3].z = 0;
    verts[3].s = gl->sh;
    verts[3].t = gl->th;

    mat = gl->has_alpha ? s_draw_alpha_material : s_draw_material;
    Material_SetCurrent(mat);
    Material_SetClipFromView(mat, gl_state.clip_from_view);
    Material_SetViewFromWorld(mat, g_identity_matrix);
    Material_SetWorldFromModel(mat, g_identity_matrix);
    Material_SetDiffuseColor(mat, 1, 1, 1, 1);

    GL_MBind(GL_TEXTURE0, gl->texnum);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].x);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].s);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}


/*
=============
Draw_Pic
=============
*/
void Draw_Pic (int x, int y, char *pic)
{
	image_t *gl;
    draw_vertex_t verts[4];
    material_id mat;

    gl = Draw_FindPic (pic);
	if (!gl)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
	if (scrap_dirty)
		Scrap_Upload ();

    verts[0].x = x;
    verts[0].y = y;
    verts[0].z = 0;
    verts[0].s = gl->sl;
    verts[0].t = gl->tl;

    verts[1].x = x;
    verts[1].y = y + gl->height;
    verts[1].z = 0;
    verts[1].s = gl->sl;
    verts[1].t = gl->th;

    verts[2].x = x + gl->width;
    verts[2].y = y;
    verts[2].z = 0;
    verts[2].s = gl->sh;
    verts[2].t = gl->tl;

    verts[3].x = x + gl->width;
    verts[3].y = y + gl->height;
    verts[3].z = 0;
    verts[3].s = gl->sh;
    verts[3].t = gl->th;

    mat = gl->has_alpha ? s_draw_alpha_material : s_draw_material;
    Material_SetCurrent(mat);
    Material_SetClipFromView(mat, gl_state.clip_from_view);
    Material_SetViewFromWorld(mat, g_identity_matrix);
    Material_SetWorldFromModel(mat, g_identity_matrix);
    Material_SetDiffuseColor(mat, 1, 1, 1, 1);

    GL_MBind(GL_TEXTURE0, gl->texnum);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].x);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(verts[0]), &verts[0].s);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

/*
=============
Draw_TileClear

This repeats a 64*64 tile graphic to fill the screen around a sized down
refresh window.
=============
*/
void Draw_TileClear (int x, int y, int w, int h, char *pic)
{
	image_t	*image;

	image = Draw_FindPic (pic);
	if (!image)
	{
		ri.Con_Printf (PRINT_ALL, "Can't find pic: %s\n", pic);
		return;
	}
#if 0
	if ( ( ( gl_config.renderer == GL_RENDERER_MCD ) || ( gl_config.renderer & GL_RENDERER_RENDITION ) )  && !image->has_alpha)
		qglDisable (GL_ALPHA_TEST);

	GL_Bind (image->texnum);
	qglBegin (GL_QUADS);
	qglTexCoord2f (x/64.0, y/64.0);
	qglVertex2f (x, y);
	qglTexCoord2f ( (x+w)/64.0, y/64.0);
	qglVertex2f (x+w, y);
	qglTexCoord2f ( (x+w)/64.0, (y+h)/64.0);
	qglVertex2f (x+w, y+h);
	qglTexCoord2f ( x/64.0, (y+h)/64.0 );
	qglVertex2f (x, y+h);
	qglEnd ();

	if ( ( ( gl_config.renderer == GL_RENDERER_MCD ) || ( gl_config.renderer & GL_RENDERER_RENDITION ) )  && !image->has_alpha)
		qglEnable (GL_ALPHA_TEST);
#endif
}


/*
=============
Draw_Fill

Fills a box of pixels with a single color
=============
*/
void Draw_Fill (int x, int y, int w, int h, int c)
{
	union
	{
		unsigned	c;
		byte		v[4];
	} color;

	if ( (unsigned)c > 255)
		ri.Sys_Error (ERR_FATAL, "Draw_Fill: bad color");
#if 0
	qglDisable (GL_TEXTURE_2D);

	color.c = d_8to24table[c];
	qglColor3f (color.v[0]/255.0,
		color.v[1]/255.0,
		color.v[2]/255.0);

	qglBegin (GL_QUADS);

	qglVertex2f (x,y);
	qglVertex2f (x+w, y);
	qglVertex2f (x+w, y+h);
	qglVertex2f (x, y+h);

	qglEnd ();
	qglColor3f (1,1,1);
	qglEnable (GL_TEXTURE_2D);
#endif
}

//=============================================================================

/*
================
Draw_FadeScreen

================
*/
void Draw_FadeScreen (void)
{
#if 0
	qglEnable (GL_BLEND);
	qglDisable (GL_TEXTURE_2D);
	qglColor4f (0, 0, 0, 0.8);
	qglBegin (GL_QUADS);

	qglVertex2f (0,0);
	qglVertex2f (vid.width, 0);
	qglVertex2f (vid.width, vid.height);
	qglVertex2f (0, vid.height);

	qglEnd ();
	qglColor4f (1,1,1,1);
	qglEnable (GL_TEXTURE_2D);
	qglDisable (GL_BLEND);
#endif
}


//====================================================================


/*
=============
Draw_StretchRaw
=============
*/
extern unsigned	r_rawpalette[256];

void Draw_StretchRaw (int x, int y, int w, int h, int cols, int rows, byte *data)
{
#if 0
	unsigned	image32[256*256];
	unsigned char image8[256*256];
	int			i, j, trows;
	byte		*source;
	int			frac, fracstep;
	float		hscale;
	int			row;
	float		t;

	GL_Bind (0);

	if (rows<=256)
	{
		hscale = 1;
		trows = rows;
	}
	else
	{
		hscale = rows/256.0;
		trows = 256;
	}
	t = rows*hscale / 256;
#if 0
	if ( !qglColorTableEXT )
#endif
	{
		unsigned *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image32[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = r_rawpalette[source[frac>>16]];
				frac += fracstep;
			}
		}

        glTexImage2D (GL_TEXTURE_2D, 0, GL_RGBA, 256, 256, 0, GL_RGBA, GL_UNSIGNED_BYTE, image32);
	}
#if 0
	else
	{
		unsigned char *dest;

		for (i=0 ; i<trows ; i++)
		{
			row = (int)(i*hscale);
			if (row > rows)
				break;
			source = data + cols*row;
			dest = &image8[i*256];
			fracstep = cols*0x10000/256;
			frac = fracstep >> 1;
			for (j=0 ; j<256 ; j++)
			{
				dest[j] = source[frac>>16];
				frac += fracstep;
			}
		}

		qglTexImage2D( GL_TEXTURE_2D, 
			           0, 
					   GL_COLOR_INDEX8_EXT, 
					   256, 256, 
					   0, 
					   GL_COLOR_INDEX, 
					   GL_UNSIGNED_BYTE, 
					   image8 );
	}
#endif
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	if ( ( gl_config.renderer == GL_RENDERER_MCD ) || ( gl_config.renderer & GL_RENDERER_RENDITION ) ) 
		glDisable (GL_ALPHA_TEST);

	glBegin (GL_QUADS);
	glTexCoord2f (0, 0);
	glVertex2f (x, y);
	glTexCoord2f (1, 0);
	glVertex2f (x+w, y);
	glTexCoord2f (1, t);
	glVertex2f (x+w, y+h);
	glTexCoord2f (0, t);
	glVertex2f (x, y+h);
	glEnd ();

	if ( ( gl_config.renderer == GL_RENDERER_MCD ) || ( gl_config.renderer & GL_RENDERER_RENDITION ) ) 
		glEnable (GL_ALPHA_TEST);
#endif
}

