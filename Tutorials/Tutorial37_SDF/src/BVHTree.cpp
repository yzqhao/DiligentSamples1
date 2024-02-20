#include "BVHTree.h"

#include <fstream>

namespace Diligent
{

void adjustAABB(BoundBox* ownerAABB, const float3& point)
{
    ownerAABB->Min.x = std::min(point.x, ownerAABB->Min.x);
    ownerAABB->Min.y = std::min(point.y, ownerAABB->Min.y);
    ownerAABB->Min.z = std::min(point.z, ownerAABB->Min.z);

    ownerAABB->Max.x = std::max(point.x, ownerAABB->Max.x);
    ownerAABB->Max.y = std::max(point.y, ownerAABB->Max.y);
    ownerAABB->Max.z = std::max(point.z, ownerAABB->Max.z);
}

void adjustAABB(BoundBox* ownerAABB, const BoundBox& otherAABB)
{
    ownerAABB->Min.x = std::min(otherAABB.Min.x, ownerAABB->Min.x);
    ownerAABB->Min.y = std::min(otherAABB.Min.y, ownerAABB->Min.y);
    ownerAABB->Min.z = std::min(otherAABB.Min.z, ownerAABB->Min.z);

    ownerAABB->Max.x = std::max(otherAABB.Max.x, ownerAABB->Max.x);
    ownerAABB->Max.y = std::max(otherAABB.Max.y, ownerAABB->Max.y);
    ownerAABB->Max.z = std::max(otherAABB.Max.z, ownerAABB->Max.z);
}

void alignAABB(BoundBox* ownerAABB, float alignment)
{
    float3 boxMin = ownerAABB->Min / alignment;
    boxMin        = float3(floorf(boxMin.x) * alignment, floorf(boxMin.y) * alignment, 0.0f);
    float3 boxMax = ownerAABB->Max / alignment;
    boxMax        = float3(ceilf(boxMax.x) * alignment, ceilf(boxMax.y) * alignment, 0.0f);
    *ownerAABB    = {boxMin, boxMax};
}

float3 calculateAABBSize(const BoundBox* ownerAABB)
{
    return ownerAABB->Max - ownerAABB->Min;
}

float3 calculateAABBExtent(const BoundBox* ownerAABB)
{
    return 0.5f * (ownerAABB->Max - ownerAABB->Min);
}

float3 calculateAABBCenter(const BoundBox* ownerAABB)
{
    return (ownerAABB->Max + ownerAABB->Min) * 0.5f;
}

void RayIntersectTriangle(const Ray& ray, const Triangle& triangle, Intersection& outIntersection)
{
    float3 P_V = cross(ray.direction, triangle.mE2);

    float P_dot_E1 = dot(P_V, triangle.mE1);

    if (P_dot_E1 == 0.f)
    {
        return;
    }

    float3 S_V = ray.origin - triangle.mV0;

    float u_val = dot(P_V, S_V) / P_dot_E1;

    if (u_val < Epilson || u_val > 1.f)
    {
        return;
    }

    float3 Q_V = cross(S_V, triangle.mE1);

    float v_val = dot(ray.direction, Q_V) / P_dot_E1;

    if (v_val < Epilson || (v_val + u_val) > 1.f)
    {
        return;
    }

    float t_val = dot(triangle.mE2, Q_V) / P_dot_E1;

    if (t_val < Epilson)
    {
        return;
    }

    if (t_val < outIntersection.mIntersection_TVal)
    {
        outIntersection.mIsIntersected     = true;
        outIntersection.mIntersection_TVal = t_val;
        outIntersection.mHittedPos         = ray.Eval(t_val);
        outIntersection.mHittedNormal      = normalize(
            ((1.f - u_val - v_val) * triangle.mN0 + u_val * triangle.mN1 + v_val * triangle.mN2));
        outIntersection.mIntersectedTriangle = &triangle;
    }
}

bool RayIntersectsBox(const float3& origin, const float3& rayDirInv, const float3& BboxMin, const float3& BboxMax)
{
    const float3 nonInvT0 = (BboxMin - origin);
    const float3 nonInvT1 = (BboxMax - origin);
    const float3 t0       = float3(nonInvT0.x * rayDirInv.x, nonInvT0.y * rayDirInv.y, nonInvT0.z * rayDirInv.z);
    const float3 t1       = float3(nonInvT1.x * rayDirInv.x, nonInvT1.y * rayDirInv.y, nonInvT1.z * rayDirInv.z);

    const float3 tmax = float3(fmax(t0.x, t1.x), fmax(t0.y, t1.y), fmax(t0.z, t1.z));
    const float3 tmin = float3(fmin(t0.x, t1.x), fmin(t0.y, t1.y), fmin(t0.z, t1.z));

    const float a1 = fmin(tmax.x, fmin(tmax.y, tmax.z));
    const float a0 = fmax(fmax(tmin.x, tmin.y), fmax(tmin.z, 0.0f));

    return a1 >= a0;
}

void BVHTreeIntersectRayAux(BVHNode* rootNode, BVHNode* node, const Ray& ray, Intersection& outIntersection)
{
    if (!node)
    {
        return;
    }

    if (node->BoundingBox.InstanceID < 0.f)
    {
        bool intersects = RayIntersectsBox(ray.origin, float3(1.f / ray.direction.x, 1.f / ray.direction.y, 1.f / ray.direction.z),
                                           node->BoundingBox.mAABB.Min, node->BoundingBox.mAABB.Max);

        if (intersects)
        {
            BVHTreeIntersectRayAux(rootNode, node->Left, ray, outIntersection);
            BVHTreeIntersectRayAux(rootNode, node->Right, ray, outIntersection);
        }
    }
    else
    {
        RayIntersectTriangle(ray, node->BoundingBox.mTriangle, outIntersection);
    }
}

void AddMeshInstanceToBBOX16(BVHTree* pBvhTree, SDFMesh* pMesh, uint16_t* pMeshIndices, uint32_t groupNum, uint32_t meshGroupSize, uint32_t idxFirstMeshInGroup, BoundBox* pAABBFirstMeshInGroup, const float4x4& meshWorldMat)
{
    float*    positions = (float*)pMesh->pGeometry->m_VertexData[0].data();
    uint32_t* indices    = (uint32_t*)pMesh->pGeometry->m_IndexData.data();

    // handle individual submesh in current submesh group
    for (uint32_t meshNum = 0; meshNum < meshGroupSize; ++meshNum)
    {
        auto& primitive = pMesh->pGeometry->Scenes[pMesh->pGeometry->DefaultSceneId].LinearNodes[0]->pMesh->Primitives[meshNum];

        // if not true, the meshIndex is simply the groupNum since there are as many submesh groups as there are meshes
        //uint32_t meshIndex = pMesh->pSubMeshesIndices ? pMesh->pSubMeshesIndices[idxFirstMeshInGroup + meshNum] : groupNum;

        uint32_t startIndex = primitive.FirstIndex;
        uint32_t indexCount = primitive.IndexCount;

        for (uint32_t i = 0; i < indexCount; i += 3)
        {
            int idx1 = indices[startIndex + i * 3] * 11;
            int idx2 = indices[startIndex + i * 3 + 1] * 11;
            int idx3 = indices[startIndex + i * 3 + 2] * 11;

            float3 position1 = float3(positions[idx1], positions[idx1 + 1], positions[idx1 + 2]);
            float3 position2 = float3(positions[idx2], positions[idx2 + 1], positions[idx2 + 2]);
            float3 position3 = float3(positions[idx3], positions[idx3 + 1], positions[idx3 + 2]);

            float3 pos0 = (meshWorldMat * float4(position1 * MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));
            float3 pos1 = (meshWorldMat * float4(position2 * MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));
            float3 pos2 = (meshWorldMat * float4(position3 * MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));

            //TODO: multiply normal by world mat
            float3 n0 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);
            float3 n1 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);
            float3 n2 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);

