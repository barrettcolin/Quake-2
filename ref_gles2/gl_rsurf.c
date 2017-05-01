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
// GL_RSURF.C: surface-related refresh code
#include <assert.h>

#include "gl_local.h"

static vec3_t	modelorg;		// relative to viewpoint

msurface_t	*r_alpha_surfaces;

#define DYNAMIC_LIGHT_WIDTH  128
#define DYNAMIC_LIGHT_HEIGHT 128

#define LIGHTMAP_BYTES 4

#define	BLOCK_WIDTH		128
#define	BLOCK_HEIGHT	128

#define	MAX_LIGHTMAPS	128
#define NUM_LIGHTMAP_BUFFERS 2

int		c_visible_lightmaps;
int		c_visible_textures;

#define GL_LIGHTMAP_FORMAT GL_RGBA

typedef enum
{
	lmt_static,
	lmt_dynamic,

	num_lightmaptypes
} lightmaptype_t;

typedef struct
{
	int internal_format;
	int	current_lightmap_texture[num_lightmaptypes];

	msurface_t	*lightmap_surfaces[MAX_LIGHTMAPS];
    GLuint lightmapTextureNames[NUM_LIGHTMAP_BUFFERS][MAX_LIGHTMAPS];

	int			allocated[num_lightmaptypes][BLOCK_WIDTH];

	// the lightmap texture data needs to be kept in
	// main memory so texsubimage can update properly
	byte		lightmap_buffer[num_lightmaptypes][4*BLOCK_WIDTH*BLOCK_HEIGHT];
} gllightmapstate_t;

static gllightmapstate_t gl_lms;


static void		LM_InitBlock( lightmaptype_t type );
static void		LM_UploadBlock( lightmaptype_t type, qboolean dynamic );
static qboolean	LM_AllocBlock (lightmaptype_t type, int w, int h, int *x, int *y);

extern void R_SetCacheState( msurface_t *surf );
extern void R_BuildLightMap (msurface_t *surf, byte *dest, int stride);

static int s_currentLightmapBuffer = 0;

static GLuint GL_GetLightmapTextureName(int textureId)
{
    return gl_lms.lightmapTextureNames[s_currentLightmapBuffer][textureId];
}

void GL_DeleteLightmaps(void)
{
    int i = 0;

    for (; i < NUM_LIGHTMAP_BUFFERS; ++i)
    {
        qglDeleteTextures(MAX_LIGHTMAPS, gl_lms.lightmapTextureNames[i]);
        memset(gl_lms.lightmapTextureNames[i], 0, sizeof(gl_lms.lightmapTextureNames[i]));
    }
}

/*
=============================================================

	BRUSH MODELS

=============================================================
*/
static void UpdateBrushEntityLightmaps(void);

/*
===============
R_TextureAnimation

Returns the proper texture for a given time and base texture
===============
*/
static image_t *R_TextureAnimation (entity_t const *ent, mtexinfo_t const *tex)
{
	int		c;

	if (!tex->next)
		return tex->image;

	c = ent->frame % tex->numframes;
	while (c)
	{
		tex = tex->next;
		c--;
	}

	return tex->image;
}

static void GL_RenderMesh(entity_t const *ent, glmesh_t *mesh)
{
    int i;

    qglBindBuffer(GL_ARRAY_BUFFER, mesh->m_vertexBuffer);
    qglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct MapModelVertex), (void *)0);
    qglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct MapModelVertex), (void *)12);
    qglVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(struct MapModelVertex), (void *)20);

    qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->m_indexBuffer);

    for (i = 0; i < mesh->m_numMeshSections; ++i)
    {
        struct MapModelMeshSection const *section = &mesh->m_meshSections[i];
        if (section->m_lightMap == 0)
            continue;

        image_t *image = R_TextureAnimation(ent, section->m_texInfo);
        unsigned indicesOffset = section->m_firstStripIndex * sizeof(VertexIndex);

        GL_MBind(GL_TEXTURE0, image->texnum);
        GL_MBind(GL_TEXTURE1, GL_GetLightmapTextureName(section->m_lightMap));

        qglDrawElements(GL_TRIANGLE_STRIP, section->m_numStripIndices, GL_UNSIGNED_SHORT, (void *)indicesOffset);
    }

    qglBindBuffer(GL_ARRAY_BUFFER, 0);
    qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

#if 0
/*
=================
WaterWarpPolyVerts

Mangles the x and y coordinates in a copy of the poly
so that any drawing routine can be water warped
=================
*/
glpoly_t *WaterWarpPolyVerts (glpoly_t *p)
{
	int		i;
	float	*v, *nv;
	static byte	buffer[1024];
	glpoly_t *out;

	out = (glpoly_t *)buffer;

	out->numverts = p->numverts;
	v = p->verts[0];
	nv = out->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE, nv+=VERTEXSIZE)
	{
		nv[0] = v[0] + 4*sin(v[1]*0.05+r_newrefdef.time)*sin(v[2]*0.05+r_newrefdef.time);
		nv[1] = v[1] + 4*sin(v[0]*0.05+r_newrefdef.time)*sin(v[2]*0.05+r_newrefdef.time);

		nv[2] = v[2];
		nv[3] = v[3];
		nv[4] = v[4];
		nv[5] = v[5];
		nv[6] = v[6];
	}

	return out;
}

