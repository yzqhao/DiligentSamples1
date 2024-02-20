#include "Common.hlsl"

cbuffer cbPerObject
{
    float4x4 g_World;
    float4 color;
};

// Vertex shader takes two inputs: vertex position and uv coordinates.
// By convention, Diligent Engine expects vertex shader inputs to be 
// labeled 'ATTRIBn', where n is the attribute number.
struct VSInput
{
    float3 Pos : ATTRIB0;
};

struct PSInput 
{ 
    float4 Pos : SV_POSITION; 
    float4 color  : TEX_COORD;
};

// Note that if separate shader objects are not supported (this is only the case for old GLES3.0 devices), vertex
// shader output variable name must match exactly the name of the pixel shader input variable.
// If the variable has structure type (like in this example), the structure declarations must also be identical.
void main(in  VSInput VSIn,
          out PSInput PSIn) 
{
    PSIn.Pos = mul(gViewProj, mul(g_World, float4(VSIn.Pos,1.0)));
    PSIn.color = color;
}
