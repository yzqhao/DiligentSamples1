#include "Geometry.h"

namespace Diligent
{

void createClusters(const MeshSTL& rawMesh, IndirectDrawIndexArguments* draw, ClusterContainer* mesh)
{
#define makeVec3(v) (float3((v).x, (v).y, (v).z))

    // 12 KiB stack space
    struct Triangle
    {
        float3 vtx[3];
    };

    Triangle triangleCache[CLUSTER_SIZE * 3];

    uint32_t* indices   = (uint32_t*)rawMesh.tris.data();
    float3*   positions = (float3*)rawMesh.vtx.data();

    const int triangleCount = draw->mIndexCount / 3;
    const int clusterCount  = (triangleCount + CLUSTER_SIZE - 1) / CLUSTER_SIZE;

    mesh->clusterCount = clusterCount;
    mesh->clusterCompacts.resize(clusterCount);
    mesh->clusters.resize(clusterCount);

    for (int i = 0; i < clusterCount; ++i)
    {
        const int clusterStart = i * CLUSTER_SIZE;
        const int clusterEnd   = std::min(clusterStart + CLUSTER_SIZE, triangleCount);

        const int clusterTriangleCount = clusterEnd - clusterStart;

        // Load all triangles into our local cache
        for (int triangleIndex = clusterStart; triangleIndex < clusterEnd; ++triangleIndex)
        {
            triangleCache[triangleIndex - clusterStart].vtx[0] =
                makeVec3(positions[indices[draw->mStartIndex + triangleIndex * 3]]);
            triangleCache[triangleIndex - clusterStart].vtx[1] =
                makeVec3(positions[indices[draw->mStartIndex + triangleIndex * 3 + 1]]);
            triangleCache[triangleIndex - clusterStart].vtx[2] =
                makeVec3(positions[indices[draw->mStartIndex + triangleIndex * 3 + 2]]);
        }

        float3 aabbMin = float3(INFINITY, INFINITY, INFINITY);
        float3 aabbMax = -aabbMin;

        for (int triangleIndex = 0; triangleIndex < clusterTriangleCount; ++triangleIndex)
        {
            const auto& triangle = triangleCache[triangleIndex];
            for (int j = 0; j < 3; ++j)
            {
                aabbMin = min(aabbMin, triangle.vtx[j]);
                aabbMax = min(aabbMax, triangle.vtx[j]);
            }
        }

        mesh->clusters[i].aabbMax = (aabbMax);
        mesh->clusters[i].aabbMin = (aabbMin);

        mesh->clusterCompacts[i].triangleCount = clusterTriangleCount;
        mesh->clusterCompacts[i].clusterStart  = clusterStart;
    }
}

}