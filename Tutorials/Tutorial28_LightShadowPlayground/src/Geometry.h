#pragma once

#include "GLTFLoader.hpp"
#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

#define CLUSTER_SIZE 256

template <class T> bool Equals(T lhs, T rhs) { return lhs + std::numeric_limits<T>::epsilon() >= rhs && lhs - std::numeric_limits<T>::epsilon() <= rhs; }

namespace Diligent
{
static const float3 MinPerElem(const float3& vec0, const float3& vec1)
{
    return float3(((float)vec0.x < vec1.x) ? vec0.x : vec1.x,
                  ((float)vec0.y < vec1.y) ? vec0.y : vec1.y,
                  ((float)vec0.z < vec1.z) ? vec0.z : vec1.z);
}

static const float3 MaxPerElem(const float3& vec0, const float3& vec1)
{
    return float3(((float)vec0.x > vec1.x) ? vec0.x : vec1.x,
                  ((float)vec0.y > vec1.y) ? vec0.y : vec1.y,
                  ((float)vec0.z > vec1.z) ? vec0.z : vec1.z);
}

typedef struct ClusterCompact
{
    uint32_t triangleCount;
    uint32_t clusterStart;
} ClusterCompact;

typedef struct Cluster
{
    float3 aabbMin, aabbMax;
    float3 coneCenter, coneAxis;
    float  coneAngleCosine;
    float  distanceFromCamera;
    bool   valid;
} Cluster;

typedef struct ClusterContainer
{
    uint32_t        clusterCount;
    ClusterCompact* clusterCompacts;
    Cluster*        clusters;
} ClusterContainer;

struct FilterBatchData
{
    uint meshIndex;         // Index into meshConstants
    uint indexOffset;       // Index relative to meshConstants[meshIndex].indexOffset
    uint faceCount;         // Number of faces in this small batch
    uint outputIndexOffset; // Offset into the output index buffer
    uint drawBatchStart;    // First slot for the current draw call
    uint accumDrawIndex;
    uint _pad0;
    uint _pad1;
};

struct FilterBatchChunk
{
    uint32_t currentBatchCount;
    uint32_t currentDrawCallCount;
};

/************************************************************************/
// Meshes
/************************************************************************/
struct SDFVolumeData;
struct MeshInfo;

typedef uint32_t                    MaterialFlags;
typedef std::vector<SDFVolumeData*> BakedSDFVolumeInstances;
typedef bool (*GenerateVolumeDataFromFileFunc)(SDFVolumeData**, MeshInfo*);

enum MaterialFlagBits
{
    MATERIAL_FLAG_NONE              = 0,
    MATERIAL_FLAG_TWO_SIDED         = (1 << 0),
    MATERIAL_FLAG_ALPHA_TESTED      = (1 << 1),
    MATERIAL_FLAG_DOUBLE_VOXEL_SIZE = (1 << 2),
    MATERIAL_FLAG_ALL               = MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE
};

struct MeshInfo
{
    const char*   name                   = NULL;
    const char*   password               = NULL;
    MaterialFlags materialFlags          = MATERIAL_FLAG_NONE;
    float         twoSidedWorldSpaceBias = 0.0f;
    bool          sdfGenerated           = false;

    MeshInfo() {}
    MeshInfo(const char* name, const char* password, MaterialFlags materialFlags, float twoSidedWorldSpaceBias) :
        name(name),
        password(password),
        materialFlags(materialFlags),
        twoSidedWorldSpaceBias(twoSidedWorldSpaceBias),
        sdfGenerated(false)
    {}
};

struct IndirectDrawIndexArguments
{
    uint32_t mIndexCount;
    uint32_t mInstanceCount;
    uint32_t mStartIndex;
    uint32_t mVertexOffset;
    uint32_t mStartInstance;
};

enum ShaderAttribute
{
    PS_ATTRIBUTE_POSITION = 0,
    PS_ATTRIBUTE_COORDNATE0,
    PS_ATTRIBUTE_NORMAL,
    PS_ATTRIBUTE_TANGENT,
    PS_ATTRIBUTE_MAX,
};

struct ShadowData
{
    void* pIndices;
    void* pAttributes[PS_ATTRIBUTE_MAX];
};

typedef struct Scene
{
    //GltfMeshStreamData* geom;
    ShadowData     pShadow;
    GLTF::Model*   m_Model;
    uint32_t       mDrawArgCount;
    MaterialFlags* materialFlags;
    char**         textures;
    char**         normalMaps;
    char**         specularMaps;
} Scene;

struct SDFMesh
{
    //GltfMeshStreamData* pGeometry = NULL;
    ShadowData   pShadow;
    GLTF::Model* m_Model;
    MeshInfo*    pSubMeshesInfo        = NULL;
    uint32_t*    pSubMeshesGroupsSizes = NULL;
    uint32_t*    pSubMeshesIndices     = NULL;
    uint32_t     numSubMeshesGroups    = 0;
    uint32_t     numGeneratedSDFMeshes = 0;
};

void   adjustAABB(BoundBox* ownerAABB, const float3& point);
void   adjustAABB(BoundBox* ownerAABB, const BoundBox& otherAABB);
void   alignAABB(BoundBox* ownerAABB, float alignment);
float3 calculateAABBSize(const BoundBox* ownerAABB);
float3 calculateAABBExtent(const BoundBox* ownerAABB);
float3 calculateAABBCenter(const BoundBox* ownerAABB);

void createClusters(bool twoSided, const Scene* scene, IndirectDrawIndexArguments* draw, ClusterContainer* subMesh);
void destroyClusters(ClusterContainer* pMesh);
void addClusterToBatchChunk(const ClusterCompact* cluster, uint batchStart, uint accumDrawCount, uint accumNumTriangles, int meshIndex, FilterBatchChunk* batchChunk, FilterBatchData* batches);

Scene* loadScene(const char* fileName, float scale, float offsetX, float offsetY, float offsetZ, IRenderDevice* pDevice, IDeviceContext* pImmediateContext);
void   removeScene(Scene* scene);

void loadBakedSDFData(SDFMesh* outMesh, uint32_t startIdx, bool generateSDFVolumeData, BakedSDFVolumeInstances& sdfVolumeInstances, GenerateVolumeDataFromFileFunc generateVolumeDataFromFileFunc);

} // namespace Diligent
