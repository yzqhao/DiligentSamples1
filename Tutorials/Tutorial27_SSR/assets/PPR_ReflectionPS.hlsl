
#define NUM_SAMPLE 1
#define MAX_PLANES 4

#define PI 3.1415926535897932384626422832795028841971
#define RADIAN 0.01745329251994329576923690768489

cbuffer cbExtendCamera : register(b0)
{
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 InvViewProjMat;

	float4 cameraWorldPos;
	float4 viewPortSize;
};


Texture2D<float4> SceneTexture : register(t1);
RWBuffer<uint> IntermediateBuffer : register(u2);
Texture2D<float4> DepthTexture : register(t3);

SamplerState SceneTexture_sampler : register(s0);
SamplerState DepthTexture_sampler : register(s1);

struct PlaneInfo
{
	float4x4 rotMat;
	float4 centerPoint;
	float4 size;
};

cbuffer planeInfoBuffer : register(b4)
{
	PlaneInfo planeInfo[MAX_PLANES];
	uint numPlanes;
	uint pad00;
	uint pad01;
	uint pad02;
};

cbuffer cbProperties : register(b5)
{
	uint renderMode;
	float useHolePatching;
	float useExpensiveHolePatching;
	float useNormalMap;

	float intensity;
	float useFadeEffect;
	float padding01;
	float padding02;
};

inline float4 getCol(in float4x4 M, const uint i) { return float4(M[0][i], M[1][i], M[2][i], M[3][i]); }


bool intersectPlane(in uint index, in float3 worldPos, in float2 fragUV, out float4 hitPos, out float2 relfectedUVonPlanar) 
{

	PlaneInfo thisPlane = planeInfo[index];

	// assuming vectors are all normalized
	//float4x4 thisPlanesMat = thisPlane.rotMat;
	float3 normalVec = getCol(thisPlane.rotMat, 2).xyz;

	float3 centerPoint = thisPlane.centerPoint.xyz;

	float3 rO = cameraWorldPos.xyz;
	float3 rD = normalize(worldPos - rO);
	float3 rD_VS = mul((float3x3)(viewMat), rD);
	

	//hitPos = float4(  normalVec, 0.0);

	if(rD_VS.z < 0.0)
		return false;	

    float denom = dot(normalVec, rD); 

    if (denom < 0.0)
	{ 
		

        float3 p0l0 = centerPoint - rO; 
        float t = dot(normalVec, p0l0) / denom; 

		if(t <= 0.0)
			return false;

		float3 hitPoint = rO + rD*t;	

		float3 gap = hitPoint - centerPoint;
		
		float xGap = dot(gap, getCol(thisPlane.rotMat, 0).xyz);
		float yGap = dot(gap, getCol(thisPlane.rotMat, 1).xyz);


		float width = thisPlane.size.x * 0.5;
		float height = thisPlane.size.y * 0.5;

		float4 reflectedPos;

		if( (abs(xGap) <= width) && (abs(yGap) <= height))
		{
			hitPos = float4(hitPoint, 1.0);
			reflectedPos = mul(viewProjMat, hitPos);
			reflectedPos /= reflectedPos.w;

			reflectedPos.xy = float2( (reflectedPos.x + 1.0) * 0.5, (1.0 - reflectedPos.y) * 0.5);

			float depth = DepthTexture.Sample(DepthTexture_sampler, reflectedPos.xy).r;

			if(depth <= reflectedPos.z)
				return false;
			
			if( reflectedPos.x < 0.0 || reflectedPos.y < 0.0  || reflectedPos.x > 1.0 || reflectedPos.y > 1.0 )
				return false;
			else
			{
				relfectedUVonPlanar = float2(xGap / width, yGap / height) * 0.5 + float2(0.5, 0.5);
				relfectedUVonPlanar *= float2(thisPlane.size.zw);

				return true; 
			}			
		}	
		else
			return false;
    } 
	else	
		return false; 
} 

float4 unPacked(in uint unpacedInfo, in float2 dividedViewSize, out uint CoordSys)
{
	float YInt = float(unpacedInfo >> 20);
	int YFrac = int( (unpacedInfo & 0x000E0000) >> 17 );
	
	uint uXInt = (unpacedInfo & 0x00010000) >> 16;

	float XInt = 0.0;

	if(uXInt == 0)
	{
		XInt = float( int(  (unpacedInfo & 0x0001FFE0) >> 5 ));
	}
	else
	{
		XInt = float(int( ((unpacedInfo & 0x0001FFE0) >> 5) | 0xFFFFF000));
	}
	
	int XFrac = int( (unpacedInfo & 0x0000001C) >> 2 );

	float Yfrac = YFrac * 0.125;
	float Xfrac = XFrac * 0.125;
	
	CoordSys = unpacedInfo & 0x00000003;

	float2 offset = float2(0.0, 0.0);

	if(CoordSys == 0)
	{
		offset = float2( (XInt) / dividedViewSize.x, (YInt)  / dividedViewSize.y);
		//offset = float2(XInt, YInt);
	}
	else if(CoordSys == 1)
	{
		offset = float2( (YInt) / dividedViewSize.x, (XInt) / dividedViewSize.y);
		//offset = float2(0.0, 1.0);
	}
	else if(CoordSys == 2)
	{
		offset = float2( (XInt) / dividedViewSize.x, -(YInt) / dividedViewSize.y);
		//offset = float2(0.5, 0.5);
	}
	else if(CoordSys == 3)
	{
		offset = float2( -(YInt) / dividedViewSize.x, (XInt) / dividedViewSize.y);
		//offset = float2(1.0, 1.0);
	}

	return float4(offset, Xfrac, Yfrac);
}