/*
================
DrawGLWaterPoly

Warp the vertex coordinates
================
*/
void DrawGLWaterPoly (glpoly_t *p)
{
	int		i;
	float	*v;

	p = WaterWarpPolyVerts (p);
	qglBegin (GL_TRIANGLE_FAN);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		qglTexCoord2f (v[3], v[4]);
		qglVertex3fv (v);
	}
	qglEnd ();
}
void DrawGLWaterPolyLightmap (glpoly_t *p)
{
	int		i;
	float	*v;

	p = WaterWarpPolyVerts (p);
	qglBegin (GL_TRIANGLE_FAN);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		qglTexCoord2f (v[5], v[6]);
		qglVertex3fv (v);
	}
	qglEnd ();
}
#endif

/*
================
DrawGLPoly
================
*/
void DrawGLPoly (glpoly_t *p)
{
    qglVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(struct MapModelVertex), p->verts[0].m_xyz);
    qglVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(struct MapModelVertex), p->verts[0].m_st[0]);
    qglDrawArrays(GL_TRIANGLE_FAN, 0, p->numverts);
}

//============
//PGM
/*
================
DrawGLFlowingPoly -- version of DrawGLPoly that handles scrolling texture
================
*/
void DrawGLFlowingPoly (msurface_t *fa)
{
#if 0
	int		i;
	float	*v;
	glpoly_t *p;
	float	scroll;

	p = fa->polys;

	scroll = -64 * ( (r_newrefdef.time / 40.0) - (int)(r_newrefdef.time / 40.0) );
	if(scroll == 0.0)
		scroll = -64.0;

	qglBegin (GL_POLYGON);
	v = p->verts[0];
	for (i=0 ; i<p->numverts ; i++, v+= VERTEXSIZE)
	{
		qglTexCoord2f ((v[3] + scroll), v[4]);
		qglVertex3fv (v);
	}
	qglEnd ();
#endif
}
//PGM
//============

/*
** R_DrawTriangleOutlines
*/
void R_DrawTriangleOutlines (void)
{
#if 0
	int			i, j;
	glpoly_t	*p;

	if (!gl_showtris->value)
		return;

	qglDisable (GL_TEXTURE_2D);
	qglDisable (GL_DEPTH_TEST);
	qglColor4f (1,1,1,1);

	for (i=0 ; i<MAX_LIGHTMAPS ; i++)
	{
		msurface_t *surf;

		for ( surf = gl_lms.lightmap_surfaces[i]; surf != 0; surf = surf->lightmapchain )
		{
			p = surf->polys;
			for ( ; p ; p=p->chain)
			{
				for (j=2 ; j<p->numverts ; j++ )
				{
					qglBegin (GL_LINE_STRIP);
					qglVertex3fv (p->verts[0]);
					qglVertex3fv (p->verts[j-1]);
					qglVertex3fv (p->verts[j]);
					qglVertex3fv (p->verts[0]);
					qglEnd ();
				}
			}
		}
	}

	qglEnable (GL_DEPTH_TEST);
	qglEnable (GL_TEXTURE_2D);
#endif
}


/*
================
R_RenderBrushPoly
================
*/
void R_RenderBrushPoly(entity_t const *ent, msurface_t *fa)
{
	int			maps;
	image_t		*image;
	qboolean is_dynamic = false;

	c_brush_polys++;

    image = R_TextureAnimation(ent, fa->texinfo);

    GL_MBind(GL_TEXTURE0, image->texnum);

    if (fa->flags & SURF_DRAWTURB)
    {
		EmitWaterPolys (fa);
		return;
	}

//======
//PGM
	if(fa->texinfo->flags & SURF_FLOWING)
		DrawGLFlowingPoly (fa);
	else
		DrawGLPoly (fa->polys);
//PGM
//======

	/*
	** check for lightmap modification
	*/
	for ( maps = 0; maps < MAXLIGHTMAPS && fa->styles[maps] != 255; maps++ )
	{
		if ( r_newrefdef.lightstyles[fa->styles[maps]].white != fa->cached_light[maps] )
			goto dynamic;
	}

	// dynamic this frame or dynamic previously
	if ( ( fa->dlightframe == r_framecount ) )
	{
dynamic:
		if ( gl_dynamic->value )
		{
			if (!( fa->texinfo->flags & (SURF_SKY|SURF_TRANS33|SURF_TRANS66|SURF_WARP ) ) )
			{
				is_dynamic = true;
			}
		}
	}

	if ( is_dynamic )
	{
		if ( ( fa->styles[maps] >= 32 || fa->styles[maps] == 0 ) && ( fa->dlightframe != r_framecount ) )
		{
			unsigned	temp[34*34];
			int			smax, tmax;

			smax = (fa->extents[0]>>4)+1;
			tmax = (fa->extents[1]>>4)+1;

			R_BuildLightMap( fa, (void *)temp, smax*4 );
			R_SetCacheState( fa );

            GL_SelectTexture(GL_TEXTURE1);
			GL_Bind(GL_GetLightmapTextureName(fa->lightmaptexturenum));

            qglTexSubImage2D( GL_TEXTURE_2D, 0,
							  fa->light_s, fa->light_t, 
							  smax, tmax, 
							  GL_LIGHTMAP_FORMAT, 
							  GL_UNSIGNED_BYTE, temp );

			fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
			gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
		}
		else
		{
			fa->lightmapchain = gl_lms.lightmap_surfaces[0];
			gl_lms.lightmap_surfaces[0] = fa;
		}
	}
	else
	{
		fa->lightmapchain = gl_lms.lightmap_surfaces[fa->lightmaptexturenum];
		gl_lms.lightmap_surfaces[fa->lightmaptexturenum] = fa;
	}
}