            adjustAABB(pAABBFirstMeshInGroup, pos0);
            adjustAABB(pAABBFirstMeshInGroup, pos1);
            adjustAABB(pAABBFirstMeshInGroup, pos2);

            pBvhTree->mBBOXDataList.push_back(BVHAABBox());
            BVHAABBox& bvhAABBOX = pBvhTree->mBBOXDataList.back();

            bvhAABBOX.mTriangle.Init(pos0, pos1, pos2, n0, n1, n2);
            bvhAABBOX.Expand(pos0);
            bvhAABBOX.Expand(pos1);
            bvhAABBOX.Expand(pos2);
            bvhAABBOX.InstanceID = 0;
        }
    }
}

void AddMeshInstanceToBBOX32(BVHTree* pBvhTree, SDFMesh* pMesh, uint32_t* pMeshIndices, uint32_t groupNum, uint32_t meshGroupSize, uint32_t idxFirstMeshInGroup, BoundBox* pAABBFirstMeshInGroup, const float4x4& meshWorldMat)
{
    float*    positions = (float*)pMesh->pGeometry->m_VertexData[0].data();
    uint32_t* indices   = (uint32_t*)pMesh->pGeometry->m_IndexData.data();

    // handle individual submesh in current submesh group
    for (uint32_t meshNum = 0; meshNum < meshGroupSize; ++meshNum)
    {
        auto& primitive = pMesh->pGeometry->Scenes[pMesh->pGeometry->DefaultSceneId].LinearNodes[0]->pMesh->Primitives[meshNum];

        // if not true, the meshIndex is simply the groupNum since there are as many submesh groups as there are meshes
        //uint32_t meshIndex = pMesh->pSubMeshesIndices ? pMesh->pSubMeshesIndices[idxFirstMeshInGroup + meshNum] : groupNum;

        uint32_t startIndex = primitive.FirstIndex;
        uint32_t indexCount = primitive.IndexCount;

        for (uint32_t i = 0; i < indexCount; i += 3)
        {
            int idx1 = indices[startIndex + i * 3] * 11;
            int idx2 = indices[startIndex + i * 3 + 1] * 11;
            int idx3 = indices[startIndex + i * 3 + 2] * 11;

            float3 position1 = float3(positions[idx1], positions[idx1 + 1], positions[idx1 + 2]);
            float3 position2 = float3(positions[idx2], positions[idx2 + 1], positions[idx2 + 2]);
            float3 position3 = float3(positions[idx3], positions[idx3 + 1], positions[idx3 + 2]);

            float3 pos0 = (meshWorldMat * float4((position1)*MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));
            float3 pos1 = (meshWorldMat * float4((position2)*MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));
            float3 pos2 = (meshWorldMat * float4((position3)*MESH_SCALE + float3(SAN_MIGUEL_OFFSETX, 0.f, 0.f), 1.0f));

            //TODO: multiply normal by world mat
            float3 n0 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);
            float3 n1 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);
            float3 n2 = float3(positions[idx1 + 3], positions[idx1 + 4], positions[idx1 + 5]);

            adjustAABB(pAABBFirstMeshInGroup, pos0);
            adjustAABB(pAABBFirstMeshInGroup, pos1);
            adjustAABB(pAABBFirstMeshInGroup, pos2);

            pBvhTree->mBBOXDataList.push_back(BVHAABBox());
            BVHAABBox& bvhAABBOX = pBvhTree->mBBOXDataList.back();

            bvhAABBOX.mTriangle.Init(pos0, pos1, pos2, n0, n1, n2);
            bvhAABBOX.Expand(pos0);
            bvhAABBOX.Expand(pos1);
            bvhAABBOX.Expand(pos2);
            bvhAABBOX.InstanceID = 0;
        }
    }
}


