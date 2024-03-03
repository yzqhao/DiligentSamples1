
struct VertexPos
{
	float3 PosL    : ATTRIB0;
};

struct VertexOut 
{ 
    float4 Pos     : SV_POSITION; 
	float4 PosW    : TEXCOORD;
};

cbuffer CameraCB
{
	float4x4 viewPorj;
    float4 cameraPos;
};

void VS(in VertexPos vin, out VertexOut PSIn) 
{
    PSIn.PosW = float4(vin.PosL, 1.0f);
    // Always center sky about camera.
	float4 posW = float4(PSIn.PosW.xyz + cameraPos, 1.0);
    PSIn.Pos = mul(viewPorj, posW);
}

struct PixelOut
{
	float4 OutColor  : SV_Target0;
};

PixelOut PS(VertexOut pin)
{
    PixelOut Out;

    Out.OutColor = float4(pin.PosW.xyz, 1.0);

    return Out;
}