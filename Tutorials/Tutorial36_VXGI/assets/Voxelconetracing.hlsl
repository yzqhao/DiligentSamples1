#include "CameraCB.hlsl"
#include "LightingUtil.hlsl"

struct VertexPosUv
{
	float3 PosL    : ATTRIB0;
	float2 TexC    : ATTRIB1;
};

struct VertexOutPosUv
{
	float4 PosH    : SV_POSITION0;
	float2 TexC    : TEXCOORD0;
};

VertexOutPosUv VS(VertexPosUv vin)
{
    VertexOutPosUv vout = (VertexOutPosUv)0.0f;

    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC.xy;
	
    return vout;
}

Texture2D AlbedoTexture;
SamplerState AlbedoTexture_sampler;
Texture2D NormalTexture;
SamplerState NormalTexture_sampler;
Texture2D RoughnessTexture;
SamplerState RoughnessTexture_sampler;
Texture2D DepthTexture;
SamplerState DepthTexture_sampler;
Texture3D VoxelGridTexture;
SamplerState VoxelGridTexture_sampler;

Texture2D<float> ShadowMaps;
SamplerState ShadowMaps_sampler;


// See http://simonstechblog.blogspot.com/2013/01/implementing-voxel-cone-tracing.html
static const int TOTAL_DIFFUSE_CONES = 6;
static const float3 DIFFUSE_CONE_DIRECTIONS[TOTAL_DIFFUSE_CONES] = { float3(0.0f, 1.0f, 0.0f), float3(0.0f, 0.5f, 0.866025f), float3(0.823639f, 0.5f, 0.267617f), float3(0.509037f, 0.5f, -0.7006629f), float3(-0.50937f, 0.5f, -0.7006629f), float3(-0.823639f, 0.5f, 0.267617f) };
static const float DIFFUSE_CONE_WEIGHTS[TOTAL_DIFFUSE_CONES] = { PI / 4.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f, 3.0f * PI / 20.0f,  3.0f * PI / 20.0f, 3.0f * PI / 20.0f };

struct Cone_Settings
{
	float aperture;
	float sampling_factor;
	float distance_offset;
	float max_distance;
	float result_intensity;
	int is_enabled;
	float _pad0;
	float _pad1;
};

cbuffer Settings
{
	Cone_Settings gDiffuse;
	Cone_Settings gSpecular;
	Cone_Settings gSoftshadows;
	Cone_Settings gAo;
	float3 u_scene_voxel_scale;
	int trace_ao_separately;
	float3 u_ambient_light;
	float gamma;
	float hard_shadow_bias;
	float direct_light_intensity;
	int voxel_grid_resolution;
	int max_mipmap_level;
	int enable_direct_light;
	int enable_hard_shadows;
};

bool is_inside_clipspace(const float3 p, float e) {
	return abs(p.x) < 1 + e && abs(p.y) < 1 + e && abs(p.z) < 1 + e;
}

//
// CONE TRACE FUNCTION
// note: aperture = tan(radians * 0.5)
//
float4 trace_cone(const float3 start_clip_pos, float3 direction, float aperture, float distance_offset, float distance_max, float sampling_factor)
{
	aperture = max(0.1f, aperture); // inf loop if 0
	direction = normalize(direction);
	float distance = distance_offset; // avoid self-collision
	float3 accumulated_color = float3(0,0,0);
	float accumulated_occlusion = 0.0f;
	
	while (distance <= distance_max && accumulated_occlusion < 1.0f)
	{
		float3 cone_clip_pos = start_clip_pos + (direction * distance);
		float3 cone_voxelgrid_pos = 0.5f * cone_clip_pos + float3(0.5, 0.5, 0.5); // from clipspace -1.0...1.0 to texcoords 0.0...1.0

		float diameter = 2.0f * aperture * distance; 
		float mipmap_level = log2(diameter * voxel_grid_resolution);
		float4 voxel_sample = VoxelGridTexture.SampleLevel(VoxelGridTexture_sampler, cone_voxelgrid_pos, min(mipmap_level, max_mipmap_level));

		// front to back composition
		accumulated_color += (1.0f - accumulated_occlusion) * voxel_sample.rgb; 
		accumulated_occlusion += (1.0f - accumulated_occlusion) * voxel_sample.a; 

		distance += diameter * sampling_factor;
	}

	accumulated_occlusion = min(accumulated_occlusion, 1.0f);
	return float4(accumulated_color, accumulated_occlusion);
}

