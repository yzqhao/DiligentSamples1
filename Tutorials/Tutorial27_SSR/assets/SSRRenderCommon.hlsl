#ifndef _SSR_RENDER_COMMON_
#define _SSR_RENDER_COMMON_

#include "Common.hlsl"

TextureCube gCubeMap : register(t0);     // 0-skymap 
TextureCube girradianceMap : register(t1);     // 1-irradianceMap
TextureCube gSpecularMap : register(t2);     // 2-specularMap

SamplerState gCubeMap_sampler : register(s0);
SamplerState girradianceMap_sampler : register(s1);
SamplerState gSpecularMap_sampler : register(s2);

static const int g_AlbedoTextureID = 84;
static const int g_NormalTextureID = 85;
static const int g_RoughnessTextureID = 86;
static const int g_MationTextureID = 87;
static const int g_DepthTextureID = 88;
static const int g_BRDFTextureID = 89;

struct GBufferPixelOut
{
	float4 albedo  : SV_Target0;
    float4 normal  : SV_Target1;
    float4 specular: SV_Target2;
	float2 motion  : SV_Target3;
};

#endif