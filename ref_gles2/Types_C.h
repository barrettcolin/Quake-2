#pragma once

#include <stdint.h>

enum Constants
{
    MAX_LIGHTMAPS = 4
};

typedef int16_t ClusterId;

typedef uint16_t VertexIndex;

struct TexInfo
{
    float vecs[2][4];
    int flags;
    int numframes;
    struct TexInfo *next;
    struct image_s *image; // not const, written during registration sequence
};

struct MapModelVertex
{
    float m_xyz[3];
    float m_st[2][2];
};

struct Poly
{
    struct Poly *next;
    struct Poly *chain;
    int numverts;
    int flags; // for SURF_UNDERWATER (not needed anymore?)
    struct MapModelVertex verts[4];
};

struct Surface
{
    int visframe; // should be drawn when node is crossed
    struct cplane_s	*plane;
    int flags;
    int firstedge; // look up in model->surfedges[], negative numbers
    int numedges; // are backwards edges
    short texturemins[2];
    short extents[2];
    int light_s, light_t; // gl lightmap coordinates
    int dlight_s, dlight_t; // gl lightmap coordinates for dynamic lightmaps
    struct Poly *polys; // multiple if warped
    struct Surface *texturechain;
    struct Surface *lightmapchain;
    struct TexInfo *texinfo;
    int dlightframe;
    int dlightbits;
    int lightmaptexturenum;
    uint8_t styles[MAX_LIGHTMAPS];
    float cached_light[MAX_LIGHTMAPS]; // values currently used in lightmap
    uint8_t *samples; // [numstyles*surfsize]
    struct glmesh_s *m_mesh;
};

struct MapModelMeshSection
{
    int m_lightMap;
    struct TexInfo *m_texInfo;
    unsigned m_firstStripIndex;
    unsigned m_numStripIndices;
};