float trace_shadow_cone(float3 from, float3 direction, float distance)
{
	float4 s = trace_cone(from, direction, gSoftshadows.aperture, gSoftshadows.distance_offset, gSoftshadows.max_distance * distance, gSoftshadows.sampling_factor);
	return 1.0f - s.a;
}

float3 calc_indirect_specular(float3 worldPos, float3 voxelpos, float3 normal, float3 directSpecular)
{
	float3 viewDirection = normalize(worldPos - gEyePosW);
	float3 coneDirection = normalize(reflect(viewDirection, normal));
  	float4 specularIntensity = float4(1,1,1,1);

	float aperture = gSpecular.aperture;

	float3 start_clip_pos = voxelpos + (normal * gSpecular.distance_offset);
	
	float4 specularCol = trace_cone(start_clip_pos, coneDirection, aperture, gSpecular.distance_offset, gSpecular.max_distance, gSpecular.sampling_factor);
	//specularCol.rgb *= directSpecular.rgb;

	return specularCol.rgb;
}

float4 calc_indirect_diffuse(float3 voxelpos, float3 normal)
{
	float4 accumulated_color = float4(0,0,0,0);

	// rotate cone around the normal
	float3 guide = float3(0.0f, 1.0f, 0.0f);
	if (abs(dot(normal, guide)) == 1.0f)
	  guide = float3(0.0f, 0.0f, 1.0f);

	// find a tangent and a bitangent
	float3 right = normalize(guide - dot(normal, guide) * normal);
	float3 up = cross(right, normal);

	for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++)
	{
		float3 coneDirection = normal;
		coneDirection += DIFFUSE_CONE_DIRECTIONS[i].x * right + DIFFUSE_CONE_DIRECTIONS[i].z * up;
		coneDirection = normalize(coneDirection);

		float3 start_clip_pos = voxelpos + (normal * gDiffuse.distance_offset);
		accumulated_color += trace_cone(start_clip_pos, coneDirection, gDiffuse.aperture, gDiffuse.distance_offset, gDiffuse.max_distance, gDiffuse.sampling_factor) * DIFFUSE_CONE_WEIGHTS[i];
	}

	return accumulated_color;
}

float calc_ambient_occlusion(float3 normal, float3 voxelpos) // this is also calculated during gDiffuse tracing, but we can do it separately with different settings too
{
	float4 accumulated_color = float4(0,0,0,0);

	float3 guide = float3(0.0f, 1.0f, 0.0f);
	if (abs(dot(normal, guide)) == 1.0f)
		guide = float3(0.0f, 0.0f, 1.0f);

	float3 right = normalize(guide - dot(normal, guide) * normal);
	float3 up = cross(right, normal);

	for (int i = 0; i < TOTAL_DIFFUSE_CONES; i++)
	{
		float3 coneDirection = normal;
		coneDirection += DIFFUSE_CONE_DIRECTIONS[i].x * right + DIFFUSE_CONE_DIRECTIONS[i].z * up;
		coneDirection = normalize(coneDirection);

		float3 start_clip_pos = voxelpos + (normal * gAo.distance_offset);
		accumulated_color += trace_cone(start_clip_pos, coneDirection, gAo.aperture, gAo.distance_offset, gAo.max_distance, gAo.sampling_factor) * DIFFUSE_CONE_WEIGHTS[i];
	}

	return accumulated_color.a;
}

float3 calc_direct_light(float3 albedo, float3 N, float4 roughnessColor, float3 worldPos, float _roughness, float _metalness, out float3 directSpecular)
{
	float3 V = normalize(gEyePosW.xyz - worldPos);
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
		float3 gSpecular = nominator / denominator;
		float3 kS = F;
		float3 kD = float3(1.0, 1.0, 1.0) - kS;
		kD *= 1.0f - _metalness;
		float NdotL = max(dot(N, L), 0.0);
		if(NdotL>0.0f)
		{
			Lo += (kD * albedo / PI + gSpecular) * radiance * NdotL;
		}
		else
		{
			Lo += float3(0.0f, 0.0f, 0.0f);
		}

		directSpecular += gSpecular;
	}

    return Lo;
}

float calc_visibility(float4 shadowcoord)
{
	return ShadowMaps.Sample(ShadowMaps_sampler, float3(shadowcoord.xy, (shadowcoord.z - hard_shadow_bias) / shadowcoord.w));
}