void SortAlongAxis(BVHTree* bvhTree, int32_t begin, int32_t end, int32_t axis)
{
    BVHAABBox* data  = bvhTree->mBBOXDataList.data() + begin;
    int32_t    count = end - begin + 1;

    if (axis == 0)
        std::qsort(data, count, sizeof(BVHAABBox), [](const void* a, const void* b) {
            const BVHAABBox* arg1 = static_cast<const BVHAABBox*>(a);
            const BVHAABBox* arg2 = static_cast<const BVHAABBox*>(b);

            float midPointA = arg1->Center[0];
            float midPointB = arg2->Center[0];

            if (midPointA < midPointB)
                return -1;
            else if (midPointA > midPointB)
                return 1;

            return 0;
        });
    else if (axis == 1)
        std::qsort(data, count, sizeof(BVHAABBox), [](const void* a, const void* b) {
            const BVHAABBox* arg1 = static_cast<const BVHAABBox*>(a);
            const BVHAABBox* arg2 = static_cast<const BVHAABBox*>(b);

            float midPointA = arg1->Center[1];
            float midPointB = arg2->Center[1];

            if (midPointA < midPointB)
                return -1;
            else if (midPointA > midPointB)
                return 1;

            return 0;
        });
    else
        std::qsort(data, count, sizeof(BVHAABBox), [](const void* a, const void* b) {
            const BVHAABBox* arg1 = static_cast<const BVHAABBox*>(a);
            const BVHAABBox* arg2 = static_cast<const BVHAABBox*>(b);

            float midPointA = arg1->Center[2];
            float midPointB = arg2->Center[2];

            if (midPointA < midPointB)
                return -1;
            else if (midPointA > midPointB)
                return 1;

            return 0;
        });
}


