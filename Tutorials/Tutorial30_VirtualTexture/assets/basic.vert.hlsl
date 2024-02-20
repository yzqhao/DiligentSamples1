
#define MAX_PLANETS 20

struct VSInput
{
	float4 Position : ATTRIB0;
	float2 UV : ATTRIB1;
	float4 Normal : ATTRIB2;
};

struct VSOutput
{
  	float4 Position : SV_Position;
	float2 outUV : TEXCOORD0;
	float3 outNormal : NORMAL;
  	float3 outPos : POSITION;
};

cbuffer SparseTextureInfo
{
	uint Width;
	uint Height;
	uint pageWidth;
	uint pageHeight;

	uint DebugMode;
	uint ID;
	uint pad1;
	uint pad2;
	
	float4 CameraPos;
};

cbuffer uniformBlock
{
	float4x4 mvp;
	float4x4 toWorld[MAX_PLANETS];
	float4 color[MAX_PLANETS];

	// Point Light Information
	float3 lightPosition;
	float3 lightColor;
};

VSOutput VS( VSInput In )
{
	VSOutput Out;

	float4x4 tempMat = mul(mvp, toWorld[ID]);
	Out.Position = mul(tempMat, float4(In.Position.xyz, 1.0f));
	
	float4 normal = normalize(mul(toWorld[ID], float4(In.Normal.xyz, 0.0f)));
	float4 pos = mul(toWorld[ID], float4(In.Position.xyz, 1.0f));

  	Out.outUV = In.UV;
  	Out.outNormal = normal.xyz;
  	Out.outPos = pos.xyz;
	return Out;
}