float3 getWorldPositionFromDepth(float2 ndc, float sceneDepth )
{
	float4 worldPos = mul(gInvViewProj, float4(ndc, sceneDepth, 1.0));
	worldPos /= worldPos.w;

	return float3(worldPos.xyz);
}

struct PixelOut
{
	float4 OutColor  : SV_Target0;
};

PixelOut PS(VertexOutPosUv pin)
{
    PixelOut Out = (PixelOut)0;

	float3 albedo = AlbedoTexture.Sample(AlbedoTexture_sampler, pin.TexC).rgb;
	float4 normalColor = NormalTexture.Sample(NormalTexture_sampler, pin.TexC);
	float4 roughnessColor = RoughnessTexture.Sample(RoughnessTexture_sampler, pin.TexC);
    float depth = DepthTexture.Sample(DepthTexture_sampler, pin.TexC).r;

	float _roughness = roughnessColor.r;
	float _metalness = normalColor.a;
	float3 N = normalize(normalColor.rgb);

	float2 ndc = float2(pin.TexC.x * 2.0 - 1.0, (1.0 - pin.TexC.y) * 2.0 - 1.0);
	float3 worldPos = getWorldPositionFromDepth(ndc, depth);

	float3 voxelpos = (worldPos * u_scene_voxel_scale);

    float4 shadowcoord = float4(0,0,0,1);
    float visibility = 1.0f;
	if (enable_hard_shadows == 1) {
        shadowcoord = mul(gLightViewProj, float4(worldPos, 1.0f));
		visibility = calc_visibility(shadowcoord);
	}

	float3 direct_diffuse_color = float3(0,0,0);
	float4 indirect_specular_color = float4(0,0,0,0);
	float4 indirect_diffuse_color = float4(0,0,0,0);
	float4 indirect_light = float4(0.0f, 0.0f, 0.0f, 1.0f); // alpha component == ambient occlusion

	bool only_render_ao = (
		enable_direct_light == 0 &&
		gDiffuse.is_enabled == 0 &&
		gSpecular.is_enabled == 0);

	if (only_render_ao)
	{
		if (trace_ao_separately == 1) {
			indirect_light.a = clamp(1.0f - calc_ambient_occlusion(N, voxelpos), 0.0f, 1.0f);
		} else {
			indirect_light = calc_indirect_diffuse(voxelpos, N);
			indirect_light.a = clamp(1.0f - indirect_light.a, 0.0f, 1.0f);
			indirect_light.rgb *= albedo;
		}

		indirect_light.rgb = float3(1,1,1);
		Out.OutColor = float4(indirect_light.a,indirect_light.a,indirect_light.a, 1.0f);
	}
	else
	{
		float3 directSpecular = float3(0,0,0);

		if (enable_direct_light == 1)
			direct_diffuse_color = albedo * calc_direct_light(albedo, N, roughnessColor, worldPos, _roughness, _metalness, directSpecular);

		float ao = 0.0f;

		if (gDiffuse.is_enabled == 1) {
			indirect_diffuse_color = calc_indirect_diffuse(voxelpos, N);
			ao = indirect_diffuse_color.a;
			indirect_diffuse_color.rgb = albedo * gDiffuse.result_intensity * indirect_diffuse_color.rgb;
		}

		if (gSpecular.is_enabled == 1)
			indirect_specular_color.rgb = albedo * gSpecular.result_intensity * calc_indirect_specular(worldPos, voxelpos, N, directSpecular);

		indirect_light = indirect_specular_color + indirect_diffuse_color;
		indirect_light.a = 1.0f;

		if (gAo.is_enabled == 1) {
			if (trace_ao_separately == 1 || gDiffuse.is_enabled == 0) 
				indirect_light.a = clamp(1.0f - calc_ambient_occlusion(N, voxelpos), 0.0f, 1.0f);
			else
				indirect_light.a = clamp(1.0f - ao, 0.0f, 1.0f);
		}

		float3 ambient_light = float3(0,0,0);//albedo * u_ambient_light * indirect_light.a;
		float3 total_light = ambient_light.rgb + (visibility * direct_light_intensity * direct_diffuse_color.rgb) + indirect_light.rgb;

		total_light = pow(total_light, float3(1.0f / gamma, 1.0f / gamma, 1.0f / gamma));
		Out.OutColor = float4(total_light, 1.0f);
	}

    return Out;
}

