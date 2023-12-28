#include "Shadows.hlsl"
#include "PBRLighting.hlsl"

Texture2D BaseColorGbuffer;
Texture2D NormalGbuffer;
Texture2D WorldPosGbuffer;
Texture2D OrmGbuffer;

SamplerState BaseColorGbuffer_sampler;
SamplerState NormalGbuffer_sampler;
SamplerState WorldPosGbuffer_sampler;
SamplerState OrmGbuffer_sampler;

StructuredBuffer<TileLightInfo> LightInfoList;

#define DIRECTIONAL_LIGHT_PIXEL_WIDTH       5.0f
#define SPOT_LIGHT_PIXEL_WIDTH              10.0f

struct VertexIn
{
    float3 PosL    : ATTRIB0;
    float2 TexC    : ATTRIB1;
};

struct VertexOut
{
	float4 PosH  : SV_POSITION;
    float2 TexC    : TEXCOORD;
};


VertexOut VS(VertexIn vin)
{
    VertexOut vout = (VertexOut)0.0f;

	// Already in homogeneous clip space.
	vout.PosH = float4(vin.PosL, 1.0f);

    vout.TexC = vin.TexC;

    return vout;
}

#define USE_TBDR 1
float4 PS(VertexOut pin) : SV_TARGET
{
	float3 FinalColor = 0.0f;

	// Get Gbuffer data
	float3 BaseColor = BaseColorGbuffer.Sample(BaseColorGbuffer_sampler, pin.TexC).rgb;	
	float3 Normal = NormalGbuffer.Sample(NormalGbuffer_sampler, pin.TexC).rgb;			
	float3 WorldPos = WorldPosGbuffer.Sample(WorldPosGbuffer_sampler, pin.TexC).rgb;
	float ShadingModelValue = WorldPosGbuffer.Sample(WorldPosGbuffer_sampler, pin.TexC).a;
	uint ShadingModel = (uint)round(ShadingModelValue * (float)0xF);	
	float Roughness = OrmGbuffer.Sample(OrmGbuffer_sampler, pin.TexC).g;
	float Metallic = OrmGbuffer.Sample(OrmGbuffer_sampler, pin.TexC).b;	
	
	if(ShadingModel == 0) // DefaultLit
	{		
		float3 CameraPosition = gEyePosW;
		float3 ViewDir = normalize(CameraPosition - WorldPos);
		Normal = normalize(Normal);
		
#if USE_TBDR		
		// Get the tile index of current pixel
		float2 ScreenPos = UVToScreen(pin.TexC, gRenderTargetSize);
		uint TileX = floor(ScreenPos.x / TILE_BLOCK_SIZE);
		uint TileY = floor(ScreenPos.y / TILE_BLOCK_SIZE);
		uint TileCountX = ceil(gRenderTargetSize.x / TILE_BLOCK_SIZE);
		uint TileIndex = TileY * TileCountX + TileX;
		
		TileLightInfo LightInfo = LightInfoList[TileIndex];
		
		// Calculate direct lighting
		[unroll(10)]
		for (uint i = 0; i < LightInfo.LightCount; i++)
		{
			uint LightIndex = LightInfo.LightIndices[i];
			LightParameters Light = Lights[LightIndex];
#else
		[unroll(10)]
		for (uint i = 0; i < LightCount; i++)
		{
			LightParameters Light = Lights[i];			
#endif //USE_TBDR
			
			// PointLight
			if(Light.LightType == 3)
			{		
				float3 LightToPoint = WorldPos - Light.Position;
				float ShadowFactor = 1.0f;  //CalcVisibilityOmni(LightToPoint, Light.ShadowMapIdx, Light.Range);
							
				float3 LightDir = normalize(Light.Position - WorldPos);
				float Attenuation = CalcDistanceAttenuation(WorldPos, Light.Position, Light.Range);
				float3 Radiance = Light.Intensity * Attenuation * Light.Color;
				
				FinalColor += DirectLighting(Radiance, LightDir, Normal, ViewDir, Roughness, Metallic, BaseColor, ShadowFactor);
			}	
		}
	}
	else if(ShadingModel == 1) //Unlit
	{
		FinalColor = BaseColor;
	}	

    return float4(FinalColor, 1.0f);
}