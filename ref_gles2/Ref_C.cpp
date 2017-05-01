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

unsigned ref_ClusterMeshBuilderGetNumSurfaces(struct ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster)
{
    if (clusterMeshBuilder->m_surfacesFromCluster.find(cluster) != clusterMeshBuilder->m_surfacesFromCluster.end())
    {
        return clusterMeshBuilder->m_surfacesFromCluster[cluster].size();
    }

    return 0;
}

struct Surface **ref_ClusterMeshBuilderGetSurfaces(struct ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster)
{
    if (clusterMeshBuilder->m_surfacesFromCluster.find(cluster) != clusterMeshBuilder->m_surfacesFromCluster.end())
    {
        return clusterMeshBuilder->m_surfacesFromCluster[cluster].data();
    }

    return NULL;
}


struct ClusterMeshData *ref_ClusterMeshDataCreate(struct ClusterMeshBuilder *clusterMeshBuilder)
{
    return new ClusterMeshData(*clusterMeshBuilder);
}

void ref_ClusterMeshDataDestroy(struct ClusterMeshData *clusterData)
{
    delete clusterData;
}

unsigned ref_ClusterMeshDataGetNumClusters(struct ClusterMeshData const *clusterData)
{
    return clusterData->m_clusterMeshes.size();
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
