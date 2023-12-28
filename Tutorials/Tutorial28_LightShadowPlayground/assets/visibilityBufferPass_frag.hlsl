
#include "Shader_Defs.h.hlsl"
#include "Packing.h.hlsl"
#include "vb_resources.h.hlsl"

struct PsIn
{
	float4 Position : SV_Position;
#if !defined(INDIRECT_ROOT_CONSTANT)
	uint drawId : TEXCOORD0;
#endif
};

uint calculateOutputVBID(bool opaque, uint drawID, uint primitiveID)
{
    uint drawID_primID = ((drawID << 23) & 0x7F800000) | (primitiveID & 0x007FFFFF);
    return (opaque) ? drawID_primID : (1 << 31) | drawID_primID;
}

float4 main( PsIn In, uint primitiveID : SV_PRIMITIVEID) : SV_TARGET
{
	float4 Out = unpackUnorm4x8(calculateOutputVBID(true, indirectDrawId, primitiveID));
	return Out;
}
