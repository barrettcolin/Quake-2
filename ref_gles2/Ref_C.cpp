#include "Ref_C.h"

#include "MapModel.h"

void ref_SurfacePolySetVertex(SurfacePoly *surfacePoly, unsigned vertexIndex, float x, float y, float z, float s0, float t0, float s1, float t1)
{
    surfacePoly->SetVertex(vertexIndex, x, y, z, s0, t0, s1, t1);
}

ClusterMeshBuilder *ref_ClusterMeshBuilderCreate()
{
    return new ClusterMeshBuilder();
}

void ref_ClusterMeshBuilderDestroy(ClusterMeshBuilder *clusterMeshBuilder)
{
    delete clusterMeshBuilder;
}

SurfacePoly *ref_ClusterMeshBuilderAllocatePoly(ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster, TextureId lightMap, TextureId baseMap, unsigned numVertices)
{
    return &clusterMeshBuilder->AllocatePoly(cluster, lightMap, baseMap, numVertices);
}

struct ClusterMeshData *ref_ClusterMeshDataCreate(struct ClusterMeshBuilder *clusterMeshBuilder)
{
    return new ClusterMeshData(*clusterMeshBuilder);
}

void ref_ClusterMeshDataDestroy(struct ClusterMeshData *clusterData)
{
    delete clusterData;
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

unsigned ref_ClusterDataGetNumClusters(struct ClusterData *clusterData)
{
    return clusterData->m_clusterNodes.size();
}

struct mnode_s **ref_ClusterDataGetClusterNodes(struct ClusterData *clusterData)
{
    return clusterData->m_clusterNodes.data();
}
