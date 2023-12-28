
cbuffer cbExtendCamera : register(b0)
{
	float4x4 viewMat;
	float4x4 projMat;
	float4x4 viewProjMat;
	float4x4 InvViewProjMat;

	float4 cameraWorldPos;
	float4 viewPortSize;
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

struct VSOutput
{
	float4 Position : SV_Position;
	float2 uv : TEXCOORD0;
};

Texture2D<float4> SceneTexture : register(t0);
Texture2D<float4> SSRTexture : register(t1);
Texture2D<float4> SSRPointTexture : register(t2);

SamplerState SceneTexture_sampler : register(s0);     //bilinearSampler
SamplerState SSRTexture_sampler : register(s1);   //bilinearSampler
SamplerState SSRPointTexture_sampler : register(s2);   //nearestSampler

float4 main( VSOutput In ) : SV_TARGET
{	
	float4 outColor;
    outColor = float4(0.0, 0.0, 0.0, 0.0);
	float4 ssrColor = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv);	
	
	if(renderMode == 0)
	{
        outColor = SceneTexture.Sample(SceneTexture_sampler, In.uv);
        outColor = outColor / (outColor + float4(1,1,1,1));

        outColor.w = 1.0f;
		return outColor;
	}		
	else if(renderMode == 1)
	{
	    outColor = float4(0.0, 0.0, 0.0, 0.0);
	}
	
	if(useHolePatching < 0.5)
	{
	    outColor.w = 1.0;

		if(ssrColor.w > 0.0)
		{
		    outColor = ssrColor;
		}
	}
	else if(ssrColor.w > 0.0)
	{
		float threshold = ssrColor.w;
		float minOffset = threshold;
		

		float4 neighborColor00 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv + float2(1.0/viewPortSize.x, 0.0));
		float4 neighborColorB00 = SSRTexture.Sample(SSRTexture_sampler, In.uv + float2(1.0/viewPortSize.x, 0.0));
		if(neighborColor00.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor00.w);			
		}

		float4 neighborColor01 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv - float2(1.0/viewPortSize.x, 0.0));
		float4 neighborColorB01 = SSRTexture.Sample(SSRTexture_sampler, In.uv - float2(1.0/viewPortSize.x, 0.0));
		if(neighborColor01.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor01.w);			
		}

		float4 neighborColor02 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv + float2(0.0, 1.0/viewPortSize.y));
		float4 neighborColorB02 = SSRTexture.Sample(SSRTexture_sampler, In.uv + float2(0.0, 1.0/viewPortSize.y));
		if(neighborColor02.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor02.w);			
		}

		float4 neighborColor03 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv - float2(0.0, 1.0/viewPortSize.y));
		float4 neighborColorB03 = SSRTexture.Sample(SSRTexture_sampler, In.uv - float2(0.0, 1.0/viewPortSize.y));
		if(neighborColor03.w > 0.0)
		{
			minOffset = min(minOffset, neighborColor03.w);			
		}

		float4 neighborColor04 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv + float2(1.0/viewPortSize.x, 1.0/viewPortSize.y));
		float4 neighborColorB04 = SSRTexture.Sample(SSRTexture_sampler, In.uv + float2(1.0/viewPortSize.x, 1.0/viewPortSize.y));


		float4 neighborColor05 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv + float2(1.0/viewPortSize.x, -1.0/viewPortSize.y));
		float4 neighborColorB05 = SSRTexture.Sample(SSRTexture_sampler, In.uv +float2(1.0/viewPortSize.x, -1.0/viewPortSize.y));


		float4 neighborColor06 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv + float2(-1.0/viewPortSize.x, 1.0/viewPortSize.y));
		float4 neighborColorB06 = SSRTexture.Sample(SSRTexture_sampler, In.uv + float2(-1.0/viewPortSize.x, 1.0/viewPortSize.y));


		float4 neighborColor07 = SSRPointTexture.Sample(SSRPointTexture_sampler, In.uv - float2(1.0/viewPortSize.x, 1.0/viewPortSize.y));
		float4 neighborColorB07 = SSRTexture.Sample(SSRTexture_sampler, In.uv - float2(1.0/viewPortSize.x, 1.0/viewPortSize.y));


		bool bUseExpensiveHolePatching = useExpensiveHolePatching > 0.5;

		if(bUseExpensiveHolePatching)
		{
				
			if(neighborColor04.w > 0.0)
			{
				minOffset = min(minOffset, neighborColor04.w);			
			}

				
			if(neighborColor05.w > 0.0)
			{
				minOffset = min(minOffset, neighborColor05.w);			
			}

				
			if(neighborColor06.w > 0.0)
			{
				minOffset = min(minOffset, neighborColor06.w);			
			}

				
			if(neighborColor07.w > 0.0)
			{
				minOffset = min(minOffset, neighborColor07.w);			
			}

		}

		float blendValue = 0.5;

		if(bUseExpensiveHolePatching)
		{
			if(minOffset == neighborColor00.w)
			{
				outColor =  lerp(neighborColor00, neighborColorB00, blendValue);
			}
			else if(minOffset == neighborColor01.w)
			{
				outColor = lerp(neighborColor01, neighborColorB01, blendValue);
			}
			else if(minOffset == neighborColor02.w)
			{
				outColor = lerp(neighborColor02, neighborColorB02, blendValue);
			}
			else if(minOffset == neighborColor03.w)
			{
				outColor = lerp(neighborColor03, neighborColorB03, blendValue);
			}
			else if(minOffset == neighborColor04.w)
			{
				outColor = lerp(neighborColor04, neighborColorB04, blendValue);
			}
			else if(minOffset == neighborColor05.w)
			{
				outColor = lerp(neighborColor05, neighborColorB05, blendValue);
			}
			else if(minOffset == neighborColor06.w)
			{
				outColor = lerp(neighborColor06, neighborColorB06, blendValue);
			}
			else if(minOffset == neighborColor07.w)
			{
				outColor = lerp(neighborColor07, neighborColorB07, blendValue);
			}
		}
		else
		{
			if(minOffset == neighborColor00.w)
			{
				outColor = lerp(neighborColor00, neighborColorB00, blendValue);
			}
			else if(minOffset == neighborColor01.w)
			{
				outColor = lerp(neighborColor01, neighborColorB01, blendValue);
			}
			else if(minOffset == neighborColor02.w)
			{
				outColor = lerp(neighborColor02, neighborColorB02, blendValue);
			}
			else if(minOffset == neighborColor03.w)
			{
				outColor = lerp(neighborColor03, neighborColorB03, blendValue);
			}
		}

		//outColor *= intensity;
		
		if(minOffset <= threshold)
		    outColor.w = 1.0;
		else
		    outColor.w = 0.0;
	}
	
	if(renderMode == 3)
	{
		if(ssrColor.w <= 0.0)
		outColor = SceneTexture.Sample(SceneTexture_sampler, In.uv);
	}

	if(renderMode == 2)
	{
	    outColor = outColor * intensity + SceneTexture.Sample(SceneTexture_sampler, In.uv);	
	}

    outColor = outColor / (outColor + float4(1.0f, 1.0f, 1.0f, 1.0f));

    outColor.w = 1.0f;
	return outColor;
}