
#define MAX_PLANES 4

cbuffer cbExtendCamera : register(b0)
{
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 InvViewProjMat;
    
	float4 cameraWorldPos;
	float4 viewPortSize;
};


Texture2D<float> DepthTexture : register(t2);
SamplerState DepthTexture_sampler : register(s4);
RWBuffer<uint> IntermediateBuffer : register(u1);


struct PlaneInfo
{
	float4x4 rotMat;
	float4 centerPoint;
	float4 size;
};

cbuffer planeInfoBuffer : register(b3)
{
	PlaneInfo planeInfo[MAX_PLANES];
	uint numPlanes;
	uint pad00;
	uint pad01;
	uint pad02;
};

float getDistance(float3 planeNormal, float3 planeCenter, float3 worldPos)
{
	//plane to point
	float d = -dot(planeNormal, planeCenter);
	return (dot(planeNormal, worldPos) + d) / length(planeNormal);
}

inline float4 getCol(in float4x4 M, const uint i) { return float4(M[0][i], M[1][i], M[2][i], M[3][i]); }

bool intersectPlane(uint index, float3 worldPos, float2 fragUV, out float4 reflectedPos)
{ 
	PlaneInfo thisPlane = planeInfo[index];

	// assuming vectors are all normalized
	float3 normalVec = getCol(thisPlane.rotMat, 2).xyz;

	float3 centerPoint = thisPlane.centerPoint.xyz;
	float3 projectedWorldPos = dot(normalVec, worldPos - centerPoint) * normalVec;
	float3 target = worldPos - 2.0 * projectedWorldPos;

	//plane to point	
	float dist = getDistance(normalVec, centerPoint, target);
	
	//if target is on the upper-side of plane, false 
	if(dist >= 0.0)
	{
		return false;	
	}

	float3 rO = cameraWorldPos.xyz;
	float3 rD = normalize(target - rO);
	float3 rD_VS = mul( (float3x3)(viewMat), rD);
		
	if(rD_VS.z < 0.0)
	{
		return false;	
	}

    float denom = dot(normalVec, rD); 

    if (denom < 0.0)
	{ 
        float3 p0l0 = centerPoint - rO; 
        float t = dot(normalVec, p0l0) / denom; 

		if(t <= 0.0)
		{
			return false;
		}

		float3 hitPoint = rO + rD*t;	

		float3 gap = hitPoint - centerPoint;
		
		float xGap = dot(gap, getCol(thisPlane.rotMat, 0).xyz);
		float yGap = dot(gap, getCol(thisPlane.rotMat, 1).xyz);

		float width = thisPlane.size.x * 0.5;
		float height = thisPlane.size.y * 0.5;

		if( (abs(xGap) <= width) && (abs(yGap) <= height))
		{
			reflectedPos = mul( viewProjMat , float4(hitPoint, 1.0) );
			reflectedPos /= reflectedPos.w;

			reflectedPos.xy = float2( (reflectedPos.x + 1.0) * 0.5, (1.0 - reflectedPos.y) * 0.5);

			float depth = DepthTexture.SampleLevel(DepthTexture_sampler, reflectedPos.xy, 0).r;

			if(depth <= reflectedPos.z)
			{
				return false;
			}
			
			if( reflectedPos.x < 0.0 || reflectedPos.y < 0.0  || reflectedPos.x > 1.0 || reflectedPos.y > 1.0 )
			{
				return false;
			}
			else
			{
				//check if it is also hit from other planes
				for(uint i=0; i < numPlanes; i++ )
				{
					if(i != index)
					{						
						PlaneInfo otherPlane = planeInfo[i];
						// assuming vectors are all normalized
						
						float3 otherNormalVec = getCol(otherPlane.rotMat, 2).xyz;

						float3 otherCenterPoint = otherPlane.centerPoint.xyz;

						float innerDenom = dot(otherNormalVec, rD); 

						if (innerDenom < 0.0)
						{ 
							float3 innerP0l0 = otherCenterPoint - rO; 
							float innerT = dot(otherNormalVec, innerP0l0) / innerDenom; 

							if(innerT <= 0.0)
								continue;
							else if(innerT < t)
							{
								float3 innerhitPoint = rO + rD*innerT;	
								float3 innergap = innerhitPoint - otherCenterPoint;
		
								float innerxGap = dot(innergap, getCol(otherPlane.rotMat, 0).xyz);
								float inneryGap = dot(innergap, getCol(otherPlane.rotMat, 1).xyz);

								float innerWidth = otherPlane.size.x * 0.5;
								float innerHeight = otherPlane.size.y * 0.5;

								// if it hits other planes
								if( (abs(innerxGap) <= innerWidth) && (abs(inneryGap) <= innerHeight))
								{
									return false;
								}								
							}	
						}
					}
				}

				return true; 				
			}
		}	
		else
			return false;
    } 
	else
		return false; 
} 

