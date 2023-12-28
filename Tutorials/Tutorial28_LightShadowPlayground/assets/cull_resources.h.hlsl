
#ifndef CULL_RESOURCES_HLSL
#define CULL_RESOURCES_HLSL

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
RWByteAddressBuffer                             indirectMaterialBuffer                                  : register(u0, space1);

/*
RWStructuredBuffer<UncompactedDrawArguments>    uncompactedDrawArgsRW0            : register(u3, space1);
RWStructuredBuffer<UncompactedDrawArguments>    uncompactedDrawArgsRW1            : register(u4, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferAlpha0      : register(u5, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferAlpha1      : register(u6, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferNoAlpha0    : register(u7, space1);
RWStructuredBuffer<uint>                        indirectDrawArgsBufferNoAlpha1    : register(u8, space1);
RWByteAddressBuffer                             filteredIndicesBuffer0            : register(u1, space1);
RWByteAddressBuffer                             filteredIndicesBuffer1            : register(u1, space1);
StructuredBuffer<UncompactedDrawArguments>      uncompactedDrawArgs[NUM_CULLING_VIEWPORTS]              : register(t0, space1);
RWByteAddressBuffer                             indirectMaterialBuffer                                  : register(u0, space1);
*/

#if defined(INDIRECT_COMMAND_BUFFER)
//RES(COMMAND_BUFFER, icbAlpha[NUM_CULLING_VIEWPORTS], UPDATE_FREQ_PER_FRAME, u9, binding=14);
//RES(COMMAND_BUFFER, icbNoAlpha[NUM_CULLING_VIEWPORTS], UPDATE_FREQ_PER_FRAME, u11, binding=14 + NUM_CULLING_VIEWPORTS);
#endif

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