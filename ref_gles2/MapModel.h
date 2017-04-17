#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Types_C.h"

struct NodeBase
{
    int m_contents;
    int m_visframe;
    float m_minMaxs[6];
    struct mnode_s *m_parent;
};

struct mnode_s : NodeBase
{
    struct cplane_s *m_plane;
    NodeBase *m_children[2];
    uint16_t m_firstSurface;
    uint16_t m_numSurfaces;
};

struct mleaf_s : NodeBase
{
    ClusterId m_cluster;
    int16_t m_area;
    int m_viewFrame;
    struct msurface_s **m_firstMarkSurface;
    int m_numMarkSurfaces;
};

struct SurfaceVertex
{
    float x, y, z, s0, t0, s1, t1;
};

struct SurfacePoly
{
    // Methods
    SurfacePoly(unsigned numVertices);

    void SetVertex(unsigned vertexIndex, float x, float y, float z, float s0, float t0, float s1, float t1);

    // Members
    std::vector<SurfaceVertex> m_vertices;
};

struct ClusterMeshBuilder
{
    // Types
    typedef std::unordered_map<TextureId, std::vector<SurfacePoly> > PolysFromBaseMap;

    typedef std::unordered_map<TextureId, PolysFromBaseMap> BaseMapPolysFromLightMap;

    typedef std::unordered_map<ClusterId, BaseMapPolysFromLightMap> SurfacesFromCluster;

    // Methods
    SurfacePoly& AllocatePoly(ClusterId cluster, TextureId lightMap, TextureId baseMap, unsigned numVertices);

    // Members
    SurfacesFromCluster m_surfacesFromCluster;
};

struct Mesh
{
    // Members
    uint32_t m_lightMap;

    uint32_t m_baseMap;

    std::vector<SurfacePoly> m_polys;
};

struct ClusterMeshData
{
    // Types
    typedef std::vector<Mesh> Meshes;

    // Methods
    ClusterMeshData(const ClusterMeshBuilder& clusterMeshBuilder);

    // Members
    std::vector<Meshes> m_clusterMeshes;
};

struct ClusterBuilder
{
    // Types
    typedef std::vector<mleaf_s*> Leafs;

    typedef std::unordered_map<ClusterId, Leafs> LeafsFromCluster;

    // Members
    LeafsFromCluster m_leafsFromCluster;
};

struct ClusterData
{
    // Types
    typedef std::vector<mnode_s*> Nodes;

    // Methods
    ClusterData(const ClusterBuilder& clusterBuilder);

    // Members
    Nodes m_clusterNodes;
};