float CalculateSurfaceArea(const BVHAABBox& bbox)
{
    float3 extents = bbox.mAABB.Max - bbox.mAABB.Min;
    return (extents[0] * extents[1] + extents[1] * extents[2] + extents[2] * extents[0]) * 2.f;
}

void FindBestSplit(BVHTree* bvhTree, int32_t begin, int32_t end, int32_t& split, int32_t& axis, float& splitCost)
{
    int32_t count     = end - begin + 1;
    int32_t bestSplit = begin;
    //int32_t globalBestSplit = begin;
    splitCost = FLT_MAX;

    split = begin;
    axis  = 0;

    for (int32_t i = 0; i < 3; i++)
    {
        SortAlongAxis(bvhTree, begin, end, i);

        BVHAABBox boundsLeft;
        BVHAABBox boundsRight;

        for (int32_t indexLeft = 0; indexLeft < count; ++indexLeft)
        {
            int32_t indexRight = count - indexLeft - 1;

            boundsLeft.Expand(bvhTree->mBBOXDataList[begin + indexLeft]);

            boundsRight.Expand(bvhTree->mBBOXDataList[begin + indexRight]);

            float surfaceAreaLeft  = CalculateSurfaceArea(boundsLeft);
            float surfaceAreaRight = CalculateSurfaceArea(boundsRight);

            bvhTree->mBBOXDataList[begin + indexLeft].SurfaceAreaLeft   = surfaceAreaLeft;
            bvhTree->mBBOXDataList[begin + indexRight].SurfaceAreaRight = surfaceAreaRight;
        }

        float bestCost = FLT_MAX;
        for (int32_t mid = begin + 1; mid <= end; ++mid)
        {
            float surfaceAreaLeft  = bvhTree->mBBOXDataList[mid - 1].SurfaceAreaLeft;
            float surfaceAreaRight = bvhTree->mBBOXDataList[mid].SurfaceAreaRight;

            int32_t countLeft  = mid - begin;
            int32_t countRight = end - mid;

            float costLeft  = surfaceAreaLeft * (float)countLeft;
            float costRight = surfaceAreaRight * (float)countRight;

            float cost = costLeft + costRight;
            if (cost < bestCost)
            {
                bestSplit = mid;
                bestCost  = cost;
            }
        }

        if (bestCost < splitCost)
        {
            split     = bestSplit;
            splitCost = bestCost;
            axis      = i;
        }
    }
}
void CalculateBounds(BVHTree* bvhTree, int32_t begin, int32_t end, float3& outMinBounds, float3& outMaxBounds)
{
    outMinBounds = float3(FLT_MAX);
    outMaxBounds = float3(-FLT_MAX);

    for (int32_t i = begin; i <= end; ++i)
    {
        const float3& memberMinBounds = bvhTree->mBBOXDataList[i].mAABB.Min;
        const float3& memberMaxBounds = bvhTree->mBBOXDataList[i].mAABB.Max;
        outMinBounds                  = float3(
            fmin(memberMinBounds.x, outMinBounds.x),
            fmin(memberMinBounds.y, outMinBounds.y),
            fmin(memberMinBounds.z, outMinBounds.z));

        outMaxBounds = float3(
            fmax(memberMaxBounds.x, outMaxBounds.x),
            fmax(memberMaxBounds.y, outMaxBounds.y),
            fmax(memberMaxBounds.z, outMaxBounds.z));
    }
}