/*
================
R_DrawAlphaSurfaces

Draw water surfaces and windows.
The BSP tree is waled front to back, so unwinding the chain
of alpha_surfaces will draw back to front, giving proper ordering.
================
*/
void R_DrawAlphaSurfaces (void)
{
	msurface_t	*s;
	float		intens;

    if(!r_alpha_surfaces)
        return;

	//
	// go back to the world matrix
	//
    Material_SetCurrent(g_unlit_alpha_material);
    Material_SetClipFromView(g_unlit_alpha_material, gl_state.clip_from_view);
    Material_SetViewFromWorld(g_unlit_alpha_material, gl_state.view_from_world);
    Material_SetWorldFromModel(g_unlit_alpha_material, g_identity_matrix);

	// the textures are prescaled up for a better lighting range,
	// so scale it back down
	intens = gl_state.inverse_intensity;

	for (s=r_alpha_surfaces ; s ; s=s->texturechain)
	{
        GL_MBind(GL_TEXTURE0, s->texinfo->image->texnum);
		c_brush_polys++;
		if (s->texinfo->flags & SURF_TRANS33)
            Material_SetDiffuseColor(g_unlit_alpha_material, intens, intens, intens, 0.33f);
		else if (s->texinfo->flags & SURF_TRANS66)
            Material_SetDiffuseColor(g_unlit_alpha_material, intens, intens, intens, 0.66f);
		else
            Material_SetDiffuseColor(g_unlit_alpha_material, intens, intens, intens, 1.0f);

		if (s->flags & SURF_DRAWTURB)
			EmitWaterPolys (s);
		else
			DrawGLPoly (s->polys);
	}

	r_alpha_surfaces = NULL;
}

/*
================
DrawTextureChains
================
*/
void DrawTextureChains(entity_t const *ent)
{
	int		i;
	msurface_t	*s;
	image_t		*image;

	c_visible_textures = 0;

    for ( i = 0, image=gltextures ; i<numgltextures ; i++,image++)
    {
        if (!image->registration_sequence)
            continue;
        c_visible_textures++;

        s = image->texturechain;
        if (!s)
            continue;

        for ( ; s ; s=s->texturechain)
        {
			if ( s->flags & SURF_DRAWTURB )
                R_RenderBrushPoly(ent, s);
        }

        image->texturechain = NULL;
    }
}


static void GL_UpdateLightmap(msurface_t *surf)
{
    int map;
    qboolean is_dynamic_style;
    qboolean is_dlight;
    qboolean was_dlight;
    int lightmap_dirty_bit = (1 << (s_currentLightmapBuffer + 16)); // 1 bit set per lightmap buffer in high 16bits of surf->flags

    if (!gl_dynamic->value)
        return;

    if (surf->texinfo->flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66 | SURF_WARP))
        return;

    for (is_dynamic_style = false, map = 0; map < MAXLIGHTMAPS && surf->styles[map] != 255; map++)
    {
        if (r_newrefdef.lightstyles[surf->styles[map]].white != surf->cached_light[map])
        {
            is_dynamic_style = true;
            break;
        }
    }

    is_dlight = (surf->dlightframe == r_framecount);
    was_dlight = surf->flags & lightmap_dirty_bit;

    if (is_dynamic_style || is_dlight || was_dlight)
    {
        unsigned	temp[128 * 128];
        int			smax, tmax;

        smax = (surf->extents[0] >> 4) + 1;
        tmax = (surf->extents[1] >> 4) + 1;

        R_BuildLightMap(surf, (void *)temp, smax * 4);

        if ((surf->styles[map] >= 32 || surf->styles[map] == 0) && !is_dlight)
        {
            R_SetCacheState(surf);
        }

        // clear stale dlights
        if (was_dlight && !is_dlight)
        {
            surf->flags &= ~lightmap_dirty_bit;
        }
        else if (is_dlight)
        {
            surf->flags |= lightmap_dirty_bit;
        }

        GL_SelectTexture(GL_TEXTURE1);
        GL_Bind(GL_GetLightmapTextureName(surf->lightmaptexturenum));

        qglTexSubImage2D(
            GL_TEXTURE_2D, 0, surf->light_s, surf->light_t, smax, tmax, GL_LIGHTMAP_FORMAT, GL_UNSIGNED_BYTE, temp);
    }
}


