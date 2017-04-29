#pragma once

#include <unordered_map>
#include <unordered_set>
#include <vector>

#include "Types_C.h"

struct ClusterMeshBuilder
{
    // Methods
    void AddSurface(ClusterId, Surface *surface);

    // Members
    std::unordered_set<Surface*> m_surfaces;

    std::unordered_map<ClusterId, std::vector<Surface*> > m_surfacesFromCluster;
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