BVHNode* CreateBVHNodeSHA(BVHTree* bvhTree, int32_t begin, int32_t end, float parentSplitCost)
{
    int32_t count = end - begin + 1;

    float3 Min;
    float3 Max;

    CalculateBounds(bvhTree, begin, end, Min, Max);

    BVHNode* node = new BVHNode();
    node->SplitCost = 1;


    ++bvhTree->mBVHNodeCount;

    node->BoundingBox.Expand(Min);
    node->BoundingBox.Expand(Max);

    if (count == 1)
    {
        //this is a leaf node
        node->Left  = NULL;
        node->Right = NULL;

        node->BoundingBox.InstanceID = bvhTree->mBBOXDataList[begin].InstanceID;
        node->BoundingBox.mTriangle  = bvhTree->mBBOXDataList[begin].mTriangle;
    }
    else
    {
        ++bvhTree->mTransitionNodeCount;

        int32_t split;
        int32_t axis;
        float   splitCost;

        //find the best axis to sort along and where the split should be according to SAH
        FindBestSplit(bvhTree, begin, end, split, axis, splitCost);

        //sort along that axis
        SortAlongAxis(bvhTree, begin, end, axis);

        //create the two branches
        node->Left  = CreateBVHNodeSHA(bvhTree, begin, split - 1, splitCost);
        node->Right = CreateBVHNodeSHA(bvhTree, split, end, splitCost);

        //Access the child with the largest probability of collision first.
        float surfaceAreaLeft  = CalculateSurfaceArea(node->Left->BoundingBox);
        float surfaceAreaRight = CalculateSurfaceArea(node->Right->BoundingBox);

        if (surfaceAreaRight > surfaceAreaLeft)
        {
            BVHNode* temp = node->Right;
            node->Right   = node->Left;
            node->Left    = temp;
        }


        //this is an intermediate Node
        node->BoundingBox.InstanceID = -1;
    }

    return node;
}

void DeleteBVHTree(BVHNode* node)
{
    if (node)
    {
        if (node->Left)
        {
            DeleteBVHTree(node->Left);
        }

        if (node->Right)
        {
            DeleteBVHTree(node->Right);
        }

        node->~BVHNode();
        free(node);
    }
}

void GenerateSampleDirections(int32_t thetaSteps, int32_t phiSteps, SDFVolumeData::SampleDirectionsList& outDirectionsList, int32_t finalThetaModifier)
{

    for (int32_t theta = 0; theta < thetaSteps; ++theta)
    {
        for (int32_t phi = 0; phi < phiSteps; ++phi)
        {
            float random1 = randomFloat01();
            float random2 = randomFloat01();

            float thetaFrac = (theta + random1) / (float)thetaSteps;
            float phiFrac   = (phi + random2) / (float)phiSteps;

            float rVal = sqrt(1.0f - thetaFrac * thetaFrac);

            const float finalPhi = 2.0f * (float)PI * phiFrac;

            outDirectionsList.push_back(float3(cos(finalPhi) * rVal,
                                               sin(finalPhi) * rVal, thetaFrac * finalThetaModifier));
        }
    }
}

