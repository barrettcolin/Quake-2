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

unsigned ref_ClusterMeshDataGetNumVertices(struct ClusterMeshData const *clusterData, ClusterId cluster);

struct MapModelVertex const *ref_ClusterMeshDataGetVertices(struct ClusterMeshData const *clusterData, ClusterId cluster);

unsigned ref_ClusterMeshDataGetNumIndices(struct ClusterMeshData const *clusterData, ClusterId cluster);

VertexIndex const *ref_ClusterMeshDataGetIndices(struct ClusterMeshData const *clusterData, ClusterId cluster);

unsigned ref_ClusterMeshDataGetNumMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster);

struct MapModelMeshSection const *ref_ClusterMeshDataGetMeshSections(struct ClusterMeshData const *clusterData, ClusterId cluster);

// ClusterBuilder
struct ClusterBuilder *ref_ClusterBuilderCreate();

void ref_ClusterBuilderDestroy(struct ClusterBuilder *clusterBuilder);

void ref_ClusterBuilderAddLeaf(struct ClusterBuilder *clusterBuilder, ClusterId cluster, struct mleaf_s *leaf);

// ClusterData
struct ClusterData *ref_ClusterDataCreate(struct ClusterBuilder *clusterBuilder);

void ref_ClusterDataDestroy(struct ClusterData *clusterData);

unsigned ref_ClusterDataGetNumClusters(struct ClusterData const *clusterData);

struct mnode_s *const *ref_ClusterDataGetClusterNodes(struct ClusterData const *clusterData);

#if defined (__cplusplus)
} // extern "C"
#endif
