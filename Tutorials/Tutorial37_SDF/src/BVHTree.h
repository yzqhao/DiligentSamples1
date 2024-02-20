
#pragma once

#include "AdvancedMath.hpp"
#include "BasicMath.hpp"
#include "SDFConstant.h"
#include "GLTFLoader.hpp"

#include <queue>
#include <algorithm>

#define MESH_SCALE 1

#define SAN_MIGUEL_OFFSETX 0.0f

namespace Diligent
{
#define Epilson 0.0000001

inline float randomFloat01()
{
    return rand() / (float)RAND_MAX;
}

inline float maxElem(const float2& vec)
{
    return (vec.x > vec.y) ? vec.x : vec.y;
}

inline float maxElem(const float3& vec)
{
    return std::max(vec.x, std::max(vec.y, vec.z));
}

void adjustAABB(BoundBox* ownerAABB, const float3& point);
void adjustAABB(BoundBox* ownerAABB, const BoundBox& otherAABB);
void alignAABB(BoundBox* ownerAABB, float alignment);
float3 calculateAABBSize(const BoundBox* ownerAABB);
float3 calculateAABBExtent(const BoundBox* ownerAABB);
float3 calculateAABBCenter(const BoundBox* ownerAABB);

struct SDFMesh
{
    GLTF::Model* pGeometry             = NULL;
    bool         sdfGenerated          = false;
    uint32_t* pSubMeshesGroupsSizes = NULL;
    uint32_t* pSubMeshesIndices     = NULL;
    uint32_t  numSubMeshesGroups    = 0;
    uint32_t  numGeneratedSDFMeshes = 0;
};

struct Triangle
{
    Triangle() = default;
    Triangle(const float3& v0, const float3& v1, const float3& v2, const float3& n0, const float3& n1, const float3& n2) :
        mV0(v0),
        mV1(v1),
        mV2(v2),
        mN0(n0),
        mN1(n1),
        mN2(n2)
    {
        mE1 = mV1 - mV0;
        mE2 = mV2 - mV0;
    }


    void Init(const float3& v0, const float3& v1, const float3& v2, const float3& n0, const float3& n1, const float3& n2)
    {
        mV0 = v0;
        mV1 = v1;
        mV2 = v2;
        mN0 = n0;
        mN1 = n1;
        mN2 = n2;

        mE1 = mV1 - mV0;
        mE2 = mV2 - mV0;
    }

    //triangle vertices
    float3 mV0;
    float3 mV1;
    float3 mV2;

    //triangle edges
    float3 mE1;
    float3 mE2;

    //vertices normal
    float3 mN0;
    float3 mN1;
    float3 mN2;
};

struct Intersection
{
    Intersection(const float3& hitted_pos,
                 const float3& hitted_normal,
                 float       t_intersection) :
        mHittedPos(hitted_pos),
        mHittedNormal(hitted_normal),
        mIntersectedTriangle(NULL),
        mIntersection_TVal(t_intersection),
        mIsIntersected(false)
    {
    }
    Intersection() :
        mHittedPos(),
        mHittedNormal(),
        mIntersectedTriangle(NULL),
        mIntersection_TVal(FLT_MAX),
        mIsIntersected(false)
    {}


    float3            mHittedPos;
    float3            mHittedNormal;
    const Triangle* mIntersectedTriangle;
    float           mIntersection_TVal;
    bool            mIsIntersected;
};

void RayIntersectTriangle(const Ray& ray, const Triangle& triangle, Intersection& outIntersection);

struct BVHAABBox
{
    BoundBox mAABB;
    float3     Center;
    Triangle mTriangle;

    int32_t InstanceID;
    float   SurfaceAreaLeft;
    float   SurfaceAreaRight;

    BVHAABBox()
    {
        mAABB.Min  = float3(FLT_MAX);
        mAABB.Max  = float3(-FLT_MAX);
        InstanceID       = 0;
        SurfaceAreaLeft  = 0.0f;
        SurfaceAreaRight = 0.0f;
    }

