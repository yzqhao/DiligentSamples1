
TextureCube g_Texture;
SamplerState g_Texture_sampler; // By convention, texture samplers must use the '_sampler' suffix

cbuffer Constants
{
    float4x4 g_View;
    float4x4 g_Proj;
    float4 gEyePosW;
};

struct VSInput
{
    float3 Pos : ATTRIB0;
    float2 UV  : ATTRIB1;
};

struct PSInput 
{ 
    float4 Pos : SV_POSITION; 
    float3 TexCoords  : TEX_COORD; 
};

void VS(in  VSInput VSIn, out PSInput PSIn) 
{
    // Transform to world space.
	float4 posW = float4(VSIn.Pos, 1.0f);
    // Always center sky about camera.
	posW.xyz += gEyePosW;

    float4 pos = mul(mul(g_Proj, g_View), posW);
    PSIn.Pos = pos.xyww;
    PSIn.TexCoords  = VSIn.Pos;
}

struct PSOutput
{
    float4 Color : SV_TARGET;
};

void PS(in PSInput  PSIn, out PSOutput PSOut)
{
    float4 Color = g_Texture.Sample(g_Texture_sampler, PSIn.TexCoords, 0);
    PSOut.Color = Color;
}

