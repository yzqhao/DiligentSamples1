

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

cbuffer HzbConst
{
	uint lst_level;
    uint tex_width;
    uint width;
    uint height;
};

Texture2D level_deps[13] : register(t3);   // 84 material tex + 4 gbuffer + 1 depth + 1 brdf lut
SamplerState level_deps_sampler : register(s3);

VertexOutPosUv VS(VertexPosUv vin)
{
    VertexOutPosUv vout = (VertexOutPosUv)0.0f;

    vout.PosH = float4(vin.PosL, 1.0f);
    vout.TexC = vin.TexC.xy;
	
    return vout;
}


float2 mip1_to_mip0(float2 p){
    return p * float2(width, height)/float2(tex_width, tex_width);
}

float PS(VertexOutPosUv pin) : SV_Depth
{
    float2 p = float2(pin.TexC);

    if(lst_level==0) p=mip1_to_mip0(p);
    else p=2*p;

    float x = level_deps[lst_level].Sample(level_deps_sampler, p+0.5).x;
    float y = level_deps[lst_level].Sample(level_deps_sampler, p+0.5+float2(1,0)).x;
    float z = level_deps[lst_level].Sample(level_deps_sampler, p+0.5+float2(1,1)).x;
    float w = level_deps[lst_level].Sample(level_deps_sampler, p+0.5+float2(0,1)).x;

    float min_z=min(min(x,y),min(z,w));
    return min_z;
}
