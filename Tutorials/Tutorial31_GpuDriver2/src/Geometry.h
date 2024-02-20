#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"
#include "MeshSTL.h"
#include "ShaderDef.h"

#include <vector>


namespace Diligent
{

//#define CLUSTER_SIZE 256

struct IndirectDrawIndexArguments
{
	uint32_t mIndexCount;
	uint32_t mInstanceCount;
	uint32_t mStartIndex;
	uint32_t mVertexOffset;
	uint32_t mStartInstance;
};

struct ClusterCompact
{
    uint32_t triangleCount;
    uint32_t clusterStart;
};

struct Cluster
{
    float3 aabbMin, aabbMax;
};

struct ClusterContainer
{
    uint32_t        clusterCount;
    std::vector<ClusterCompact> clusterCompacts;
    std::vector<Cluster>        clusters;
};

struct Scene
{
    std::vector<MeshSTL>          mStlModel;
    std::vector<ClusterContainer> mMeshes;
};

void createClusters(const MeshSTL& rawMesh, IndirectDrawIndexArguments* draw, ClusterContainer* subMesh);

} // namespace Diligent