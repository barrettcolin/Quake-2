#pragma once

#include <stdint.h>

typedef int16_t ClusterId;
typedef uint32_t TextureId;

typedef uint16_t VertexIndex;

struct MapModelVertex
{
    float x, y, z;
    float s0, t0;
    float s1, t1;
};

struct MapModelMeshSection
{
    TextureId m_lightMap;
    TextureId m_baseMap;
    unsigned m_firstStripIndex;
    unsigned m_numStripIndices;
};