void DoCalculateMeshSDFTask(void* dataPtr, uintptr_t index)
{
    CalculateMeshSDFTask* task = (CalculateMeshSDFTask*)(dataPtr);

    const BoundBox& sdfVolumeBounds    = *task->mSDFVolumeBounds;
    const int3&     sdfVolumeDimension = *task->mSDFVolumeDimension;
    int32_t         zIndex             = task->mZIndex;
    float           sdfVolumeMaxDist   = task->mSDFVolumeMaxDist;

    const SDFVolumeData::SampleDirectionsList& directionsList = *task->mDirectionsList;
    //const SDFVolumeData::TriangeList& meshTrianglesList = *task->mMeshTrianglesList;

    SDFVolumeData::SDFVolumeList& sdfVolumeList = *task->mSDFVolumeList;

    BVHTree* bvhTree = task->mBVHTree;


    //float3 floatSDFVolumeDimension = float3((float)sdfVolumeDimension.x, (float)sdfVolumeDimension.y, (float)sdfVolumeDimension.z);

    float3 sdfVolumeBoundsSize = calculateAABBSize(&sdfVolumeBounds);

    float3 sdfVoxelSize(
        sdfVolumeBoundsSize.x / sdfVolumeDimension.x,
        sdfVolumeBoundsSize.y / sdfVolumeDimension.y,
        sdfVolumeBoundsSize.z / sdfVolumeDimension.z);

    float voxelDiameterSquared = dot(sdfVoxelSize, sdfVoxelSize);

    for (int32_t yIndex = 0; yIndex < sdfVolumeDimension.y; ++yIndex)
    {
        for (int32_t xIndex = 0; xIndex < sdfVolumeDimension.x; ++xIndex)
        {
            float3 offsettedIndex = float3((float)(xIndex) + 0.5f, float(yIndex) + 0.5f, float(zIndex) + 0.5f);

            float3 voxelPos = float3(offsettedIndex.x * sdfVoxelSize.x,
                                     offsettedIndex.y * sdfVoxelSize.y, offsettedIndex.z * sdfVoxelSize.z) +
                sdfVolumeBounds.Min;

            int32_t outIndex = (zIndex * sdfVolumeDimension.y *
                                    sdfVolumeDimension.x +
                                yIndex * sdfVolumeDimension.x + xIndex);


            float   minDistance = sdfVolumeMaxDist;
            int32_t hit         = 0;
            int32_t hitBack     = 0;

            for (uint32_t sampleIndex = 0; sampleIndex < directionsList.size(); ++sampleIndex)
            {
                float3 rayDir = directionsList[sampleIndex];
                //float3 endPos = voxelPos + rayDir * sdfVolumeMaxDist;

                Ray newRay(voxelPos, rayDir);


                bool intersectWithBbox = RayIntersectsBox(newRay.origin,
                                                          float3(1.f / newRay.direction.x, 1.f / newRay.direction.y, 1.f / newRay.direction.z), sdfVolumeBounds.Min, sdfVolumeBounds.Max);

                //if we pass the cheap bbox testing
                if (intersectWithBbox)
                {
                    Intersection meshTriangleIntersect;
                    //optimized version
                    BVHTreeIntersectRayAux(bvhTree->mRootNode, bvhTree->mRootNode, newRay, meshTriangleIntersect);
                    if (meshTriangleIntersect.mIsIntersected)
                    {
                        ++hit;
                        const float3& hitNormal = meshTriangleIntersect.mHittedNormal;
                        if (dot(rayDir, hitNormal) > 0 && !task->mIsTwoSided)
                        {
                            ++hitBack;
                        }

                        const float3 finalEndPos = newRay.Eval(
                            meshTriangleIntersect.mIntersection_TVal);

                        float newDist = length(newRay.origin - finalEndPos);

                        if (newDist < minDistance)
                        {
                            minDistance = newDist;
                        }
                    }
                }
            }

            //


            float unsignedDist = minDistance;

            //if 50% hit backface, we consider the voxel sdf value to be inside the mesh
            minDistance *= (hit == 0 || hitBack < (directionsList.size() * 0.5f)) ? 1 : -1;

            //if we are very close to the surface and 95% of our rays hit backfaces, the sdf value
            //is inside the mesh
            if ((unsignedDist * unsignedDist) < voxelDiameterSquared && hitBack > 0.95f * hit)
            {
                minDistance = -unsignedDist;
            }

            minDistance = fmin(minDistance, sdfVolumeMaxDist);
            //float maxExtent = fmax(fmax(sdfVolumeBounds.GetExtent().x,
            //sdfVolumeBounds.GetExtent().y), sdfVolumeBounds.GetExtent().z);
            float3 sdfVolumeBoundsExtent = calculateAABBExtent(&sdfVolumeBounds);
            float  maxExtent             = maxElem(sdfVolumeBoundsExtent);

            float volumeSpaceDist = minDistance / maxExtent;



            sdfVolumeList[outIndex] = volumeSpaceDist;
        }
    }
}

bool GenerateVolumeDataFromFile(SDFVolumeData** ppOutVolumeData, const std::string& sdffile)
{
    std::ifstream is(sdffile.c_str(), std::ifstream::binary);
    if (!is)
    {
        return false;
    }

    *ppOutVolumeData             = new (SDFVolumeData);
    SDFVolumeData& outVolumeData = **ppOutVolumeData;

    int32_t x, y, z;
    is.read((char*)&x, sizeof(int32_t));
    is.read((char*)&y, sizeof(int32_t));
    is.read((char*)&z, sizeof(int32_t));
    outVolumeData.mSDFVolumeSize.x = (x);
    outVolumeData.mSDFVolumeSize.y = (y);
    outVolumeData.mSDFVolumeSize.z = (z);

    uint32_t finalSDFVolumeDataCount = outVolumeData.mSDFVolumeSize.x * outVolumeData.mSDFVolumeSize.y * outVolumeData.mSDFVolumeSize.z;

    outVolumeData.mSDFVolumeList.resize(finalSDFVolumeDataCount);

    is.read((char*) & outVolumeData.mSDFVolumeList[0], finalSDFVolumeDataCount * sizeof(float));
    float3 Min;
    float3 Max;
    is.read((char*)&Min, sizeof(float3));
    is.read((char*)&Max, sizeof(float3));
    outVolumeData.mLocalBoundingBox.Min = (Min);
    outVolumeData.mLocalBoundingBox.Max = (Max);
    is.read((char*)&outVolumeData.mIsTwoSided, sizeof(bool));
    is.close();

    return true;
}

