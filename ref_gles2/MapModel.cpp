#include "MapModel.h"

#include <algorithm>
#include <cassert>

void ClusterMeshBuilder::AddSurface(ClusterId cluster, Surface *surface)
{
    if (m_surfaces.find(surface) != m_surfaces.end())
    {
        return;
    }

    m_surfaces.insert(surface);

    m_surfacesFromCluster[cluster][surface->lightmaptexturenum][surface->texinfo->image].m_surfs.push_back(surface);
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

    for(const auto& clusterPair : clusterMeshBuilder.m_surfacesFromCluster)
    {
        ClusterId clusterIndex = clusterPair.first;

        for (const auto& lightPair : clusterPair.second)
        {
            int lightMapIndex = lightPair.first;

            for (const auto& basePair : lightPair.second)
            {
                struct image_s const *baseMap = basePair.first;

                if (basePair.second.m_surfs.size() == 0)
                    continue;

                ClusterMesh& clusterMesh = m_clusterMeshes[clusterIndex];
                MapModelMeshSection& meshSection = *(clusterMesh.m_sections.emplace(clusterMesh.m_sections.end()));
                meshSection.m_lightMap = lightMapIndex;
                meshSection.m_texInfo = basePair.second.m_surfs[0]->texinfo; // all texinfos reference same image

                bool first = true;
                meshSection.m_firstStripIndex = clusterMesh.m_indices.size();
                for (const auto& surface : basePair.second.m_surfs)
                {
                    for (Poly *poly = surface->polys; poly; poly = poly->next)
                    {
                        unsigned firstIndex = clusterMesh.m_vertices.size();
                        unsigned numVertices = poly->numverts;
                        clusterMesh.m_vertices.insert(clusterMesh.m_vertices.end(), poly->verts, poly->verts + numVertices);

                        if (!first)
                        {
                            clusterMesh.m_indices.push_back(clusterMesh.m_indices.back());
                            clusterMesh.m_indices.push_back(firstIndex);
                            meshSection.m_numStripIndices += 2;
                        }

                        // e.g. want to produce sequence 0, 1, 7, 2, 6, 3, 5, 4
                        clusterMesh.m_indices.push_back(firstIndex);
                        for (unsigned i = 1; i < numVertices; ++i)
                            clusterMesh.m_indices.push_back(((i % 2 == 0) ? (numVertices - (i / 2)) : ((i / 2) + 1)) + firstIndex);

                        meshSection.m_numStripIndices += numVertices;

                        first = false;
                    }
                }
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
