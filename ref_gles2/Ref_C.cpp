#include "Ref_C.h"

#include "MapModel.h"

ClusterMeshBuilder *ref_ClusterMeshBuilderCreate()
{
    return new ClusterMeshBuilder();
}

void ref_ClusterMeshBuilderDestroy(ClusterMeshBuilder *clusterMeshBuilder)
{
    delete clusterMeshBuilder;
}

void ref_ClusterMeshBuilderAddSurface(ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster, Surface *surface)
{
    clusterMeshBuilder->AddSurface(cluster, surface);
}

struct ClusterMeshData *ref_ClusterMeshDataCreate(struct ClusterMeshBuilder *clusterMeshBuilder)
{
    return new ClusterMeshData(*clusterMeshBuilder);
}

void ref_ClusterMeshDataDestroy(struct ClusterMeshData *clusterData)
{
    delete clusterData;
}

unsigned ref_ClusterMeshDataGetNumVertices(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_vertices.size();
}

MapModelVertex const *ref_ClusterMeshDataGetVertices(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_vertices.data();
}

unsigned ref_ClusterMeshDataGetNumIndices(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_indices.size();
}

VertexIndex const *ref_ClusterMeshDataGetIndices(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_indices.data();
}

unsigned ref_ClusterMeshDataGetNumMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_sections.size();
}

struct MapModelMeshSection const *ref_ClusterMeshDataGetMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster)
{
    return clusterData->m_clusterMeshes[cluster].m_sections.data();
}

ClusterBuilder *ref_ClusterBuilderCreate()
{
    return new ClusterBuilder();
}

void ref_ClusterBuilderDestroy(ClusterBuilder *clusterBuilder)
{
    delete clusterBuilder;
}

void ref_ClusterBuilderAddLeaf(struct ClusterBuilder *clusterBuilder, ClusterId cluster, mleaf_s *leaf)
{
    clusterBuilder->m_leafsFromCluster[cluster].push_back(leaf);
}

struct ClusterData *ref_ClusterDataCreate(struct ClusterBuilder *clusterBuilder)
{
    return new ClusterData(*clusterBuilder);
}

void ref_ClusterDataDestroy(struct ClusterData *clusterData)
{
    delete clusterData;
}

unsigned ref_ClusterDataGetNumClusters(struct ClusterData const *clusterData)
{
    return clusterData->m_clusterNodes.size();
}

struct mnode_s * const*ref_ClusterDataGetClusterNodes(struct ClusterData const *clusterData)
{
    return clusterData->m_clusterNodes.data();
}
