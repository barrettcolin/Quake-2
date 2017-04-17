#include "MapModel.h"

#include <cassert>

SurfacePoly::SurfacePoly(unsigned numVertices)
    : m_vertices(numVertices)
{

}

void SurfacePoly::SetVertex(unsigned vertexIndex, float x, float y, float z, float s0, float t0, float s1, float t1)
{
    SurfaceVertex& vert = m_vertices[vertexIndex];
    {
        vert.x = x; vert.y = y; vert.z = z;
        vert.s0 = s0; vert.t0 = t0;
        vert.s1 = s1; vert.t1 = t1;
    }
}

SurfacePoly& ClusterMeshBuilder::AllocatePoly(ClusterId cluster, uint32_t lightMap, uint32_t baseMap, unsigned numVertices)
{
    std::vector<SurfacePoly>& surfaces = m_surfacesFromCluster[cluster][lightMap][baseMap];
    return *surfaces.emplace(surfaces.end(), numVertices);
}

ClusterMeshData::ClusterMeshData(const ClusterMeshBuilder& clusterMeshBuilder)
{
    // Calc num clusters
    int maxClusterId = 0;
    for (auto clusterData = clusterMeshBuilder.m_surfacesFromCluster.begin();
        clusterData != clusterMeshBuilder.m_surfacesFromCluster.end();
        ++clusterData)
    {
        int clusterId = clusterData->first;
        if (clusterId > maxClusterId)
            maxClusterId = clusterId;
    }

    // Populate m_clusterMeshes
    int numClusters = maxClusterId + 1;
    m_clusterMeshes.resize(numClusters);

    for (auto clusterData = clusterMeshBuilder.m_surfacesFromCluster.begin();
        clusterData != clusterMeshBuilder.m_surfacesFromCluster.end(); 
        ++clusterData)
    {
        ClusterId clusterIndex = clusterData->first;

        for (auto lightMapData = clusterData->second.begin(); 
            lightMapData != clusterData->second.end();
            ++lightMapData)
        {
            TextureId lightMapIndex = lightMapData->first;

            for (auto baseMapData = lightMapData->second.begin();
                baseMapData != lightMapData->second.end();
                ++baseMapData)
            {
                TextureId baseMapIndex = baseMapData->first;

                auto mesh = m_clusterMeshes[clusterIndex].emplace(m_clusterMeshes[clusterIndex].end());
                mesh->m_lightMap = lightMapIndex;
                mesh->m_baseMap = baseMapIndex;
                mesh->m_polys = baseMapData->second;
            }
        }
    }
}

ClusterData::ClusterData(const ClusterBuilder& clusterBuilder)
{
    // Calc num clusters
    int maxClusterId = 0;
    for (auto clusterData = clusterBuilder.m_leafsFromCluster.begin();
        clusterData != clusterBuilder.m_leafsFromCluster.end();
        ++clusterData)
    {
        int clusterId = clusterData->first;
        if (clusterId > maxClusterId)
            maxClusterId = clusterId;
    }

    // Populate m_clusterNodes
    int numClusters = maxClusterId + 1;
    m_clusterNodes.resize(numClusters);

    for (auto clusterData = clusterBuilder.m_leafsFromCluster.begin();
        clusterData != clusterBuilder.m_leafsFromCluster.end(); 
        ++clusterData)
    {
        ClusterId cluster = clusterData->first;
        const ClusterBuilder::Leafs& leafs = clusterData->second;
        unsigned numLeafs = leafs.size();

        if (numLeafs == 0)
            continue;

        m_clusterNodes[cluster] = leafs[0]->m_parent;

        for (unsigned j = 1; j < numLeafs; ++j)
        {
            mnode_s *clusterNode = m_clusterNodes[j];
            mleaf_s *leaf = leafs[j];
            mnode_s *leafNode = leaf->m_parent;

            if (!leafNode)
                continue;

            while (clusterNode)
            {
                if (clusterNode->m_parent == leafNode)
                {
                    m_clusterNodes[cluster] = leafNode;
                    break;
                }
                clusterNode = clusterNode->m_parent;
            }
        }
    }
}
