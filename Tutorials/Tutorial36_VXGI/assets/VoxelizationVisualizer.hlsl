

Texture3D TexVoxelgrid;
SamplerState TexVoxelgrid_sampler;

Texture2D TexCubeBack;
SamplerState TexCubeBack_sampler;

Texture2D TexCubeFront;
SamplerState TexCubeFront_sampler;

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

cbuffer VoxelizationSettings
{
	int visualize_mipmap_level;
};

cbuffer CameraCB
{
	float3 u_camera_world_position;
};

VertexOutPosUv VS(VertexPosUv vin)
{
    VertexOutPosUv vout = (VertexOutPosUv)0.0f;

    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC.xy;
	
    return vout;
}

struct PixelOut
{
	float4 OutColor  : SV_Target0;
};

bool is_inside_voxelgrid(const float3 p) {
	return abs(p.x) < 1.1f && abs(p.y) < 1.1f && abs(p.z) < 1.1f;
}

float3 f3(float x)
{
    return float3(x,x,x);
}

PixelOut PS(VertexOutPosUv pin)
{
    PixelOut Out;

    float4 accumulated_color = float4(0,0,0,0);
	float3 ray_origin = is_inside_voxelgrid(u_camera_world_position) ? u_camera_world_position : TexCubeFront.Sample(TexCubeFront_sampler, pin.TexC).xyz;
	float3 ray_end = TexCubeBack.Sample(TexCubeBack_sampler, pin.TexC).xyz;
	float3 ray_direction = normalize(ray_end - ray_origin);

	const float ray_step_size = 0.003f;
	int total_samples = int(length(ray_end - ray_origin) / ray_step_size);

	for (int i=0; i < total_samples; i++)
	{
		float3 uvw = (ray_origin + ray_direction * ray_step_size * i);
		float4 texSample = TexVoxelgrid.SampleLevel(TexVoxelgrid_sampler, (uvw + f3(1.0f)) * f3(0.5), visualize_mipmap_level);	

		if (texSample.a > 0) {
			texSample.rgb /= texSample.a;
			accumulated_color.rgb = accumulated_color.rgb + (1.0f - accumulated_color.a) * texSample.a * texSample.rgb;
			accumulated_color.a   = accumulated_color.a   + (1.0f - accumulated_color.a) * texSample.a;
		}

		if (accumulated_color.a > 0.95) // early exit
			break;
	}

	accumulated_color.rgb = pow(accumulated_color.rgb, f3(1.0f / 2.2f));
	Out.OutColor = accumulated_color;

    return Out;
}