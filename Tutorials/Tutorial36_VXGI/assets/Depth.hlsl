
#include "CameraCB.hlsl"
#include "LightingUtil.hlsl"

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 worldMat;
};

struct VertexPosUvNormal
{
	float3 PosL    : ATTRIB0;
	float2 TexC    : ATTRIB1;
    float3 NormalL : ATTRIB2;
};

struct VertexOut 
{ 
    float4 Pos     : SV_POSITION; 
};

void VS(in VertexPosUvNormal vin, out VertexOut PSIn) 
{
    float4 posW = mul(worldMat, float4(vin.PosL, 1.0f));
    PSIn.Pos = mul(gLightViewProj, posW);
    PSIn.Pos.z = PSIn.Pos.z / 2.0 + 0.5;
}

