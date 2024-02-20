#include "CameraCB.hlsl"
#include "LightingUtil.hlsl"

Texture2D textureMaps[81];
SamplerState textureMaps_sampler;

Texture2D fluxTexture;
SamplerState fluxTexture_sampler;
Texture2D positionTexture;
SamplerState positionTexture_sampler;
Texture2D normalTexture;
SamplerState normalTexture_sampler;

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
    float4 ShadowPos  : TEXCOORD5;
    float3 NormalW : NORMAL;
	float2 TexC    : TEXCOORD;
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
    vout.ShadowPos = mul(mul(gLightProj, gLightView), posW);
	
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

//计算间接光照
float3 calcIndirect(float4 ShadowPos, float3 FragPos, float3 normal)
{
	float3 projectCoord = ShadowPos.xyz / ShadowPos.w;
	projectCoord = projectCoord*0.5 + 0.5;
	if (projectCoord.x<0 || projectCoord.x>1 || projectCoord.y<0 || projectCoord.y>1)
		return float3(0,0,0);

	float filterSize = 2048;

	float3 indir_color = float3(0,0,0);
	float3 temp = float3(0,0,0);
	for(int i=0; i<SAMPLE_NUMBER; i++){
		float2 uv = projectCoord.xy+randomArray[i].xy*(gFilterRange/filterSize);
		//间接光源光强
		float3 irrandance = fluxTexture.Sample(fluxTexture_sampler, uv).rgb;
		float3 idir_p = positionTexture.Sample(positionTexture_sampler, uv).rgb;
		float3 idir_wi = normalize(idir_p - FragPos);
		float3 idir_normal = normalize(normalTexture.Sample(normalTexture_sampler, uv).rgb);
		float3 sample_color = (irrandance*max(dot(idir_wi,normal),0) * max(dot(idir_normal,FragPos-idir_p),0) / pow(length(idir_p-FragPos),2)) * randomArray[i].z;
		indir_color += sample_color;
		temp = irrandance;
	}
	indir_color = clamp(indir_color/SAMPLE_NUMBER, 0.0, 1.0);

	return indir_color * 20;
}

float4 PS(VertexOut pin) : SV_TARGET
{
    float4 Out;

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


	float2 ndc = float2(pin.TexC.x * 2.0 - 1.0, (1.0 - pin.TexC.y) * 2.0 - 1.0);
	float3 R = reflect(-V, N);
	float3 F0 = float3(0.04f, 0.04f, 0.04f);
	F0 = lerp(F0, albedo, _metalness);

	float3 Lo = float3(0.0, 0.0, 0.0);
	// 方向光
	for(int i = 0; i < gAmountOfDLights; ++i)
	{
		float3 L = -normalize(gDLights[i].mDir.xyz);
		float3 H = normalize(V + L);
		float3 radiance = gDLights[i].mCol.rgb * gDLights[i].mCol.a;
		float NDF = distributionGGX(N, H, _roughness);
		float G = GeometrySmith(N, V, L, _roughness);
		float3 F = fresnelSchlick(dot(N,H), F0);
		float3 nominator = NDF * G * F;
		float denominator = 4.0f * max(dot(N,V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		float3 specular = nominator / denominator;
		float3 kS = F;
		float3 kD = float3(1.0, 1.0, 1.0) - kS;
		kD *= 1.0f - _metalness;
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL>0.0f)
		{
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}
		else
		{
			Lo += float3(0.0f, 0.0f, 0.0f);
		}
	}

	// 点光
	for(int pl_i = 0; pl_i < gAmountOfPLights; ++pl_i)
	{
		float3 L = normalize(gPLights[pl_i].pos.xyz - pin.PosW);
		float3 H = normalize(V + L);
		float distance = length(gPLights[pl_i].pos.xyz - pin.PosW);
		float distanceByRadius = 1.0f - pow(distance / gPLights[pl_i].radius, 4);
		float clamped = pow(clamp(distanceByRadius, 0.0f, 1.0f), 2.0f);
		float attenuation = clamped / (distance * distance + 1.0f);
		float3 radiance = gPLights[pl_i].col.rgb * attenuation * gPLights[pl_i].intensity;
		float NDF = distributionGGX(N, H, _roughness);
		float G = GeometrySmith(N, V, L, _roughness);
		float3 F = fresnelSchlick(dot(N, H), F0);
		float3 nominator = NDF * G * F;
		float denominator = 4.0f * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.001;
		float3 specular = nominator / denominator;
		float3 kS = F;
		float3 kD = float3(1.0, 1.0, 1.0) - kS;
		kD *= 1.0f - _metalness;
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL>0.0f) {
			Lo += (kD * albedo / PI + specular) * radiance * NdotL;
		}else {
			Lo+= float3(0.0f, 0.0f, 0.0f);
		}
	}
	
	float3 color = Lo + (gRSM ? calcIndirect(pin.ShadowPos, pin.PosW, N) : float3(0,0,0));
    Out = float4(color.r, color.g, color.b, 1.0f);
    return Out;
}