float4 getWorldPosition(float2 UV, float depth)
{
	float4 worldPos = mul(InvViewProjMat ,float4(UV.x * 2.0 - 1.0, (1.0 - UV.y) * 2.0 - 1.0, depth, 1.0));
	worldPos /= worldPos.w;
	return worldPos;
}

uint packInfo(float2 offset)
{
	uint CoordSys = 0;

	uint YInt = 0;
	int YFrac = 0;
	int XInt = 0;
	int XFrac = 0;

	//define CoordSystem
	if(abs(offset.y) < abs(offset.x) )
	{
		if(offset.x < 0.0) // 3
		{
			YInt = uint(abs(offset.x));
			YFrac = int(frac(offset.x)*8.0);
			
			XInt = int(offset.y);
			XFrac = int(frac(offset.y)*8.0);

			CoordSys = 3;
		}
		else // 1
		{
			YInt = uint(offset.x);
			YFrac = int(frac(offset.x)*8.0);
			
			XInt = int(offset.y);
			XFrac = int(frac(offset.y)*8.0);

			CoordSys = 1;
		}
	}
	else	
	{
		if(offset.y < 0.0) // 2
		{
			YInt = uint(abs(offset.y));
			YFrac = int(frac(offset.y)*8.0);
			
			XInt = int(offset.x);
			XFrac = int(frac(offset.x)*8.0);

			CoordSys = 2;
		}
		else // 0
		{
			YInt = uint(offset.y);
			YFrac = int(frac(offset.y)*8.0);
			
			XInt = int(offset.x);
			XFrac = int(frac(offset.x)*8.0);

			CoordSys = 0;
		}
	}

	return  ( (YInt & 0x00000fff ) << 20) | ( (YFrac & 0x00000007) << 17) | ( (XInt & 0x00000fff) << 5) | ( (XFrac & 0x00000007 )<< 2) | CoordSys;
}

[numthreads(128, 1, 1)]
void csmain(uint3 DTid : SV_DispatchThreadID, uint GI : SV_GroupIndex, uint3 GTid : SV_GroupThreadID)
{	
	uint screenWidth = uint( viewPortSize.x );
	uint screenHeight = uint( viewPortSize.y );
	
	uint indexDX = DTid.x;	

	if(indexDX >= screenWidth * screenHeight)
		return;

	uint indexY = indexDX / screenWidth;
	uint indexX = indexDX - screenWidth * indexY;

	float2 fragUV = float2( float(indexX) / viewPortSize.x, float(indexY) / viewPortSize.y );
		
	float depth = DepthTexture.SampleLevel(DepthTexture_sampler, fragUV, 0).r;

	//if there is no obj
	if(depth >= 1.0)
		return;

	float4 worldPos = getWorldPosition(fragUV, depth);
	
	float4 reflectedPos = float4(0.0, 0.0, 0.0, 0.0);
	float2 reflectedUV;
	float2 offset;


	// float minDist = 1000000.0;
	

	for(uint i = 0; i < numPlanes; i++)
	{	
		if(intersectPlane( i, worldPos.xyz, fragUV, reflectedPos ))
		{			
			reflectedUV =  float2( reflectedPos.x * viewPortSize.x, reflectedPos.y * viewPortSize.y);
			offset = float2( (fragUV.x - reflectedPos.x) * viewPortSize.x, ( fragUV.y - reflectedPos.y) * viewPortSize.y);
			
			uint newIndex =  uint(reflectedUV.x) + uint(reflectedUV.y) * screenWidth;

			//pack info
			uint intermediateBufferValue = packInfo(offset);
			InterlockedMin(IntermediateBuffer[newIndex], intermediateBufferValue);
		}	
	}
}