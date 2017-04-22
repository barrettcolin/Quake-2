#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Types_C.h"

struct cplane_s;
struct image_s;
struct mnode_s;

struct NodeBase
{
    int m_contents;
    int m_visframe;
    float m_minMaxs[6];
    mnode_s *m_parent;
};

struct mnode_s : NodeBase
{
    cplane_s *m_plane;
    NodeBase *m_children[2];
    uint16_t m_firstSurface;
    uint16_t m_numSurfaces;
    int m_viewFrame;
    struct glmesh_s *m_clusterMesh;
};

struct mleaf_s : NodeBase
{
    ClusterId m_cluster;
    int16_t m_area;
    int m_viewFrame;
    Surface **m_firstMarkSurface;
    int m_numMarkSurfaces;
};

struct ClusterMeshBuilder
{
    // Types
    struct Surfs
    {
        std::vector<Surface*> m_surfs;
    };

    typedef std::unordered_map<struct image_s const*, Surfs> SurfsFromImage;

    typedef std::unordered_map<int, SurfsFromImage> ImageSurfsFromLightMap;

    typedef std::unordered_map<ClusterId, ImageSurfsFromLightMap> SurfsFromCluster;

    // Methods
    void AddSurface(ClusterId, Surface *surface);

    // Members
    std::unordered_set<Surface*> m_surfaces;

    SurfsFromCluster m_surfacesFromCluster;
};

struct ClusterMesh
{
    // Members
    std::vector<VertexIndex> m_indices;

    std::vector<MapModelVertex> m_vertices;

    std::vector<MapModelMeshSection> m_sections;
};

struct ClusterMeshData
{
    // Methods
    ClusterMeshData(const ClusterMeshBuilder& clusterMeshBuilder);

    // Members
    std::vector<ClusterMesh> m_clusterMeshes;
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