    void Expand(float3& point)
    {
        mAABB.Min = float3(
            fmin(mAABB.Min.x, point.x),
            fmin(mAABB.Min.y, point.y),
            fmin(mAABB.Min.z, point.z));

        mAABB.Max = float3(
            fmax(mAABB.Max.x, point.x),
            fmax(mAABB.Max.y, point.y),
            fmax(mAABB.Max.z, point.z));

        Center = 0.5f * (mAABB.Max + mAABB.Min);
    }

    void Expand(BVHAABBox& aabox)
    {
        Expand(aabox.mAABB.Min);
        Expand(aabox.mAABB.Max);
    }
};



struct BVHNode
{
    float     SplitCost;
    BVHAABBox BoundingBox;
    BVHNode*  Left;
    BVHNode*  Right;
};


struct BVHTree
{
    BVHTree() :
        mRootNode(NULL), mBVHNodeCount(0), mTransitionNodeCount(0)
    {
        mBBOXDataList.reserve(1000000);
    }

    std::vector<BVHAABBox> mBBOXDataList;
    BVHNode*                 mRootNode;

    uint32_t mBVHNodeCount;
    uint32_t mTransitionNodeCount;
};


bool RayIntersectsBox(const float3& origin, const float3& rayDirInv, const float3& BboxMin, const float3& BboxMax);

void BVHTreeIntersectRayAux(BVHNode* rootNode, BVHNode* node, const Ray& ray, Intersection& outIntersection);

void AddMeshInstanceToBBOX16(BVHTree* pBvhTree, SDFMesh* pMesh, uint16_t* pMeshIndices, uint32_t groupNum, uint32_t meshGroupSize, uint32_t idxFirstMeshInGroup, BoundBox* pAABBFirstMeshInGroup, const float4x4& meshWorldMat);

void AddMeshInstanceToBBOX32(BVHTree* pBvhTree, SDFMesh* pMesh, uint32_t* pMeshIndices, uint32_t groupNum, uint32_t meshGroupSize, uint32_t idxFirstMeshInGroup, BoundBox* pAABBFirstMeshInGroup, const float4x4& meshWorldMat);

void SortAlongAxis(BVHTree* bvhTree, int32_t begin, int32_t end, int32_t axis);
float CalculateSurfaceArea(const BVHAABBox& bbox);
void FindBestSplit(BVHTree* bvhTree, int32_t begin, int32_t end, int32_t& split, int32_t& axis, float& splitCost);
void CalculateBounds(BVHTree* bvhTree, int32_t begin, int32_t end, float3& outMinBounds, float3& outMaxBounds);
BVHNode* CreateBVHNodeSHA(BVHTree* bvhTree, int32_t begin, int32_t end, float parentSplitCost);
void DeleteBVHTree(BVHNode* node);


struct SDFTextureLayoutNode
{
    SDFTextureLayoutNode(const int3& nodeCoord, const int3& nodeSize) :
        mNodeCoord(nodeCoord),
        mNodeSize(nodeSize),
        mUsed(false)
    {
    }
    //node coord not in texel space but in raw volume dimension space
    int3 mNodeCoord;
    int3 mNodeSize;
    bool  mUsed;
};

struct SDFVolumeTextureAtlasLayout
{
    SDFVolumeTextureAtlasLayout(const int3& atlasLayoutSize) :
        mAtlasLayoutSize(atlasLayoutSize)
    {
        mAllocationCoord       = int3(-SDF_MAX_VOXEL_ONE_DIMENSION_X, 0, SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Z * 3);
        mDoubleAllocationCoord = int3(-SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_X, 0, 0);
    }


    bool AddNewNode(const int3& volumeDimension, int3& outCoord)
    {
        if (volumeDimension.x <= SDF_MAX_VOXEL_ONE_DIMENSION_X &&
            volumeDimension.y <= SDF_MAX_VOXEL_ONE_DIMENSION_Y &&
            volumeDimension.z <= SDF_MAX_VOXEL_ONE_DIMENSION_Z)
        {
            return AddNormalNode(volumeDimension, outCoord);
        }
        return AddDoubleNode(volumeDimension, outCoord);
    }

