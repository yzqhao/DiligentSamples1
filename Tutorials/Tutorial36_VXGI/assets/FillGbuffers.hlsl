#include "CameraCB.hlsl"

Texture2D textureMaps[81] : register(t3);   // 84 material tex
SamplerState textureMaps_sampler : register(s3);

struct VertexPosUvNormal
{
	float3 PosL    : ATTRIB0;
	float2 TexC    : ATTRIB1;
    float3 NormalL : ATTRIB2;
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
    float3 PosW    : TEXCOORD2;
    float4 CurPos  : TEXCOORD3;
    float4 PrePos  : TEXCOORD4;
    float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
};

struct GBufferPixelOut
{
	float4 albedo  : SV_Target0;
    float4 normal  : SV_Target1;
    float4 specular: SV_Target2;
	float2 motion  : SV_Target3;
};

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 worldMat;
	float4x4 prevWorldMat;
	uint gMaterialIndex;
	float roughness;
	float metalness;
	int pbrMaterials;
};


VertexOut VS(VertexPosUvNormal vin)
{
	VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posW = mul(worldMat, float4(vin.PosL, 1.0f));
    vout.PosW = posW.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = normalize(mul(worldMat, float4(vin.NormalL, 0.0f)).xyz);

    // Transform to homogeneous clip space.
    vout.PosH = mul(gViewProj, posW);

    vout.CurPos = vout.PosH;
    vout.PrePos = mul(gPreViewProj, mul(prevWorldMat, float4(vin.PosL, 1.0f)));
	

    vout.TexC = vin.TexC.xy;
	
    return vout;
}


float3 reconstructNormal(float4 sampleNormal)
{
	float3 tangentNormal;
	tangentNormal.xy = sampleNormal.rg * 2 - 1;
	tangentNormal.z = sqrt(1 - saturate(dot(tangentNormal.xy, tangentNormal.xy)));
	return tangentNormal;
}


float3 getNormalFromMap(Texture2D normalTex, float3 viewDirection, float3 normal, float2 uv)
{
	float4 rawNormal = normalTex.Sample(textureMaps_sampler, uv);
	float3 tangentNormal = reconstructNormal(rawNormal);


	float3 dPdx = ddx(viewDirection);
	float3 dPdy = ddy(viewDirection);

	float2 dUVdx = ddx(uv);
	float2 dUVdy = ddy(uv);

	float3 N = normalize(normal);

	float3 crossPdyN = cross(dPdy, N);

	float3 crossNPdx = cross(N, dPdx);

	float3 T = crossPdyN * dUVdx.x + crossNPdx * dUVdy.x;

	float3 B = crossPdyN * dUVdx.y + crossNPdx * dUVdy.y;

	float invScale = rsqrt(max(dot(T, T), dot(B, B)));

	float3x3 TBN = float3x3(T * invScale, B * invScale, N);

	return mul(tangentNormal, TBN);
}

GBufferPixelOut PS(VertexOut pin)
{
    GBufferPixelOut Out = (GBufferPixelOut)0;

	//MaterialData matData = gMaterialData[gMaterialIndex];
	//uint textureMapIds = matData.textureMapIds;
	uint textureMapIds = gMaterialIndex;

	const uint albedoMapId = (((textureMapIds) >> 0) & 0xFF);
	const uint normalMapId = (((textureMapIds) >> 8) & 0xFF);
	const uint metallicMapId = (((textureMapIds) >> 16) & 0xFF);
	const uint roughnessMapId = (((textureMapIds) >> 24) & 0xFF);

	const uint aoMapId = 5;

	float4 albedoAndAlpha = textureMaps[albedoMapId].Sample(textureMaps_sampler, pin.TexC);

	const float alpha = albedoAndAlpha.a;
	if(alpha < 0.5)
		clip(-1);

	float3 albedo = float3(0.5f, 0.0f, 0.0f);
	float _roughness = (roughness);
	float _metalness = (metalness);
	float ao = 1.0f;

	float3 N = normalize(pin.NormalW);
	float3 V = normalize(gEyePosW.xyz - pin.PosW);


	if((pbrMaterials) != -1)
	{
		albedo = albedoAndAlpha.rgb;
		N = getNormalFromMap(textureMaps[normalMapId], -V, pin.NormalW, pin.TexC);
		_metalness = textureMaps[metallicMapId].Sample(textureMaps_sampler, pin.TexC).r;
		_roughness = textureMaps[roughnessMapId].Sample(textureMaps_sampler, pin.TexC).r;
		ao = textureMaps[aoMapId].SampleLevel(textureMaps_sampler, pin.TexC, 0).r;
	}

	if(roughnessMapId == 6) {
		_roughness = 0.05f;
	}

	Out.albedo = float4(albedo, alpha);
	Out.normal = float4(N, _metalness);
	Out.specular = float4(_roughness, ao, pin.TexC);

	Out.motion = pin.CurPos.xy / pin.CurPos.w - pin.PrePos.xy / pin.PrePos.w;

    return Out;
}
