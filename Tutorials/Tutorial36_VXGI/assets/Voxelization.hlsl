
#include "CameraCB.hlsl"
#include "LightingUtil.hlsl"

Texture2D TexVoxelgrid;
SamplerState TexVoxelgrid_sampler;

// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 M;
	float4x4 N;
	float3 sceneVoxelScale;
};

cbuffer cbPass : register(b1)
{
    float4x4 shadowmapMvp;
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
	float2 TexC    : TEXCOORD;
	float3 voxelPos    : TEXCOORD1;
    float4 PosW    : TEXCOORD2;
    float4 ShadowPos    : TEXCOORD3;
    float3 NormalW : NORMAL;
};

void VS(in VertexPosUvNormal vin, out VertexOut PSIn) 
{
    float4 posW = mul(M, float4(vin.PosL, 1.0f));
    PSIn.PosW  = posW;
    PSIn.NormalW = normalize(mul(N, float4(vin.NormalL, 0.0f)).xyz);
    PSIn.TexC = vin.TexC.xy;

    // we want to voxelize everything so the whole world is scaled to be inside clip space (-1.0...1.0)
	PSIn.Pos = vec4(posW.xyz * sceneVoxelScale, 1.0f); 
}


struct GSOutput
{
    VertexOut VSOut;
};

[maxvertexcount(3)]
void GS(triangle VSOutput In[3], inout TriangleStream<GSOutput> triStream )
{
    //
	// we're projecting the triangle face orthogonally with the dominant axis of its normal vector.
	// the end goal is to maximize the amount of generated fragments. more details at OpenGL Insights, pages 303-318
	//
	float3x3 swizzle_mat;

	const float3 edge1 = In[1].Pos.xyz - In[0].Pos.xyz;
	const float3 edge2 = In[2].Pos.xyz - In[0].Pos.xyz;
	const float3 face_normal = abs(cross(edge1, edge2)); 

	if (face_normal.x >= face_normal.y && face_normal.x >= face_normal.z) { // see: Introduction to Geometric Computing, page 33 (Ghali, 2008)
		swizzle_mat = mat3(
			float3(0.0, 0.0, 1.0),
			float3(0.0, 1.0, 0.0),
			float3(1.0, 0.0, 0.0));
	} else if (face_normal.y >= face_normal.z) {
		swizzle_mat = mat3(
			float3(1.0, 0.0, 0.0),
			float3(0.0, 0.0, 1.0),
			float3(0.0, 1.0, 0.0));
	} else {
		swizzle_mat = mat3(
			float3(1.0, 0.0, 0.0),
			float3(0.0, 1.0, 0.0),
			float3(0.0, 0.0, 1.0));
	}

	for (int i=0; i < 3; i++)
	{
        Out.VSOut = In[i];
        Out.VSOut.voxelPos = In[i].Pos.xyz;
        Out.VSOut.ShadowPos = mul(shadowmapMvp, In[i].PosW);
        Out.VSOut.Pos = float4(mul(In[i].Pos.xyz, swizzle_mat), 1.0f);
        triStream.Append( Out );
	}
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

RWTexture3D<float4> VoxelizeVolume;

inline void Write3D(RWTexture3D<float4> tex, int3 p, float4 val) 
{ 
	tex[p] = val;
}

float3 from_clipspace_to_texcoords(float3 p) {
	return 0.5f * p + float3(0.5f); 
}

void PS(VertexOut pin)
{
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

    float4 Out = float4(Lo, 1.0f);

	float3 voxelgrid_tex_pos = from_clipspace_to_texcoords(pin.voxelPos);
	int3 voxelgrid_resolution;
	VoxelizeVolume.GetDimensions(0, voxelgrid_resolution.x, voxelgrid_resolution.y, voxelgrid_resolution.z);
	Write3D(VoxelizeVolume, int3(voxelgrid_resolution * voxelgrid_tex_pos), Out);

    return Out;
}