#pragma once

#include "Types_C.h"

#if defined (__cplusplus)
extern "C" {
#endif

// BuilderSurf
void ref_SurfacePolySetVertex(struct SurfacePoly *surfacePoly, unsigned vertexIndex, float x, float y, float z, float s0, float t0, float s1, float t1);

// ClusterMeshBuilder
struct ClusterMeshBuilder *ref_ClusterMeshBuilderCreate();

void ref_ClusterMeshBuilderDestroy(struct ClusterMeshBuilder *clusterMeshBuilder);

struct SurfacePoly *ref_ClusterMeshBuilderAllocatePoly(struct ClusterMeshBuilder *clusterMeshBuilder, ClusterId cluster, TextureId lightMap, TextureId baseMap, unsigned numVertices);

// ClusterMeshData
struct ClusterMeshData *ref_ClusterMeshDataCreate(struct ClusterMeshBuilder *clusterMeshBuilder);

void ref_ClusterMeshDataDestroy(struct ClusterMeshData *clusterData);

// ClusterBuilder
struct ClusterBuilder *ref_ClusterBuilderCreate();

void ref_ClusterBuilderDestroy(struct ClusterBuilder *clusterBuilder);

void ref_ClusterBuilderAddLeaf(struct ClusterBuilder *clusterBuilder, ClusterId cluster, struct mleaf_s *leaf);

// ClusterData
struct ClusterData *ref_ClusterDataCreate(struct ClusterBuilder *clusterBuilder);

void ref_ClusterDataDestroy(struct ClusterData *clusterData);

unsigned ref_ClusterDataGetNumClusters(struct ClusterData *clusterData);

struct mnode_s **ref_ClusterDataGetClusterNodes(struct ClusterData *clusterData);

#if defined (__cplusplus)
} // extern "C"
#endif
