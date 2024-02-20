
struct VertexPosUv
{
	float3 PosL    : ATTRIB0;
	float2 TexC    : ATTRIB1;
};

struct VSOutput
{
	float4 Position : SV_Position;
	float2 uv : TEXCOORD0;
};

VSOutput main( VertexPosUv In )
{
	VSOutput Out;

	Out.Position = float4(In.PosL, 1.0);
	Out.uv = In.TexC;

	return (Out);
}