void GenerateVolumeDataFromMesh(SDFVolumeData** ppOutVolumeData, SDFMesh* pMainMesh, const std::string& sdffile, uint32_t groupNum, uint32_t meshGroupSize,
 uint32_t idxFirstMeshInGroup, float sdfResolutionScale)
{
    if (GenerateVolumeDataFromFile(ppOutVolumeData, sdffile))
        return;

    *ppOutVolumeData = new(SDFVolumeData);

    SDFVolumeData& outVolumeData = **ppOutVolumeData;

    //for now assume all triangles are valid and useable
    int3 maxNumVoxelsOneDimension;
    int3 minNumVoxelsOneDimension;

    if (true)
    {
        maxNumVoxelsOneDimension = int3(
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_X,
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Y,
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Z);

        minNumVoxelsOneDimension = int3(
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_X,
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Y,
            SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Z);
    }
    else
    {
        maxNumVoxelsOneDimension = int3(
            SDF_MAX_VOXEL_ONE_DIMENSION_X,
            SDF_MAX_VOXEL_ONE_DIMENSION_Y,
            SDF_MAX_VOXEL_ONE_DIMENSION_Z);
        minNumVoxelsOneDimension = int3(
            SDF_MIN_VOXEL_ONE_DIMENSION_X,
            SDF_MIN_VOXEL_ONE_DIMENSION_Y,
            SDF_MIN_VOXEL_ONE_DIMENSION_Z);
    }

    const float voxelDensity = 1.0f;

    const float numVoxelPerLocalSpaceUnit = voxelDensity * sdfResolutionScale;

    BoundBox subMeshBBox = {float3(FLT_MAX, FLT_MAX, FLT_MAX), float3(-FLT_MAX, -FLT_MAX, -FLT_MAX)};

    BVHTree bvhTree;

    if (false)//(pMainMesh->pGeometry->mIndexType == INDEX_TYPE_UINT16)
    {
        uint16_t* indices = (uint16_t*)pMainMesh->pGeometry->m_IndexData.data();
        AddMeshInstanceToBBOX16(&bvhTree, pMainMesh, indices,
                                groupNum, meshGroupSize, idxFirstMeshInGroup, &subMeshBBox, float4x4::Identity());
    }
    else
    {
        uint32_t* indices = (uint32_t*)pMainMesh->pGeometry->m_IndexData.data();
        AddMeshInstanceToBBOX32(&bvhTree, pMainMesh, indices,
                                groupNum, meshGroupSize, idxFirstMeshInGroup, &subMeshBBox, float4x4::Identity());
    }

    bvhTree.mRootNode = CreateBVHNodeSHA(&bvhTree, 0, (int32_t)bvhTree.mBBOXDataList.size() - 1, FLT_MAX);

    float3 subMeshExtent = 0.5f * (subMeshBBox.Max - subMeshBBox.Min);

    float maxExtentSize = maxElem(subMeshExtent);

    float3 minNewExtent(0.2f * maxExtentSize);
    float3 standardExtentSize = 4.f * subMeshExtent;
    float3 dynamicNewExtent(standardExtentSize.x / minNumVoxelsOneDimension.x,
                          standardExtentSize.y / minNumVoxelsOneDimension.y, standardExtentSize.z / minNumVoxelsOneDimension.z);

    float3 finalNewExtent = subMeshExtent + float3(fmax(minNewExtent.x, dynamicNewExtent.x), fmax(minNewExtent.y, dynamicNewExtent.y), fmax(minNewExtent.z, dynamicNewExtent.z));

    float3 subMeshBBoxCenter = (subMeshBBox.Max + subMeshBBox.Min) * 0.5f;

    BoundBox newSDFVolumeBound;
    newSDFVolumeBound.Min = subMeshBBoxCenter - finalNewExtent;
    newSDFVolumeBound.Max = subMeshBBoxCenter + finalNewExtent;

    float3 newSDFVolumeBoundSize   = newSDFVolumeBound.Max - newSDFVolumeBound.Min;
    float3 newSDFVolumeBoundExtent = 0.5f * (newSDFVolumeBound.Max - newSDFVolumeBound.Min);

    float newSDFVolumeMaxDistance = length(newSDFVolumeBoundExtent);
    float3  dynamicDimension        = float3(
        newSDFVolumeBoundSize.x * numVoxelPerLocalSpaceUnit,
        newSDFVolumeBoundSize.y * numVoxelPerLocalSpaceUnit,
        newSDFVolumeBoundSize.z * numVoxelPerLocalSpaceUnit);

    int3 finalSDFVolumeDimension(
        clamp((int32_t)(dynamicDimension.x), minNumVoxelsOneDimension.x, maxNumVoxelsOneDimension.x),
        clamp((int32_t)(dynamicDimension.y), minNumVoxelsOneDimension.y, maxNumVoxelsOneDimension.y),
        clamp((int32_t)(dynamicDimension.z), minNumVoxelsOneDimension.z, maxNumVoxelsOneDimension.z));

    uint32_t finalSDFVolumeDataCount = finalSDFVolumeDimension.x *
        finalSDFVolumeDimension.y *
        finalSDFVolumeDimension.z;

    outVolumeData.mSDFVolumeList.resize(finalSDFVolumeDataCount);

    // here we begin our stratified sampling calculation
    const uint32_t numVoxelDistanceSample = SDF_STRATIFIED_DIRECTIONS_NUM;

    SDFVolumeData::SampleDirectionsList sampleDirectionsList;

    int32_t thetaStep = (int32_t)floor((sqrt((float)numVoxelDistanceSample / (PI * 2.f))));
    int32_t phiStep   = (int32_t)floor((float)thetaStep * PI);

    sampleDirectionsList.reserve(thetaStep * phiStep * 2);

    GenerateSampleDirections(thetaStep, phiStep, sampleDirectionsList);

    SDFVolumeData::SampleDirectionsList otherHemisphereSampleDirectionList;
    GenerateSampleDirections(thetaStep, phiStep, sampleDirectionsList, -1);

    CalculateMeshSDFTask calculateMeshSDFTask = {};
    calculateMeshSDFTask.mDirectionsList      = &sampleDirectionsList;
    calculateMeshSDFTask.mSDFVolumeBounds     = &newSDFVolumeBound;
    calculateMeshSDFTask.mSDFVolumeDimension  = &finalSDFVolumeDimension;
    calculateMeshSDFTask.mSDFVolumeMaxDist    = newSDFVolumeMaxDistance;
    calculateMeshSDFTask.mSDFVolumeList       = &outVolumeData.mSDFVolumeList;
    calculateMeshSDFTask.mBVHTree             = &bvhTree;
    calculateMeshSDFTask.mIsTwoSided          = false;

    for (int32_t zIndex = 0; zIndex < finalSDFVolumeDimension.z; ++zIndex)
    {
        calculateMeshSDFTask.mZIndex = zIndex;
        DoCalculateMeshSDFTask(&calculateMeshSDFTask, 0);
    }

    DeleteBVHTree(bvhTree.mRootNode);

    std::ofstream os(sdffile.c_str(), std::ifstream::binary);
    int32_t x = finalSDFVolumeDimension.x;
    int32_t y = finalSDFVolumeDimension.y;
    int32_t z = finalSDFVolumeDimension.z;
    os.write((char*)&x, sizeof(int32_t));
    os.write((char*)&y, sizeof(int32_t));
    os.write((char*)&z, sizeof(int32_t));
    os.write((char*) & outVolumeData.mSDFVolumeList[0], finalSDFVolumeDataCount * sizeof(float));
    float3 Min = (newSDFVolumeBound.Min);
    float3 Max = (newSDFVolumeBound.Max);
    os.write((char*)&Min, sizeof(float3));
    os.write((char*)&Max, sizeof(float3));
    os.close();

    float minVolumeDist = 1.0f;
    float maxVolumeDist = -1.0f;

    //we can probably move the calculation of the minimum & maximum distance of the SDF value
    //into the CalculateMeshSDFValue function
    for (uint32_t index = 0; index < outVolumeData.mSDFVolumeList.size(); ++index)
    {
        const float volumeSpaceDist = outVolumeData.mSDFVolumeList[index];
        minVolumeDist               = fmin(volumeSpaceDist, minVolumeDist);
        maxVolumeDist               = fmax(volumeSpaceDist, maxVolumeDist);
    }

    //TODO, not every mesh is going to be closed
    //do the check sometime in the future
    outVolumeData.mIsTwoSided             = false;
    outVolumeData.mSDFVolumeSize          = finalSDFVolumeDimension;
    outVolumeData.mLocalBoundingBox       = newSDFVolumeBound;
    outVolumeData.mDistMinMax             = float2(minVolumeDist, maxVolumeDist);
    //outVolumeData.mTwoSidedWorldSpaceBias = pMeshGroupInfo->twoSidedWorldSpaceBias;
}

} // namespace Diligent