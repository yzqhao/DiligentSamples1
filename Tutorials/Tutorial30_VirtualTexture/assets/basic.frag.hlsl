

Texture2D SparseTexture;
SamplerState SparseTexture_sampler;

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

struct PsIn
{    
  float4 Position : SV_Position;
  float2 UV : TEXCOORD0;
	float3 Normal : NORMAL;
  float3 Pos : POSITION;
};

cbuffer VTBufferInfo
{
	uint TotalPageCount;
	uint CurrentFrameOffset;
};

RWStructuredBuffer<uint> VTVisBuffer;


#define SampleClampedSparseTex2D(TEX, SMP, P, L, RC) ((TEX).Sample((SMP), (P), 0, L, RC))

float4 PS( PsIn In ) : SV_TARGET
{
	float4 Out;
  float4 color = float4(0.0, 0.0, 0.0, 0.0);
  float4 outColor = float4(0.0, 0.0, 0.0, 0.0);

  uint minDim     = min(Width, Height);
  uint minPageDim = min(pageWidth, pageHeight);

  float maxLod = floor(log2(float(minDim)) - log2(float(minPageDim)));
  // float LOD = textureQueryLod(sampler2D(SparseTexture, uSampler0), In.UV).y;
  // float LOD = SparseTexture.CalculateLevelOfDetailUnclamped(uSampler0, In.UV);
  float LOD = SparseTexture.CalculateLevelOfDetailUnclamped(SparseTexture_sampler, In.UV);
  LOD = clamp(LOD, 0.0f, maxLod);

  float minLod = floor(LOD);
	minLod = clamp(minLod, 0.0f, maxLod);

  float localMinLod = minLod;
  
  uint residencyCode = 0; 

  [loop]
  do
  {
    // color = SampleLvlTex2D(SparseTexture, uSampler0, In.UV, 0, min(localMinLod, maxLod), residencyCode);
    color = SparseTexture.Sample(SparseTexture_sampler, In.UV, 0, min(localMinLod, maxLod), residencyCode);
    localMinLod += 1.0f;

    if(localMinLod > maxLod)
      break;

  }while(!CheckAccessFullyMapped(residencyCode));

  uint pageOffset = 0;

  uint pageCountWidth = Width / pageWidth;
  uint pageCountHeight = Height / pageHeight;

  for(int i = 1; i <= int(minLod); i++)
  {
    pageOffset += pageCountWidth * pageCountHeight;

    pageCountWidth = pageCountWidth / 2;
    pageCountHeight = pageCountHeight /2;
  }

  float debugLineWidth = 0.025 / (minLod + 1.0f);

  pageOffset += uint(float(pageCountWidth) * In.UV.x) + uint(float(pageCountHeight) * In.UV.y) * pageCountWidth;

  float NoL = max(dot(In.Normal, -normalize(In.Pos.xyz)), 0.05f);  
  
  float4 debugColor;

  if(DebugMode == 1)
  {
    float4 resultColor = float4(0.0, 0.0, 0.0, 0.0);

    float2 modVal = frac(In.UV * float2(float(pageCountWidth), float(pageCountHeight))) + float2(debugLineWidth, debugLineWidth);

    if(modVal.x < debugLineWidth * 2.0f || modVal.y < debugLineWidth * 2.0f )
    {
      outColor.rgb = lerp(float3(1.0, 0.0, 0.0), float3(0.0, 1.0, 0.0), LOD / maxLod);
      outColor.a = 1.0;
    }

    outColor = lerp(float4(color.rgb * NoL, color.a), float4(outColor.rgb, color.a), outColor.a);
  }  
  else
  {
    outColor = float4(color.rgb * NoL, color.a);
  }

  if (pageOffset < TotalPageCount)
  {
    (VTVisBuffer[(pageOffset) * 2])  = 1;
  }
  Out = outColor;
	return (Out);
}