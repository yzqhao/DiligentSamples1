

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
void main(triangle VSOutput In[3], inout TriangleStream<GSOutput> triStream )
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



RWTexture3D<uint> VoxelizeVolume;

void PS(VertexOut pin)
{
    

    return Out;
}