float4 getWorldPosition(float2 UV, float depth)
{
	float4 worldPos = mul( InvViewProjMat, float4(UV.x*2.0 - 1.0, (1.0 - UV.y) * 2.0 - 1.0, depth, 1.0));
	worldPos /= worldPos.w;

	return worldPos;
}

float fade(float2 UV)
{
	float2 NDC = UV * 2.0 - float2(1.0, 1.0);

	return clamp( 1.0 - max( pow( NDC.y * NDC.y, 4.0) , pow( NDC.x * NDC.x, 4.0)) , 0.0, 1.0); 
}

struct VSOutput
{
	float4 Position : SV_Position;
	float2 uv : TEXCOORD0;
};

#define UINT_MAX 4294967295
#define FLT_MAX  3.402823466e+38F

float4 main( VSOutput In ) : SV_TARGET
{	
	float4 outColor;
	outColor =  float4(0.0, 0.0, 0.0, 0.0);
	
	uint screenWidth = uint( viewPortSize.x );
	// uint screenHeight = uint( viewPortSize).y );
	
	uint indexY = uint( In.uv.y * viewPortSize.y );
	uint indexX = uint( In.uv.x * viewPortSize.x );
	
	uint index = indexY * screenWidth +  indexX;

	uint bufferInfo = IntermediateBuffer[index];	
	//InterlockedAdd(IntermediateBuffer)[index], 0, bufferInfo);

	bool bIsInterect = false;

	uint CoordSys;
	float2 offset = unPacked( bufferInfo , viewPortSize.xy, CoordSys).xy;

	float depth =  DepthTexture.Sample(DepthTexture_sampler, In.uv).r;

	float4 worldPos = getWorldPosition(In.uv, depth);

	float4 HitPos_WS;
	float2 UVforNormalMap = float2(0.0, 0.0);

	float4 minHitPos_WS;
	float2 minUVforNormalMap = UVforNormalMap;

	float minDist = 1000000.0;

	//Check if current pixel is in the bound of planars
	for(uint i = 0; i < numPlanes; i++)
	{	
		if(intersectPlane( i, worldPos.xyz, In.uv, HitPos_WS, UVforNormalMap))
		{
			float localDist =  distance(HitPos_WS.xyz, cameraWorldPos.xyz);
			if( localDist <  minDist )
			{
				minDist = localDist;
				minHitPos_WS = HitPos_WS;
				minUVforNormalMap = UVforNormalMap;
			}
			bIsInterect = true;
		}
	}
	

	//If is not in the boundary of planar, exit
	if(!bIsInterect)
	{
		//clear IntermediateBuffer
		InterlockedMax(IntermediateBuffer[index] , UINT_MAX);
		return (outColor);
	}	
		

	float2 relfectedUV = In.uv + offset.xy;

	float offsetLen = FLT_MAX;

	if( bufferInfo < UINT_MAX)
	{
		//values correction
		float correctionPixel = 1.0;

		if(CoordSys == 0)
			relfectedUV = relfectedUV.xy + float2(0.0, correctionPixel/viewPortSize.y);
		else if(CoordSys == 1)
			relfectedUV = relfectedUV.xy + float2(correctionPixel/viewPortSize.x, 0.0);
		else if(CoordSys == 2)
			relfectedUV = relfectedUV.xy - float2(0.0, correctionPixel/viewPortSize.y);
		else if(CoordSys == 3)
			relfectedUV = relfectedUV.xy - float2(correctionPixel/viewPortSize.x, 0.0);

		offsetLen = length(offset.xy);
	
		outColor = SceneTexture.SampleLevel(SceneTexture_sampler, relfectedUV, 0);
		
		if(useFadeEffect > 0.5 )
			outColor *= fade(relfectedUV);
	
		outColor.w = offsetLen;	
		
	}
	else
	{
		outColor.w = FLT_MAX;
	}

	//clear IntermediateBuffer
	InterlockedMax(IntermediateBuffer[ index ] , UINT_MAX);
	return (outColor);
}
