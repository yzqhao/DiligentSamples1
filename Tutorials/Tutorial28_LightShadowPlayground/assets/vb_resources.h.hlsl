
#ifndef vb_resources_h
#define vb_resources_h

Texture2D diffuseMaps[256] : register(t1);
ByteAddressBuffer indirectMaterialBuffer : register(t0, space1);
//RES(Tex2D(float4), diffuseMaps[256], UPDATE_FREQ_NONE, t1, binding = 2);
//RES(SamplerState, nearClampSampler, UPDATE_FREQ_NONE, s0, binding = 3);
//RES(ByteBuffer, indirectMaterialBuffer, UPDATE_FREQ_PER_FRAME, t0, binding = 1);

cbuffer objectUniformBlock : register(b0, space3)
{
    float4x4 WorldViewProjMat;
	float4x4 WorldMat;
};

#if defined(INDIRECT_ROOT_CONSTANT)
    cbuffer indirectRootConstant : register(b2)
    {
        uint indirectDrawId;
    };
    //#define getDrawID() Get(indirectDrawId)
#else
    //#define getDrawID() In.drawId
#endif

#endif /* vb_resources_h */
