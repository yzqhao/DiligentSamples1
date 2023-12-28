

#include "Shader_Defs.h.hlsl"
#include "vb_resources.h.hlsl"

struct VsIn
{
    float3 Position: POSITION;
};

struct PsIn
{
	float4 Position : SV_Position;
#if !defined(INDIRECT_ROOT_CONSTANT)
	uint drawId : TEXCOORD0;
#endif
};

PsIn main(VsIn In, uint instanceId : SV_INSTANCEID)
{
	PsIn Out;
	Out.Position = mul(WorldViewProjMat, float4(In.Position.xyz, 1.0f));
	return (Out);
}

