#pragma once

#include "Types_C.h"

#if defined (__cplusplus)
extern "C" {
#endif

// ClusterMeshBuilder
struct ClusterMeshBuilder *ref_ClusterMeshBuilderCreate();

void ref_ClusterMeshBuilderDestroy(struct ClusterMeshBuilder *clusterMeshBuilder);

void ref_ClusterMeshBuilderAddSurface(struct ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster, struct Surface *surface);

// ClusterMeshData
struct ClusterMeshData *ref_ClusterMeshDataCreate(struct ClusterMeshBuilder *clusterMeshBuilder);

void ref_ClusterMeshDataDestroy(struct ClusterMeshData *clusterData);

unsigned ref_ClusterMeshDataGetNumClusters(struct ClusterMeshData const *clusterData);

unsigned ref_ClusterMeshDataGetNumVertices(struct ClusterMeshData const *clusterData, ClusterId cluster);

struct MapModelVertex const *ref_ClusterMeshDataGetVertices(struct ClusterMeshData const *clusterData, ClusterId cluster);

unsigned ref_ClusterMeshDataGetNumIndices(struct ClusterMeshData const *clusterData, ClusterId cluster);

VertexIndex const *ref_ClusterMeshDataGetIndices(struct ClusterMeshData const *clusterData, ClusterId cluster);

unsigned ref_ClusterMeshDataGetNumMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster);

struct MapModelMeshSection const *ref_ClusterMeshDataGetMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster);

#if defined (__cplusplus)
} // extern "C"
#endif