/*
=================
R_DrawInlineBModel
=================
*/
static void R_DrawInlineBModel(entity_t const *ent)
{
	int			i;
	cplane_t	*pplane;
	float		dot;
	msurface_t	*psurf;
    float alpha;
    GLfloat world_from_model[16];
    int is_translucent = (ent->flags & RF_TRANSLUCENT);

    Matrix_FromAnglesOrigin(ent->angles, ent->origin, world_from_model);

	psurf = &ent->model->surfaces[ent->model->firstmodelsurface];

    alpha = is_translucent ? 0.25 : 1;

    if (ent->model->meshes[0])
    {
        material_id mat = is_translucent ? g_lightmapped_alpha_material : g_lightmapped_material;
        Material_SetCurrent(mat);
        Material_SetWorldFromModel(mat, world_from_model); // proj + view already set when drawing world
        if (is_translucent)
        {
            Material_SetDiffuseColor(mat, 1, 1, 1, alpha);
        }
        GL_RenderMesh(ent, ent->model->meshes[0]);
    }

    //
	// draw texture
	//
    for (i = 0; i < ent->model->nummodelsurfaces; i++, psurf++)
	{
	// find which side of the node we are on
		pplane = psurf->plane;

		dot = DotProduct (modelorg, pplane->normal) - pplane->dist;

	// draw the polygon
		if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
			(!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
		{
			if (psurf->texinfo->flags & (SURF_TRANS33|SURF_TRANS66) )
			{	// add to the translucent chain (drawn with world_matrix, so this probably doesn't work)
				psurf->texturechain = r_alpha_surfaces;
				r_alpha_surfaces = psurf;
			}
            else if(psurf->flags & SURF_DRAWTURB)
			{
                material_id mat = g_unlit_material;
                Material_SetCurrent(mat);
                Material_SetWorldFromModel(mat, world_from_model); // proj + view already set when drawing world
                Material_SetDiffuseColor(g_unlit_material, gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, alpha);
                R_RenderBrushPoly(ent, psurf);
			}
		}
	}
}

/*
=================
R_DrawBrushModel
=================
*/
void R_DrawBrushModel (entity_t *ent)
{
	vec3_t		mins, maxs;
	int			i;
	qboolean	rotated;

	if (ent->model->nummodelsurfaces == 0)
		return;

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	if (ent->angles[0] || ent->angles[1] || ent->angles[2])
	{
		rotated = true;
		for (i=0 ; i<3 ; i++)
		{
			mins[i] = ent->origin[i] - ent->model->radius;
			maxs[i] = ent->origin[i] + ent->model->radius;
		}
	}
	else
	{
		rotated = false;
		VectorAdd (ent->origin, ent->model->mins, mins);
		VectorAdd (ent->origin, ent->model->maxs, maxs);
	}

	if (R_CullBox (mins, maxs))
		return;

	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));

	VectorSubtract (r_newrefdef.vieworg, ent->origin, modelorg);
	if (rotated)
	{
		vec3_t	temp;
		vec3_t	forward, right, up;

		VectorCopy (modelorg, temp);
		AngleVectors (ent->angles, forward, right, up);
		modelorg[0] = DotProduct (temp, forward);
		modelorg[1] = -DotProduct (temp, right);
		modelorg[2] = DotProduct (temp, up);
	}

    R_DrawInlineBModel(ent);
}

/*
=============================================================

	WORLD MODEL

=============================================================
*/
static int CalcSide(cplane_t const *plane)
{
    float dot;

    switch (plane->type)
    {
    case PLANE_X:
        dot = modelorg[0] - plane->dist;
        break;
    case PLANE_Y:
        dot = modelorg[1] - plane->dist;
        break;
    case PLANE_Z:
        dot = modelorg[2] - plane->dist;
        break;
    default:
        dot = DotProduct(modelorg, plane->normal) - plane->dist;
        break;
    }

    return (dot >= 0) ? 0 : 1;
}


static void MarkSurfacesAndUpdateLightmaps(entity_t const *ent, mnode_t *node)
{
    qboolean is_solid_leaf = (node->contents == CONTENTS_SOLID);
    qboolean not_visible_frame = (node->visframe != r_visframecount);
    if (is_solid_leaf || not_visible_frame)
        return;

    // Node culled?
    if (R_CullBox(node->minmaxs, node->minmaxs + 3))
        return;

    // Node is leaf?
    if (node->contents != -1)
    {
        int i;
        msurface_t **mark;
        mleaf_t *pleaf = (mleaf_t *)node;

        // Area test
        if (r_newrefdef.areabits && !(r_newrefdef.areabits[pleaf->area >> 3] & (1 << (pleaf->area & 7))))
            return;

        // Mark surfaces
        mark = pleaf->firstmarksurface;
        i = pleaf->nummarksurfaces;

        if (i)
        {
            do
            {
                (*mark)->visframe = r_framecount;
                mark++;
            } while (--i);
        }
    }
    else
    {
        // Determine plane side
        int side = CalcSide(node->plane);
        int plane_planeback = side ? SURF_PLANEBACK : 0;

        // Front side
        MarkSurfacesAndUpdateLightmaps(ent, node->children[side]);

        // Surfaces
        {
            int i;
            msurface_t *surf;
            for (i = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; i; i--, surf++)
            {
                int surf_planeback;
                if (surf->visframe != r_framecount)
                    continue;

                surf_planeback = surf->flags & SURF_PLANEBACK;
                if (surf_planeback != plane_planeback)
                    continue;

                if ((surf->flags & SURF_DRAWTURB) || (surf->texinfo->flags & (SURF_SKY | SURF_TRANS33 | SURF_TRANS66)))
                    continue;

                GL_UpdateLightmap(surf);
            }
        }

        // Back side
        MarkSurfacesAndUpdateLightmaps(ent, node->children[!side]);
    }
}


