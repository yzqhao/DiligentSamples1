#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

#include <vector>


namespace Diligent
{


#define MAX_TEXTURE_UNITS 256U

// This defines the amount of triangles that will be processed in parallel by the
// compute shader in the triangle filtering process.
// Should be a multiple of the wavefront size
#define CLUSTER_SIZE 256

// BATCH_COUNT limits the amount of triangle batches we can process on the GPU.
// It depends on the amoutnt of data we need to store in the constant buffers, since
// the max constant buffer size is 64KB - sizeof(SmallBatchData) * 2048 = 64KB
#define BATCH_COUNT 2048



// This value defines the amount of threads per group that will be used to clear the
// indirect draw buffers.
#define CLEAR_THREAD_COUNT 256

// The following value defines the maximum amount of indirect draw calls that will be
// drawn at once. This value depends on the number of submeshes or individual objects
// in the scene. Changing a scene will require to change this value accordingly.
#define MAX_DRAWS_INDIRECT 256

// This defines the amount of viewports that are going to be culled in parallel.
#define NUM_CULLING_VIEWPORTS 1
#define VIEW_CAMERA           0

#define INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS 8

#if defined(INDIRECT_ROOT_CONSTANT)
#    define INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET 1
#else
#    define INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET 0
#endif

#define MATERIAL_BUFFER_SIZE (MAX_DRAWS_INDIRECT * 2 * NUM_CULLING_VIEWPORTS)

struct RootConstant
{
    uint drawId;
};

struct SmallBatchData
{
    uint mMeshIndex;         // Index into meshConstants
    uint mIndexOffset;       // Index relative to the meshConstants[meshIndex].indexOffset
    uint mFaceCount;         // Number of faces in this small batch
    uint mOutputIndexOffset; // Offset into the output index buffer
    uint mDrawBatchStart;    // First slot for the current draw call
    uint mAccumDrawIndex;
    uint _pad0;
    uint _pad1;
};

struct MeshConstants
{
    uint faceCount;
    uint indexOffset;
    uint materialID;
    uint twoSided; //0 or 1
    float4x4 worldMatrix;
};

struct UncompactedDrawArguments
{
    uint mNumIndices;
    uint mStartIndex;
    uint mMaterialID;
    uint pad_;
};

struct CullingViewPort
{
    float mWindowSizeX;
    float mWindowSizeY;
    uint  mSampleCount;
    uint  _pad0;
};

} // namespace Diligent