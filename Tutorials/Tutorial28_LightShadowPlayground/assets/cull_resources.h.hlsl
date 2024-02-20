
#ifndef CULL_RESOURCES_HLSL
#define CULL_RESOURCES_HLSL

#include "Shader_Defs.h.hlsl"

// UPDATE_FREQ_NONE
StructuredBuffer<uint> materialProps : register(t11, space0);
ByteAddressBuffer vertexDataBuffer : register(t12, space0);
ByteAddressBuffer indexDataBuffer : register(t13, space0);
StructuredBuffer<MeshConstants> meshConstantsBuffer : register(t14, space0);

// UPDATE_FREQ_PER_FRAME
RWStructuredBuffer<UncompactedDrawArguments>    uncompactedDrawArgsRW[NUM_CULLING_VIEWPORTS]            : register(u3, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferAlpha[NUM_CULLING_VIEWPORTS]      : register(u5, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferNoAlpha[NUM_CULLING_VIEWPORTS]    : register(u7, space1);
RWByteAddressBuffer                             filteredIndicesBuffer[NUM_CULLING_VIEWPORTS]            : register(u1, space1);
StructuredBuffer<UncompactedDrawArguments>      uncompactedDrawArgs[NUM_CULLING_VIEWPORTS]              : register(t0, space1);
RWBuffer<uint>                                  indirectMaterialBuffer                                  : register(u0, space1);

cbuffer visibilityBufferConstants : register(b0)   // b13, space1
{
    float4x4 mWorldViewProjMat[NUM_CULLING_VIEWPORTS];
    CullingViewPort mCullingViewports[NUM_CULLING_VIEWPORTS];
};

cbuffer batchData_rootcbv : register(b1)   // b14, space0
{
	SmallBatchData smallBatchDataBuffer[BATCH_COUNT];
};

#endif