static void DrawSurfaces(entity_t const *ent, mnode_t *node)
{
    qboolean is_solid_leaf = (node->contents == CONTENTS_SOLID);
    qboolean not_visible_frame = (node->visframe != r_visframecount);
    if (is_solid_leaf || not_visible_frame)
        return;

    // Node culled?
    if (R_CullBox(node->minmaxs, node->minmaxs + 3))
        return;

    // Node is not leaf?
    if (node->contents == -1)
    {
        // Determine plane side
        int side = CalcSide(node->plane);
        int plane_planeback = side ? SURF_PLANEBACK : 0;

        // Front side
        DrawSurfaces(ent, node->children[side]);

        // Surfaces
        {
            int i;
            msurface_t *surf;
            for (i = node->numsurfaces, surf = r_worldmodel->surfaces + node->firstsurface; i; i--, surf++)
            {
                int surf_planeback;
                if (surf->visframe != r_framecount)
                    continue;

                surf_planeback = surf->flags & SURF_PLANEBACK;
                if (surf_planeback != plane_planeback)
                    continue;

                if (surf->texinfo->flags & SURF_SKY)
                {
                    // Sky
                    R_AddSkySurface(surf);
                }
                else if (surf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66))
                {
                    // Transparent chain
                    surf->texturechain = r_alpha_surfaces;
                    r_alpha_surfaces = surf;
                }
                else
                {
                    if (!(surf->flags & SURF_DRAWTURB) && surf->m_mesh && (surf->m_mesh->m_viewFrame != r_framecount))
                    {
                        GL_RenderMesh(ent, surf->m_mesh);
                        surf->m_mesh->m_viewFrame = r_framecount;
                    }
                    else
                    {
                        // the polygon is visible, so add it to the texture
                        // sorted chain
                        // FIXME: this is a hack for animation
                        image_t *image = R_TextureAnimation(ent, surf->texinfo);
                        surf->texturechain = image->texturechain;
                        image->texturechain = surf;
                    }
                }
            }
        }

        // Back side
        DrawSurfaces(ent, node->children[!side]);
    }
}


/*
=============
R_DrawWorld
=============
*/
void R_DrawWorld (void)
{
	entity_t	ent;

    s_currentLightmapBuffer = r_framecount % NUM_LIGHTMAP_BUFFERS;

	if (!r_drawworld->value)
		return;

	if ( r_newrefdef.rdflags & RDF_NOWORLDMODEL )
		return;

	VectorCopy (r_newrefdef.vieworg, modelorg);

	// auto cycle the world frame for texture animation
	memset (&ent, 0, sizeof(ent));
	ent.frame = (int)(r_newrefdef.time*2);

	gl_state.currenttextures[0] = gl_state.currenttextures[1] = -1;

	memset (gl_lms.lightmap_surfaces, 0, sizeof(gl_lms.lightmap_surfaces));
	R_ClearSkyBox ();

    Material_SetCurrent(g_lightmapped_material);
    Material_SetClipFromView(g_lightmapped_material, gl_state.clip_from_view);
    Material_SetViewFromWorld(g_lightmapped_material, gl_state.view_from_world);
    Material_SetWorldFromModel(g_lightmapped_material, g_identity_matrix);

    rmt_BeginCPUSample(MarkSurfacesAndUpdateLightmaps, 0);
    MarkSurfacesAndUpdateLightmaps(&ent, r_worldmodel->nodes);

    // Update brush entity lightmaps here too
    UpdateBrushEntityLightmaps();
    rmt_EndCPUSample();

    rmt_BeginCPUSample(DrawSurfaces, 0);
    DrawSurfaces(&ent, r_worldmodel->nodes);
    rmt_EndCPUSample();

    qglBindBuffer(GL_ARRAY_BUFFER, 0);
    qglBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    // R_RenderBrushPoly for SURF_DRAWTURB surfaces not rendered by RecursiveWorldNode
    Material_SetCurrent(g_unlit_material);
    Material_SetClipFromView(g_unlit_material, gl_state.clip_from_view);
    Material_SetViewFromWorld(g_unlit_material, gl_state.view_from_world);
    Material_SetWorldFromModel(g_unlit_material, g_identity_matrix);
    Material_SetDiffuseColor(g_unlit_material, gl_state.inverse_intensity, gl_state.inverse_intensity, gl_state.inverse_intensity, 1);
    DrawTextureChains(&ent);
	
    Material_SetDiffuseColor(g_unlit_material, 1, 1, 1, 1);
	R_DrawSkyBox ();

	R_DrawTriangleOutlines ();
}


