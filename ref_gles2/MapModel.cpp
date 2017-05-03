#include "MapModel.h"

#include <algorithm>
#include <cassert>

void ClusterMeshBuilder::AddClusterSurface(ClusterId cluster, Surface *surface)
{
    if (m_surfaces.find(surface) != m_surfaces.end())
    {
        return;
    }

    m_surfaces.insert(surface);

    m_surfacesFromCluster[cluster].push_back(surface);
}

ClusterMeshData::ClusterMeshData(const ClusterMeshBuilder& clusterMeshBuilder)
{
    // Calc num clusters
    int maxClusterId = 0;
    for (const auto& clusterSurfaces : clusterMeshBuilder.m_surfacesFromCluster)
    {
        int clusterId = clusterSurfaces.first;
        if (clusterId > maxClusterId)
            maxClusterId = clusterId;
    }

    // Populate m_clusterMeshes
    int numClusters = maxClusterId + 1;
    m_clusterMeshes.resize(numClusters);

    for (const auto& clusterSurface : clusterMeshBuilder.m_surfacesFromCluster)
    {
        ClusterId cluster = clusterSurface.first;
        ClusterMesh& clusterMesh = m_clusterMeshes[cluster];

        std::unordered_map<int, std::unordered_map<image_s *, std::vector<Surface*> > > surfacesFromImageFromLightmap;
        for (const auto& surf : clusterSurface.second)
        {
            surfacesFromImageFromLightmap[surf->lightmaptexturenum][surf->texinfo->image].push_back(surf);
        }

        for (const auto& lightmapSurfacesFromImage : surfacesFromImageFromLightmap)
        {
            int lightMapIndex = lightmapSurfacesFromImage.first;

            for (const auto& imageSurfaces : lightmapSurfacesFromImage.second)
            {
                image_s *image = imageSurfaces.first;

                MapModelMeshSection& meshSection = *(clusterMesh.m_sections.emplace(clusterMesh.m_sections.end()));
                meshSection.m_lightMap = lightMapIndex;
                meshSection.m_texInfo = imageSurfaces.second[0]->texinfo; // all texinfos reference same image

                bool first = true;
                meshSection.m_firstStripIndex = clusterMesh.m_indices.size();
                for (const auto& surface : imageSurfaces.second)
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

                        // e.g. want to produce sequence 0, 1, 7, 2, 6, 3, 5, 4 (even), 0, 7, 1, 6, 2, 5, 3, 4 (odd)
                        std::vector<VertexIndex> indices(numVertices);
                        indices[0] = firstIndex;

                        // flip if we're appending to an odd number of triangles
                        const bool flipWinding = ((meshSection.m_numStripIndices % 2) != 0);

                        if (flipWinding)
                        {
                            for (unsigned i = 1; i < numVertices; ++i)
                                indices[i] = ((i % 2 == 0) ? (((i - 1) / 2) + 1) : (numVertices - ((i + 1) / 2))) + firstIndex;
                        }
                        else
                        {
                            for (unsigned i = 1; i < numVertices; ++i)
                                indices[i] = ((i % 2 == 0) ? (numVertices - (i / 2)) : ((i / 2) + 1)) + firstIndex;
                        }

                        clusterMesh.m_indices.insert(clusterMesh.m_indices.end(), indices.begin(), indices.end());
                        meshSection.m_numStripIndices += numVertices;

                        first = false;
                    }
                }
            }
        }
    }
}
