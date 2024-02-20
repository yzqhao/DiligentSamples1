
struct VertexPosUvNormal
{
	float3 PosL    : ATTRIB0;
	float2 TexC    : ATTRIB1;
    float3 NormalL : ATTRIB2;
};

cbuffer Constants
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4 gEyePosW;
};

cbuffer ObjectConstants
{
    float4x4 g_World;
    float4 Coef[16];
};

struct VertexOut
{
	float4 PosH    : SV_POSITION;
	float2 TexC    : TEXCOORD;
    float3 PosV    : TEXCOORD2;
    float3 NormalW : NORMAL;
};

VertexOut VS(VertexPosUvNormal vin)
{
    VertexOut vout = (VertexOut)0.0f;
	
    // Transform to world space.
    float4 posV = mul(g_View, mul(g_World, float4(vin.PosL, 1.0f)));
    vout.PosV = posV.xyz;

    // Assumes nonuniform scaling; otherwise, need to use inverse-transpose of world matrix.
    vout.NormalW = normalize(mul(g_World, float4(vin.NormalL, 0.0f)).xyz);

    // Transform to homogeneous clip space.
    vout.PosH = mul(g_Proj, posV);
	
    vout.TexC = vin.TexC.xy;
	
    return vout;
}

static const float PI = 3.1415926535897932384626433832795;


Texture2D BRDFLut;
SamplerState BRDFLut_sampler;

TextureCube Texture1;
TextureCube Texture2;
TextureCube Texture3;
SamplerState Texture1_sampler;
SamplerState Texture2_sampler;
SamplerState Texture3_sampler;

float3 f3(float x)
{
	return float3(x, x, x);
}

float3 FresnelSchlickRoughness(float cosTheta, float3 F0, float roughness)
{
    return F0 + (max(f3(1.0 - roughness), F0) - F0) * pow(max(1.0 - cosTheta, 0.0), 5.0);
}  

float4 PS(VertexOut pin) : SV_TARGET
{	
	if((abs(pin.NormalW.x) < 0.0001f) && (abs(pin.NormalW.y) < 0.0001f) && (abs(pin.NormalW.z) < 0.0001f))
	{
		return float4(0, 0, 0, 1);
	}

	float3 F0 = float3(0.2,0.2,0.2);
	float roughness = 0.2;
	float3 N = normalize(pin.NormalW);
	float3 V = -normalize(pin.PosV);
	float3 F        = FresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
	float2 envBRDF  = BRDFLut.Sample(BRDFLut_sampler, float2(max(dot(N, V), 0.0), roughness)).rg;
	float3 specular = (F * envBRDF.x + envBRDF.y);

	float3 Basis1 = Texture1.Sample(Texture1_sampler, N, 0).xyz;
	float3 Basis2 = Texture2.Sample(Texture2_sampler, N, 0).xyz;
	float3 Basis3 = Texture3.Sample(Texture3_sampler, N, 0).xyz;

	float Basis[9];
	Basis[0]  = Basis1.x / PI;
	Basis[1]  = Basis1.y / PI;
	Basis[2]  = Basis1.z / PI;
	Basis[3]  = Basis2.x / PI;
	Basis[4]  = Basis2.y / PI;
	Basis[5]  = Basis2.z / PI;
	Basis[6]  = Basis3.x / PI;
	Basis[7]  = Basis3.y / PI;
	Basis[8]  = Basis3.z / PI;

	float3 color = float3(0,0,0);
	for (int i = 0; i < 9; i++)
		color += Coef[i].xyz * Basis[i];

	return float4(1 * color + 0.0 * specular,1.0);
}