    bool AddNormalNode(const int3& volumeDimension, int3& outCoord)
    {
        if ((mAllocationCoord.x + (SDF_MAX_VOXEL_ONE_DIMENSION_X * 2)) <= mAtlasLayoutSize.x)
        {
            mAllocationCoord.x = (mAllocationCoord.x + SDF_MAX_VOXEL_ONE_DIMENSION_X);
            mNodes.push_back(SDFTextureLayoutNode(
                mAllocationCoord, volumeDimension));
        }
        else if ((mAllocationCoord.y + (SDF_MAX_VOXEL_ONE_DIMENSION_Y * 2)) <= mAtlasLayoutSize.y)
        {
            mAllocationCoord.x = (0);
            mAllocationCoord.y = (mAllocationCoord.y + SDF_MAX_VOXEL_ONE_DIMENSION_Y);

            mNodes.push_back(SDFTextureLayoutNode(
                mAllocationCoord, volumeDimension));
        }
        else if ((mAllocationCoord.z + (SDF_MAX_VOXEL_ONE_DIMENSION_Z * 2)) <= mAtlasLayoutSize.z)
        {
            mAllocationCoord.x = (0);
            mAllocationCoord.y = (0);
            mAllocationCoord.z = (mAllocationCoord.z + SDF_MAX_VOXEL_ONE_DIMENSION_Z);

            mNodes.push_back(SDFTextureLayoutNode(
                mAllocationCoord, volumeDimension));
        }
        else
        {
            return false;
        }
        outCoord = mNodes.back().mNodeCoord;
        return true;
    }
    bool AddDoubleNode(const int3& volumeDimension, int3& outCoord)
    {
        if ((mDoubleAllocationCoord.x + (SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_X * 2)) <= mAtlasLayoutSize.x)
        {
            mDoubleAllocationCoord.x = (mDoubleAllocationCoord.x + SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_X);
            mNodes.push_back(SDFTextureLayoutNode(
                mDoubleAllocationCoord, volumeDimension));
        }
        else if ((mDoubleAllocationCoord.y + (SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Y * 2)) <= mAtlasLayoutSize.y)
        {
            mDoubleAllocationCoord.x = (0);
            mDoubleAllocationCoord.y = (mDoubleAllocationCoord.y + SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Y);

            mNodes.push_back(SDFTextureLayoutNode(
                mDoubleAllocationCoord, volumeDimension));
        }
        else if ((mDoubleAllocationCoord.z + (SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Z * 2)) <= mAtlasLayoutSize.z)
        {
            mDoubleAllocationCoord.x = (0);
            mDoubleAllocationCoord.y = (0);
            mDoubleAllocationCoord.z = (mDoubleAllocationCoord.z + SDF_DOUBLE_MAX_VOXEL_ONE_DIMENSION_Z);

            mNodes.push_back(SDFTextureLayoutNode(
                mDoubleAllocationCoord, volumeDimension));
        }

        else
        {
            return false;
        }
        outCoord = mNodes.back().mNodeCoord;
        return true;
    }

    int3                               mAtlasLayoutSize;
    std::vector<SDFTextureLayoutNode> mNodes;

    int3 mAllocationCoord;
    int3 mDoubleAllocationCoord;
};

struct SDFVolumeData;
struct SDFVolumeTextureNode
{
    SDFVolumeTextureNode(SDFVolumeData* sdfVolumeData /*, SDFMesh* mainMesh, SDFMeshInstance* meshInstance*/) :
        mAtlasAllocationCoord(-1, -1, -1),
        mSDFVolumeData(sdfVolumeData),
        /*mMainMesh(mainMesh),
		mMeshInstance(meshInstance),*/
        mHasBeenAdded(false)
    {
    }


    int3          mAtlasAllocationCoord;
    SDFVolumeData* mSDFVolumeData;
    /*SDFMesh* mMainMesh;
	SDFMeshInstance* mMeshInstance;*/