/*
===============
R_MarkLeaves

Mark the leaves and nodes that are in the PVS for the current
cluster
===============
*/
void R_MarkLeaves (void)
{
	byte	*vis;
	byte	fatvis[MAX_MAP_LEAFS/8];
	mnode_t	*node;
	int		i, c;
	mleaf_t	*leaf;
	int		cluster;

	if (r_oldviewcluster == r_viewcluster && r_oldviewcluster2 == r_viewcluster2 && !r_novis->value && r_viewcluster != -1)
		return;

	// development aid to let you run around and see exactly where
	// the pvs ends
	if (gl_lockpvs->value)
		return;

	r_visframecount++;
	r_oldviewcluster = r_viewcluster;
	r_oldviewcluster2 = r_viewcluster2;

	if (r_novis->value || r_viewcluster == -1 || !r_worldmodel->vis)
	{
		// mark everything
		for (i=0 ; i<r_worldmodel->numleafs ; i++)
			r_worldmodel->leafs[i].visframe = r_visframecount;
		for (i=0 ; i<r_worldmodel->numnodes ; i++)
			r_worldmodel->nodes[i].visframe = r_visframecount;
		return;
	}

	vis = Mod_ClusterPVS (r_viewcluster, r_worldmodel);
	// may have to combine two clusters because of solid water boundaries
	if (r_viewcluster2 != r_viewcluster)
	{
		memcpy (fatvis, vis, (r_worldmodel->numleafs+7)/8);
		vis = Mod_ClusterPVS (r_viewcluster2, r_worldmodel);
		c = (r_worldmodel->numleafs+31)/32;
		for (i=0 ; i<c ; i++)
			((int *)fatvis)[i] |= ((int *)vis)[i];
		vis = fatvis;
	}
	
	for (i=0,leaf=r_worldmodel->leafs ; i<r_worldmodel->numleafs ; i++, leaf++)
	{
		cluster = leaf->cluster;
		if (cluster == -1)
			continue;
		if (vis[cluster>>3] & (1<<(cluster&7)))
		{
			node = (mnode_t *)leaf;
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}

#if 0
	for (i=0 ; i<r_worldmodel->vis->numclusters ; i++)
	{
		if (vis[i>>3] & (1<<(i&7)))
		{
			node = (mnode_t *)&r_worldmodel->leafs[i];	// FIXME: cluster
			do
			{
				if (node->visframe == r_visframecount)
					break;
				node->visframe = r_visframecount;
				node = node->parent;
			} while (node);
		}
	}
#endif
}



/*
=============================================================================

  LIGHTMAP ALLOCATION

=============================================================================
*/

static void LM_InitBlock( lightmaptype_t type )
{
	memset( gl_lms.allocated[type], 0, sizeof( gl_lms.allocated[type] ) );
}

static void LM_UploadBlock( lightmaptype_t type, qboolean dynamic )
{
    GL_SelectTexture(GL_TEXTURE1);

	if ( dynamic )
	{
		int i, height = 0;

        GL_Bind(gl_lms.lightmapTextureNames[0][0]);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        for ( i = 0; i < BLOCK_WIDTH; i++ )
		{
			if ( gl_lms.allocated[type][i] > height )
				height = gl_lms.allocated[type][i];
		}

        qglTexSubImage2D( GL_TEXTURE_2D,
						  0,
						  0, 0,
						  BLOCK_WIDTH, height,
						  GL_LIGHTMAP_FORMAT,
						  GL_UNSIGNED_BYTE,
						  gl_lms.lightmap_buffer[type] );
	}
	else
	{
        int b = 0, 
            texture = gl_lms.current_lightmap_texture[type], 
            next_texture =
                ((gl_lms.current_lightmap_texture[lmt_static] >
                gl_lms.current_lightmap_texture[lmt_dynamic])
                ? gl_lms.current_lightmap_texture[lmt_static]
                  : gl_lms.current_lightmap_texture[lmt_dynamic]) + 1;

        for (; b < NUM_LIGHTMAP_BUFFERS; ++b)
        {
            if (gl_lms.lightmapTextureNames[b][texture] == 0)
                qglGenTextures(1, &gl_lms.lightmapTextureNames[b][texture]);

            GL_Bind(gl_lms.lightmapTextureNames[b][texture]);
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // write texture for current
            qglTexImage2D(GL_TEXTURE_2D,
                0,
                gl_lms.internal_format,
                BLOCK_WIDTH, BLOCK_HEIGHT,
                0,
                GL_LIGHTMAP_FORMAT,
                GL_UNSIGNED_BYTE,
                gl_lms.lightmap_buffer[type]);
        }

        if ( next_texture == MAX_LIGHTMAPS )
			ri.Sys_Error( ERR_DROP, "LM_UploadBlock() - MAX_LIGHTMAPS exceeded\n" );
        else
            gl_lms.current_lightmap_texture[type] = next_texture;
	}
}

// returns a texture number and the position inside it
static qboolean LM_AllocBlock (lightmaptype_t type, int w, int h, int *x, int *y)
{
	int		i, j;
	int		best, best2;

	best = BLOCK_HEIGHT;

	for (i=0 ; i<BLOCK_WIDTH-w ; i++)
	{
		best2 = 0;

		for (j=0 ; j<w ; j++)
		{
			if (gl_lms.allocated[type][i+j] >= best)
				break;
			if (gl_lms.allocated[type][i+j] > best2)
				best2 = gl_lms.allocated[type][i+j];
		}
		if (j == w)
		{	// this is a valid spot
			*x = i;
			*y = best = best2;
		}
	}

	if (best + h > BLOCK_HEIGHT)
		return false;

	for (i=0 ; i<w ; i++)
		gl_lms.allocated[type][*x + i] = best + h;

	return true;
}

/*
================
GL_BuildPolygonFromSurface
================
*/
void GL_BuildPolygonFromSurface(medge_t const *pedges, int const *surfedges, mvertex_t const *vertexes, msurface_t *fa)
{
	int			i, lindex, lnumverts;
	medge_t		const *r_pedge;
	float		const *vec;
	float		s, t;
	glpoly_t	*poly;
	vec3_t		total;

// reconstruct the polygon
	lnumverts = fa->numedges;

	VectorClear (total);
	//
	// draw texture
	//
	poly = Hunk_Alloc (sizeof(glpoly_t) + (lnumverts-4) * sizeof(struct MapModelVertex));
	poly->next = fa->polys;
	poly->flags = fa->flags;
	fa->polys = poly;
	poly->numverts = lnumverts;

	for (i=0 ; i<lnumverts ; i++)
	{
		lindex = surfedges[fa->firstedge + i];

		if (lindex > 0)
		{
			r_pedge = &pedges[lindex];
			vec = vertexes[r_pedge->v[0]].position;
		}
		else
		{
			r_pedge = &pedges[-lindex];
			vec = vertexes[r_pedge->v[1]].position;
		}
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s /= fa->texinfo->image->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t /= fa->texinfo->image->height;

		VectorAdd (total, vec, total);
		VectorCopy (vec, poly->verts[i].m_xyz);
		poly->verts[i].m_st[0][0] = s;
		poly->verts[i].m_st[0][1] = t;

		//
		// lightmap texture coordinates
		//
		s = DotProduct (vec, fa->texinfo->vecs[0]) + fa->texinfo->vecs[0][3];
		s -= fa->texturemins[0];
		s += fa->light_s*16;
		s += 8;
		s /= BLOCK_WIDTH*16; //fa->texinfo->texture->width;

		t = DotProduct (vec, fa->texinfo->vecs[1]) + fa->texinfo->vecs[1][3];
		t -= fa->texturemins[1];
		t += fa->light_t*16;
		t += 8;
		t /= BLOCK_HEIGHT*16; //fa->texinfo->texture->height;

		poly->verts[i].m_st[1][0] = s;
		poly->verts[i].m_st[1][1] = t;
	}

	poly->numverts = lnumverts;
}

/*
========================
GL_CreateSurfaceLightmap
========================
*/
void GL_CreateSurfaceLightmap (msurface_t *surf, qboolean is_dynamic)
{
	int		smax, tmax;
	byte	*base;
	lightmaptype_t type = is_dynamic ? lmt_dynamic : lmt_static;

	if (surf->flags & (SURF_DRAWSKY|SURF_DRAWTURB))
		return;

	smax = (surf->extents[0]>>4)+1;
	tmax = (surf->extents[1]>>4)+1;

	if ( !LM_AllocBlock( type, smax, tmax, &surf->light_s, &surf->light_t ) )
	{
		LM_UploadBlock( type, false );
		LM_InitBlock(type);
		if ( !LM_AllocBlock( type, smax, tmax, &surf->light_s, &surf->light_t ) )
		{
			ri.Sys_Error( ERR_FATAL, "Consecutive calls to LM_AllocBlock(%d,%d) failed\n", smax, tmax );
		}
	}

	surf->lightmaptexturenum = gl_lms.current_lightmap_texture[type];

	base = gl_lms.lightmap_buffer[type];
	base += (surf->light_t * BLOCK_WIDTH + surf->light_s) * LIGHTMAP_BYTES;

	R_SetCacheState( surf );
	R_BuildLightMap (surf, base, BLOCK_WIDTH*LIGHTMAP_BYTES);
}


/*
==================
GL_BeginBuildingLightmaps

==================
*/
void GL_BeginBuildingLightmaps (model_t *m)
{
	static lightstyle_t	lightstyles[MAX_LIGHTSTYLES];
	int				i;
	unsigned		dummy[128*128];

	memset( gl_lms.allocated, 0, sizeof(gl_lms.allocated) );

	r_framecount = 1;		// no dlightcache

	/*
	** setup the base lightstyles so the lightmaps won't have to be regenerated
	** the first time they're seen
	*/
	for (i=0 ; i<MAX_LIGHTSTYLES ; i++)
	{
		lightstyles[i].rgb[0] = 1;
		lightstyles[i].rgb[1] = 1;
		lightstyles[i].rgb[2] = 1;
		lightstyles[i].white = 3;
	}
	r_newrefdef.lightstyles = lightstyles;

	gl_lms.current_lightmap_texture[lmt_static] = 1;
	gl_lms.current_lightmap_texture[lmt_dynamic] = 2;

	/*
	** if mono lightmaps are enabled and we want to use alpha
	** blending (a,1-a) then we're likely running on a 3DLabs
	** Permedia2.  In a perfect world we'd use a GL_ALPHA lightmap
	** in order to conserve space and maximize bandwidth, however 
	** this isn't a perfect world.
	**
	** So we have to use alpha lightmaps, but stored in GL_RGBA format,
	** which means we only get 1/16th the color resolution we should when
	** using alpha lightmaps.  If we find another board that supports
	** only alpha lightmaps but that can at least support the GL_ALPHA
	** format then we should change this code to use real alpha maps.
	*/
	if ( toupper( gl_monolightmap->string[0] ) == 'A' )
	{
        gl_lms.internal_format = GL_RGBA;
	}
	/*
	** try to do hacked colored lighting with a blended texture
	*/
	else if ( toupper( gl_monolightmap->string[0] ) == 'C' )
	{
        gl_lms.internal_format = GL_RGBA;
	}
	else if ( toupper( gl_monolightmap->string[0] ) == 'I' )
	{
        gl_lms.internal_format = GL_LUMINANCE;
	}
	else if ( toupper( gl_monolightmap->string[0] ) == 'L' ) 
	{
        gl_lms.internal_format = GL_LUMINANCE;
	}
	else
	{
        gl_lms.internal_format = GL_RGBA;
	}

	/*
	** initialize the dynamic lightmap texture
	*/
    for (i = 0; i < NUM_LIGHTMAP_BUFFERS; ++i)
    {
        qglGenTextures(1, &gl_lms.lightmapTextureNames[i][0]);
        GL_MBind(GL_TEXTURE1, gl_lms.lightmapTextureNames[i][0]);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        qglTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        qglTexImage2D(GL_TEXTURE_2D,
            0,
            gl_lms.internal_format,
            BLOCK_WIDTH, BLOCK_HEIGHT,
            0,
            GL_LIGHTMAP_FORMAT,
            GL_UNSIGNED_BYTE,
            dummy);
    }
}

/*
=======================
GL_EndBuildingLightmaps
=======================
*/
void GL_EndBuildingLightmaps (void)
{
	LM_UploadBlock( lmt_static, false );
	LM_UploadBlock( lmt_dynamic, false );
}

static void UpdateBrushEntityLightmaps(void)
{
    int i;

    for (i = 0; i < r_newrefdef.num_entities; i++)
    {
        entity_t *brushEnt = &r_newrefdef.entities[i];
        if (brushEnt->flags & (RF_TRANSLUCENT | RF_BEAM))
            continue;

        if (brushEnt->model && (brushEnt->model->type == mod_brush) && brushEnt->model->nummodelsurfaces)
        {
            qboolean rotated;
            int j;
            vec3_t mins, maxs;
            msurface_t *psurf = brushEnt->model->surfaces + brushEnt->model->firstmodelsurface;

            if (brushEnt->angles[0] || brushEnt->angles[1] || brushEnt->angles[2])
            {
                int k;
                rotated = true;
                for (k = 0; k < 3; k++)
                {
                    mins[k] = brushEnt->origin[k] - brushEnt->model->radius;
                    maxs[k] = brushEnt->origin[k] + brushEnt->model->radius;
                }
            }
            else
            {
                rotated = false;
                VectorAdd(brushEnt->origin, brushEnt->model->mins, mins);
                VectorAdd(brushEnt->origin, brushEnt->model->maxs, maxs);
            }

            VectorSubtract(r_newrefdef.vieworg, brushEnt->origin, modelorg);
            if (rotated)
            {
                vec3_t temp;
                vec3_t forward, right, up;

                VectorCopy(modelorg, temp);
                AngleVectors(brushEnt->angles, forward, right, up);
                modelorg[0] = DotProduct(temp, forward);
                modelorg[1] = -DotProduct(temp, right);
                modelorg[2] = DotProduct(temp, up);
            }

            for (j = 0; j < brushEnt->model->nummodelsurfaces; j++, psurf++)
            {
                cplane_t const *pplane = psurf->plane;
                float dot = DotProduct(modelorg, pplane->normal) - pplane->dist;

                if (((psurf->flags & SURF_PLANEBACK) && (dot < -BACKFACE_EPSILON)) ||
                    (!(psurf->flags & SURF_PLANEBACK) && (dot > BACKFACE_EPSILON)))
                {
                    if ((psurf->flags & SURF_DRAWTURB) || (psurf->texinfo->flags & (SURF_TRANS33 | SURF_TRANS66)))
                        continue;

                    GL_UpdateLightmap(psurf);
                }
            }
        }
    }

    VectorCopy(r_newrefdef.vieworg, modelorg);
}