    //the coordinate of this node inside the volume texture atlases
    //not in texel space
    bool mHasBeenAdded;
};

struct SDFVolumeData
{
    typedef std::vector<float3>     SampleDirectionsList;
    typedef std::vector<float>    SDFVolumeList;
    typedef std::vector<Triangle> TriangeList;

    SDFVolumeData(SDFMesh* mainMesh) :
        mSDFVolumeSize(0),
        mLocalBoundingBox(),
        mDistMinMax(FLT_MAX, FLT_MIN),
        mIsTwoSided(false),
        mTwoSidedWorldSpaceBias(0.f),
        mSDFVolumeTextureNode(this)
    {
    }
    SDFVolumeData() :
        mSDFVolumeSize(0),
        mLocalBoundingBox(),
        mDistMinMax(FLT_MAX, FLT_MIN),
        mIsTwoSided(false),
        mTwoSidedWorldSpaceBias(0.f),
        mSDFVolumeTextureNode(this)
    {
    }

    //
    SDFVolumeList mSDFVolumeList;
    //
    //Size of the distance volume
    int3 mSDFVolumeSize;
    //
    //Local Space of the Bounding Box volume
    BoundBox mLocalBoundingBox;

    //stores the min & the maximum distances found in the volume
    //in the space of the world voxel volume
    //x stores the minimum while y stores the maximum
    float2 mDistMinMax;
    //
    bool mIsTwoSided;
    //
    float mTwoSidedWorldSpaceBias;

    SDFVolumeTextureNode mSDFVolumeTextureNode;
};

struct SDFVolumeTextureAtlas
{
    SDFVolumeTextureAtlas(const int3& atlasSize) :
        mSDFVolumeAtlasLayout(atlasSize)
    {
    }

    void AddVolumeTextureNode(SDFVolumeTextureNode* volumeTextureNode)
    {
        //int3 atlasCoord = volumeTextureNode->mAtlasAllocationCoord;

        if (volumeTextureNode->mHasBeenAdded)
        {
            return;
        }

        mSDFVolumeAtlasLayout.AddNewNode(volumeTextureNode->mSDFVolumeData->mSDFVolumeSize,
                                         volumeTextureNode->mAtlasAllocationCoord);
        mPendingNodeQueue.push(volumeTextureNode);
        volumeTextureNode->mHasBeenAdded = true;
    }

    SDFVolumeTextureNode* ProcessQueuedNode()
    {
        if (mPendingNodeQueue.empty())
        {
            return NULL;
        }

        SDFVolumeTextureNode* node = mPendingNodeQueue.front();
        mCurrentNodeList.push_back(node);
        mPendingNodeQueue.pop();
        return node;
    }

    SDFVolumeTextureAtlasLayout mSDFVolumeAtlasLayout;

    std::queue<SDFVolumeTextureNode*>  mPendingNodeQueue;
    std::vector<SDFVolumeTextureNode*> mCurrentNodeList;
};

void GenerateSampleDirections(int32_t thetaSteps, int32_t phiSteps, SDFVolumeData::SampleDirectionsList& outDirectionsList, int32_t finalThetaModifier = 1);


struct CalculateMeshSDFTask
{
    const SDFVolumeData::SampleDirectionsList* mDirectionsList;
    const SDFVolumeData::TriangeList*          mMeshTrianglesList;
    const BoundBox*                                mSDFVolumeBounds;
    const int3*                               mSDFVolumeDimension;
    int32_t                                    mZIndex;
    float                                      mSDFVolumeMaxDist;
    BVHTree*                                   mBVHTree;
    SDFVolumeData::SDFVolumeList*              mSDFVolumeList;
    bool                                       mIsTwoSided;
};


void DoCalculateMeshSDFTask(void* dataPtr, uintptr_t index);

bool GenerateVolumeDataFromFile(SDFVolumeData** ppOutVolumeData, const std::string& sdffile);
void GenerateVolumeDataFromMesh(SDFVolumeData** ppOutVolumeData, SDFMesh* pMainMesh, const std::string& sdffile, uint32_t groupNum, uint32_t meshGroupSize, uint32_t idxFirstMeshInGroup, float sdfResolutionScale);

} // namespace Diligent
