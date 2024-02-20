
#define USE_RAY_DIFFERENTIALS

#ifndef SAMPLE_COUNT
#define SAMPLE_COUNT 4
#endif

#include "Packing.h.fsl"
#include "Shader_Defs.h.fsl"
#include "shading.h.fsl"
#include "ASMShader_Defs.h.fsl"

#define SHADOW_TYPE_ESM				0
#define SHADOW_TYPE_ASM				1
#define SHADOW_TYPE_MESH_BAKED_SDF	2

cbuffer lightUniformBlock
{
    float4x4 mLightViewProj;
    float4 lightPosition;
    float4 lightColor;
	float4 mLightUpVec;
	float4 mTanLightAngleAndThresholdValue;
	float3, mLightDir;
};

cbuffer cameraUniformBlock
{
    float4x4 View;
    float4x4 Project;
    float4x4 ViewProject;
    float4x4 InvView;
	float4x4 InvProj;
	float4x4 InvViewProject;
	float4 mCameraPos;
	float mNear;
	float mFarNearDiff;
	float mFarNear;
	float paddingForAlignment0;
	float2 mTwoOverRes;
};

cbuffer ASMUniformBlock
{
	float4x4 mIndexTexMat;
	float4x4 mPrerenderIndexTexMat;
	float4   mSearchVector;
	float4   mPrerenderSearchVector;
	float4   mWarpVector;
	float4   mPrerenderWarpVector;
	float4   mMiscBool;
	float    mPenumbraSize;
};

cbuffer objectUniformBlock
{
    float4x4 WorldViewProjMat;
    float4x4 WorldMat;
};

cbuffer renderSettingUniformBlock
{
    float4 WindowDimension;
    int ShadowType;
};

cbuffer ESMInputConstants
{
    float mEsmControl;
};

RES(Tex2D(float4), IndexTexture[10], UPDATE_FREQ_NONE, t20, binding = 20);
RES(Depth2D(float), DepthAtlasTexture, UPDATE_FREQ_NONE, t30, binding = 21);
RES(Tex2D(float), DEMTexture, UPDATE_FREQ_NONE, t31, binding = 22);
RES(Tex2D(float), PrerenderLodClampTexture, UPDATE_FREQ_NONE, t32, binding = 23);
RES(Tex2D(float), ESMShadowTexture, UPDATE_FREQ_NONE, t33, binding = 24);
RES(Tex2D(float2), SDFShadowTexture, UPDATE_FREQ_NONE, t19, binding = 19);

#if SAMPLE_COUNT > 1
RES(Tex2DMS(float4 SAMPLE_COUNT), vbPassTexture, UPDATE_FREQ_NONE, t18, binding = 18);
#else
RES(Tex2D(float4), vbPassTexture, UPDATE_FREQ_NONE, t18, binding = 18);
#endif

RES(ByteBuffer, vertexPos,                      UPDATE_FREQ_NONE, t0, binding = 0);
RES(ByteBuffer, vertexTexCoord,                 UPDATE_FREQ_NONE, t1, binding = 1);
RES(ByteBuffer, vertexNormal,                   UPDATE_FREQ_NONE, t2, binding = 2);
RES(ByteBuffer, vertexTangent,                  UPDATE_FREQ_NONE, t3, binding = 3);
RES(ByteBuffer, filteredIndexBuffer,            UPDATE_FREQ_PER_FRAME, t4, binding = 4);
RES(ByteBuffer, indirectMaterialBuffer,         UPDATE_FREQ_PER_FRAME, t5, binding = 5);
RES(Buffer(MeshConstants), meshConstantsBuffer, UPDATE_FREQ_NONE, t6, binding = 6);

// Per frame descriptors
RES(Buffer(uint), indirectDrawArgs[2], UPDATE_FREQ_PER_FRAME, t12, binding = 12);

#if defined(METAL) || defined(ORBIS) || defined(PROSPERO)
	RES(Tex2D(float4), diffuseMaps[MAX_TEXTURE_UNITS],  UPDATE_FREQ_NONE, t25, binding = 25);
	RES(Tex2D(float4), normalMaps[MAX_TEXTURE_UNITS],   UPDATE_FREQ_NONE, t26, binding = 26);
	RES(Tex2D(float4), specularMaps[MAX_TEXTURE_UNITS], UPDATE_FREQ_NONE, t27, binding = 27);
#else
	 RES(Tex2D(float4), diffuseMaps[MAX_TEXTURE_UNITS],  space4, t0,   binding = 25);
	 RES(Tex2D(float4), normalMaps[MAX_TEXTURE_UNITS],   space5, t257, binding = 26);
	 RES(Tex2D(float4), specularMaps[MAX_TEXTURE_UNITS], space6, t514, binding = 27);
#endif

RES(SamplerComparisonState, ShadowCmpSampler, UPDATE_FREQ_NONE, s4, binding = 11);
RES(SamplerState, clampBorderNearSampler, UPDATE_FREQ_NONE, s3, binding = 10);
RES(SamplerState, clampMiplessLinearSampler, UPDATE_FREQ_NONE, s2, binding = 8);
RES(SamplerState, clampMiplessNearSampler, UPDATE_FREQ_NONE, s1, binding = 9);

RES(SamplerState, textureSampler, UPDATE_FREQ_NONE, s0, binding = 7);

struct ASMFrustumDesc
{
	float3 mIndexCoord;
	int mStartingMip;
};

struct PsIn
{
	float4 Position : SV_Position;
	float2 ScreenPos : TEXCOORD0;
};

struct PsOut
{
    float4 FinalColor : SV_Target0;
};

float calcESMShadowFactor(float3 worldPos)
{
	float4 posLS = mul(mLightViewProj, float4(worldPos.xyz, 1.0));
	posLS /= posLS.w;
	posLS.y *= -1;
	posLS.xy = posLS.xy * 0.5 + f2(0.5);


	float2 HalfGaps = float2(0.00048828125, 0.00048828125);
	// float2 Gaps = float2(0.0009765625, 0.0009765625);

	posLS.xy += HalfGaps;

	float shadowFactor = 0.0;

	// const float esmControl = mEsmControl;

	if( AllGreaterThan(abs(posLS.xy-f2(0.5)), 0.5) )
	{
		return shadowFactor;
	}

	float4 shadowDepthSample = float4(0, 0, 0, 0);
	shadowDepthSample.x = SampleLvlTex2D(ESMShadowTexture, clampMiplessLinearSampler, posLS.xy, 0).r;
	shadowDepthSample.y = SampleLvlOffsetTex2D(ESMShadowTexture, clampMiplessLinearSampler, posLS.xy, 0, int2(1, 0)).r;
	shadowDepthSample.z = SampleLvlOffsetTex2D(ESMShadowTexture, clampMiplessLinearSampler, posLS.xy, 0, int2(0, 1)).r;
	shadowDepthSample.w = SampleLvlOffsetTex2D(ESMShadowTexture, clampMiplessLinearSampler, posLS.xy, 0, int2(1, 1)).r;
	float avgShadowDepthSample = (shadowDepthSample.x + shadowDepthSample.y + shadowDepthSample.z + shadowDepthSample.w) * 0.25f;
	shadowFactor = saturate(2.0 - exp((posLS.z - avgShadowDepthSample) * mEsmControl));
	return shadowFactor;
}


float GetASMFadeInConstant(float w)
{
	return 2.0 * frac(abs(w));
}

float PCF(float3 shadowMapCoord, float2 kernelSize )
{
	const float depthBias = -0.0005;
	//const float depthBias = -0.001f;
	//const float depthBias = -0.000f;
    float2 tapOffset[9] =
    {
        float2( 0.00, 0.00),
        float2( 1.20, 0.00),
        float2(-1.20, 0.00),
        float2( 0.00, 1.20),
        float2( 0.00,-1.20),
        float2( 0.84, 0.84),
        float2(-0.84, 0.84),
        float2(-0.84,-0.84),
        float2( 0.84,-0.84),
    };

    float tapWeight[9] =
    {
        0.120892,
        0.110858,
        0.110858,
        0.110858,
        0.110858,
        0.111050,
        0.111050,
        0.111050,
        0.111050,
    };

    float shadowFactor = 0;
    for( int i = 0; i < 9; ++i )
    {
        float2 sampleCoord = shadowMapCoord.xy + kernelSize * tapOffset[i];

		// shadowFactor += tapWeight[i] * DepthAtlasTexture.SampleCmpLevelZero( ShadowCmpSampler, sampleCoord, shadowMapCoord.z - depthBias );
		shadowFactor += tapWeight[i] * CompareTex2D(DepthAtlasTexture, ShadowCmpSampler, float3(sampleCoord, shadowMapCoord.z - depthBias));
    }
	return shadowFactor;
}

float4 SampleFrustumIndirectionTexture(ASMFrustumDesc frustumDesc, float mip)
{
	
	float lerpVal = frac(mip);
	int floorMip = int(floor(mip));

	uint index = frustumDesc.mStartingMip + floorMip;
	float4 indirectionCoordData = f4(0);
	BeginNonUniformResourceIndex(index, 10);
		indirectionCoordData =  SampleLvlTex2D(IndexTexture[ index ], clampBorderNearSampler,  float2(frustumDesc.mIndexCoord.xy), 0);
	EndNonUniformResourceIndex();

	if(lerpVal > 0.0)
	{
		index += 1;
		float4 upperIndirectionCoordData = f4(0);
		BeginNonUniformResourceIndex(index, 10);
			indirectionCoordData =  SampleLvlTex2D(IndexTexture[ index ], clampBorderNearSampler,  float2(frustumDesc.mIndexCoord.xy), 0);
		EndNonUniformResourceIndex();

		indirectionCoordData = lerp(indirectionCoordData, upperIndirectionCoordData, lerpVal);
	}
	
	return indirectionCoordData;
	
}

//indirectionCoordData = t, aka, result of sampling indirection texture using indexCoord
float3 GetASMTileCoord(float3 indexCoord, float4 indirectionCoordData)
{
	
	indexCoord.xy = floor( abs( indirectionCoordData.w ) ) * 
		ASMOneOverDepthAtlasSize * indexCoord.xy + indirectionCoordData.xy;

	//Index coordinate z value is being subtracted here because the application is using reversed depth buffer
    indexCoord.z = indexCoord.z - indirectionCoordData.z;
    return indexCoord;
}

float3 GetIndirectionTextureCoord(float4x4 indexTexMat, float3 worldPos)
{
	return mul(indexTexMat, float4(worldPos, 1.0)).xyz;
}

bool GetClampedDepthDifference(ASMFrustumDesc frustumDesc, out(float) depthDiff)
{
	const int DEM_LOD = gs_ASMMaxRefinement;
	
	float4 indirectionCoordData = SampleFrustumIndirectionTexture(frustumDesc, float(DEM_LOD));

	if(indirectionCoordData.w > 0)
	{
		float3 asmTileCoord = GetASMTileCoord(frustumDesc.mIndexCoord, indirectionCoordData);
		float demDepth = SampleLvlTex2D(DEMTexture, clampMiplessLinearSampler, asmTileCoord.xy, 0).r;
		depthDiff = saturate( demDepth  - asmTileCoord.z );
		return true;
	}
	return false;
}

float SampleASMShadowMap(float mip, float kernelSize, ASMFrustumDesc frustumDesc, inout(float) fadeInFactor)
{
	float4 IndexT = SampleFrustumIndirectionTexture(frustumDesc, mip);
	float shadowFactor = 0.0;

	float2 newKernelSize = float2( kernelSize / gs_ASMDepthAtlasTextureWidth, 
		kernelSize / gs_ASMDepthAtlasTextureHeight );
	if(IndexT.w != 0)
	{
		fadeInFactor = GetASMFadeInConstant(IndexT.w);
		float3 depthTileCoord = GetASMTileCoord(frustumDesc.mIndexCoord, IndexT);
		shadowFactor = PCF(depthTileCoord, newKernelSize);
	}

	return shadowFactor;
}

float GetASMShadowFactor(float mip, float kernelSize, ASMFrustumDesc frustumDesc, ASMFrustumDesc preRenderFrustumDesc)
{
	float shadowFactor = 1.0;
	float fadeInFactor = 1.0;

	if(mMiscBool.x == 1.0)
	{
		float lodClamp = SampleLvlTex2D(PrerenderLodClampTexture, 
			clampBorderNearSampler, preRenderFrustumDesc.mIndexCoord.xy, 0).r * gs_ASMMaxRefinement;
		
		shadowFactor = SampleASMShadowMap(max(mip, lodClamp), kernelSize, preRenderFrustumDesc, fadeInFactor);
	}


	if(fadeInFactor > 0)
	{
		float otherShadowFactor = 0.0;
		float otherFadeInFactor = 1.0;

		otherShadowFactor = SampleASMShadowMap(mip, kernelSize, 
			frustumDesc, otherFadeInFactor);


		shadowFactor = lerp(shadowFactor, otherShadowFactor, fadeInFactor);
	}
	return shadowFactor;
}

float GetBlockerDistance(ASMFrustumDesc frustumDesc, float3 worldPos, 
	float4x4 indexTexMat, float3 blockerSearchDir, float mip)
{
	int absMip = int(mip);
	float3 indexCoord = GetIndirectionTextureCoord(indexTexMat, worldPos);
	uint index = frustumDesc.mStartingMip + absMip;
	float4 indirectionData = f4(0);
	BeginNonUniformResourceIndex(index, 10);
		indirectionData = SampleLvlTex2D(IndexTexture[index], clampBorderNearSampler,	float2(indexCoord.xy), 0);
	EndNonUniformResourceIndex();

	float3 tileCoord = GetASMTileCoord(indexCoord, indirectionData);

	float2 tileCoordBoundsMin = floor( tileCoord.xy * ASMDepthAtlasSizeOverDepthTileSize - ASMHalfOverDepthTileSize ) * ASMDEMTileSizeOverDEMAtlasSize + ASMDEMTileCoord;
	float2 tileCoordBoundsMax = tileCoordBoundsMin + ASMDEMTileSize;

	float num = 0;
	float sum = 0;

	if(indirectionData.w != 0)
	{
		float3 sampleCoord = tileCoord;
		sampleCoord.z += 0.5 * blockerSearchDir.z * 0.1;

		LOOP
		for(int i = 0; i < 9; ++i)
		{
			float demValue = SampleLvlTex2D(DEMTexture, clampMiplessLinearSampler, 
				clamp(sampleCoord.xy, tileCoordBoundsMin, tileCoordBoundsMax), 0).r;

			FLATTEN if(demValue >= sampleCoord.z)
			{
				sum += demValue;
				num += 1.0;
			}
			sampleCoord += blockerSearchDir * 0.1;
		}
	
	}
	float blockerDepth = num > 0 ? sum * rcp(num) : 1.0;
	return saturate(blockerDepth - tileCoord.z) * gs_ASMTileFarPlane;
}

float SampleShadowFactor(float3 worldPos)
{
	float shadowFactor = 0.0;
	// float fadeInFactor = 0.0;
	float demLOD = gs_ASMMaxRefinement;

	ASMFrustumDesc frustumDesc;
	ASMFrustumDesc preRenderFrustumDesc;

	int swap = int(mMiscBool.z); 
	frustumDesc.mStartingMip = swap * 5;
	preRenderFrustumDesc.mStartingMip = (1 - swap) * 5;

	float blockerDistance = 0.0;
	float preRenderBlockerDistance = 0.0;

	if(mMiscBool.y == 1.0)
	{
		blockerDistance = GetBlockerDistance(frustumDesc, worldPos, 
			mIndexTexMat, mSearchVector.xyz, demLOD);

		preRenderBlockerDistance = GetBlockerDistance(preRenderFrustumDesc, worldPos, 
			mPrerenderIndexTexMat, mPrerenderSearchVector.xyz, demLOD);
	}
	
	frustumDesc.mIndexCoord = GetIndirectionTextureCoord(
		mIndexTexMat, worldPos + blockerDistance * mWarpVector.xyz);
	
	preRenderFrustumDesc.mIndexCoord = GetIndirectionTextureCoord(
		mPrerenderIndexTexMat, worldPos + preRenderBlockerDistance * mPrerenderWarpVector.xyz);

	float depthDiff = 0.0;
	if(GetClampedDepthDifference(frustumDesc, depthDiff))
	{
		float penumbraSizeFactor = saturate(mPenumbraSize * depthDiff - 0.05);
		float kernelSize = saturate(depthDiff * 10.0 + 0.5);

		float lod = penumbraSizeFactor * gs_ASMMaxRefinement;
		float mip = floor(lod);
		
			
		
		shadowFactor = GetASMShadowFactor(mip, kernelSize, 
			frustumDesc, preRenderFrustumDesc);

		if(penumbraSizeFactor > 0.0 && penumbraSizeFactor < 1.0)
		{
			float upperShadowFactor = GetASMShadowFactor(mip + 1, kernelSize, 
				frustumDesc, preRenderFrustumDesc);

			shadowFactor = lerp(shadowFactor, upperShadowFactor, lod - mip);
		}

	}
	return 1.0 - shadowFactor;
}

struct DerivativesOutput
{
	float3 db_dx;
	float3 db_dy;
};

// 2D interpolation results for texture gradient values
struct GradientInterpolationResults
{
	float2 interp;
	float2 dx;
	float2 dy;
};

// Barycentric coordinates and gradients, struct needed to interpolate values.
struct BarycentricDeriv
{
	float3 m_lambda;
	float3 m_ddx;
	float3 m_ddy;
};

// Calculates the local (barycentric coordinates) position of a ray hitting a triangle (Muller-Trumbore algorithm)
// Parameters: p0,p1,p2 -> World space coordinates of triangle
// o -> Origin of ray in world space (Mainly view camera here)
// d-> Unit vector direction of ray from origin
float3 rayTriangleIntersection(float3 p0, float3 p1, float3 p2, float3 o, float3 d)
{
	float3 v0v1 = p1-p0;
	float3 v0v2 = p2-p0;
	float3 pvec = cross(d,v0v2);
	float det = dot(v0v1,pvec);
	float invDet = 1/det;
	float3 tvec = o - p0;
	float u = dot(tvec,pvec) * invDet;
	float3 qvec = cross(tvec,v0v1);
	float v = dot(d,qvec) *invDet;
	float w = 1.0f - v - u;
	return float3(w,u,v);
}

// Computes the partial derivatives of a triangle from the projected screen space vertices
BarycentricDeriv CalcFullBary(float4 pt0, float4 pt1, float4 pt2, float2 pixelNdc, float2 winSize)
{
	BarycentricDeriv ret ;
	float3 invW =  1.0f / float3(pt0.w, pt1.w, pt2.w);
	//Project points on screen to calculate post projection positions in 2D
	float2 ndc0 = pt0.xy * invW.x;
	float2 ndc1 = pt1.xy * invW.y;
	float2 ndc2 = pt2.xy * invW.z;

	// Computing partial derivatives and prospective correct attribute interpolation with barycentric coordinates
	// Equation for calculation taken from Appendix A of DAIS paper:
	// https://cg.ivd.kit.edu/publications/2015/dais/DAIS.pdf

	// Calculating inverse of determinant(rcp of area of triangle).
	float invDet = rcp(determinant(float2x2(ndc2 - ndc1, ndc0 - ndc1)));

	//determining the partial derivatives
	// ddx[i] = (y[i+1] - y[i-1])/Determinant
	ret.m_ddx = float3(ndc1.y - ndc2.y, ndc2.y - ndc0.y, ndc0.y - ndc1.y) * invDet * invW;
	ret.m_ddy = float3(ndc2.x - ndc1.x, ndc0.x - ndc2.x, ndc1.x - ndc0.x) * invDet * invW;
	// sum of partial derivatives.
	float ddxSum = dot(ret.m_ddx, float3(1,1,1));
	float ddySum = dot(ret.m_ddy, float3(1,1,1));
	
	// Delta vector from pixel's screen position to vertex 0 of the triangle.
	float2 deltaVec = pixelNdc - ndc0;

	// Calculating interpolated W at point.
	float interpInvW = invW.x + deltaVec.x*ddxSum + deltaVec.y*ddySum;
	float interpW = rcp(interpInvW);
	// The barycentric co-ordinate (m_lambda) is determined by perspective-correct interpolation. 
	// Equation taken from DAIS paper.
	ret.m_lambda.x = interpW * (invW[0] + deltaVec.x*ret.m_ddx.x + deltaVec.y*ret.m_ddy.x);
	ret.m_lambda.y = interpW * (0.0f    + deltaVec.x*ret.m_ddx.y + deltaVec.y*ret.m_ddy.y);
	ret.m_lambda.z = interpW * (0.0f    + deltaVec.x*ret.m_ddx.z + deltaVec.y*ret.m_ddy.z);

	//Scaling from NDC to pixel units
	ret.m_ddx *= (2.0f/winSize.x);
	ret.m_ddy *= (2.0f/winSize.y);
	ddxSum    *= (2.0f/winSize.x);
	ddySum    *= (2.0f/winSize.y);

	// This part fixes the derivatives error happening for the projected triangles.
	// Instead of calculating the derivatives constantly across the 2D triangle we use a projected version
	// of the gradients, this is more accurate and closely matches GPU raster behavior.
	// Final gradient equation: ddx = (((lambda/w) + ddx) / (w+|ddx|)) - lambda

	// Calculating interpW at partial derivatives position sum.
	float interpW_ddx = 1.0f / (interpInvW + ddxSum);
	float interpW_ddy = 1.0f / (interpInvW + ddySum);

	// Calculating perspective projected derivatives.
	ret.m_ddx = interpW_ddx*(ret.m_lambda*interpInvW + ret.m_ddx) - ret.m_lambda;
	ret.m_ddy = interpW_ddy*(ret.m_lambda*interpInvW + ret.m_ddy) - ret.m_lambda;  

	return ret;
}

// Helper functions to interpolate vertex attributes using derivatives.

// Interpolate a float3 vector.
float InterpolateWithDeriv(BarycentricDeriv deriv, float3 v)
{
	return dot(v,deriv.m_lambda);
}
// Interpolate single values over triangle vertices.
float InterpolateWithDeriv(BarycentricDeriv deriv, float v0, float v1, float v2)
{
	float3 mergedV = float3(v0, v1, v2);
	return InterpolateWithDeriv(deriv,mergedV);
}

// Interpolate a float3 attribute for each vertex of the triangle.
// Attribute parameters: a 3x3 matrix (Row denotes attributes per vertex).
float3 InterpolateWithDeriv(BarycentricDeriv deriv,f3x3 attributes)
{
	float3 attr0 = getCol0(attributes);
	float3 attr1 = getCol1(attributes);
	float3 attr2 = getCol2(attributes);
	return float3(dot(attr0,deriv.m_lambda),dot(attr1,deriv.m_lambda),dot(attr2,deriv.m_lambda));
}

// Interpolate 2D attributes using the partial derivatives and generates dx and dy for texture sampling.
// Attribute paramters: a 3x2 matrix of float2 attributes (Column denotes attribuets per vertex).
GradientInterpolationResults Interpolate2DWithDeriv(BarycentricDeriv deriv,f3x2 attributes)
{
	float3 attr0 = getRow0(attributes);
	float3 attr1 = getRow1(attributes);
	
	GradientInterpolationResults result;
	// independently interpolate x and y attributes.
	result.interp.x = InterpolateWithDeriv(deriv,attr0);
	result.interp.y = InterpolateWithDeriv(deriv,attr1);

	// Calculate attributes' dx and dy (for texture sampling).
	result.dx.x = dot(attr0,deriv.m_ddx);
	result.dx.y = dot(attr1,deriv.m_ddx);
	result.dy.x = dot(attr0,deriv.m_ddy);
	result.dy.y = dot(attr1,deriv.m_ddy);
	return result;
}

// Calculate ray differentials for a point in world-space
// Parameters: pt0,pt1,pt2 -> world space coordinates of the triangle currently visible on the pixel
// position -> world-space calculated position of the current pixel by reconstructing Z value
// positionDX,positionDY -> world-space positions a pixel footprint right and down of the calculated position w.r.t traingle
BarycentricDeriv CalcRayBary(float3 pt0, float3 pt1, float3 pt2,float3 position,float3 positionDX,float3 positionDY,
								float3 camPos)
{
	BarycentricDeriv ret ;

	// Calculating unit vector directions of all 3 rays
	float3 curRay = position - camPos;
	float3 rayDX = positionDX - camPos;
	float3 rayDY = positionDY - camPos;
	// Calculating barycentric coordinates of each rays hitting the triangle
	float3 H = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(curRay));
	float3 Hx = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(rayDX));
	float3 Hy = rayTriangleIntersection(pt0,pt1,pt2,camPos,normalize(rayDY));
	ret.m_lambda = H;
	// Ray coordinates differential
	ret.m_ddx = Hx-H;
	ret.m_ddy = Hy-H;
	return ret;
}

float depthLinearization(float depth, float near, float far)
{
	return (2.0 * near) / (far + near - depth * (far - near));
}

// Pixel shader
PsOut PS_MAIN( PsIn In, SV_SampleIndex(uint) i )
{
	INIT_MAIN;
	PsOut Out;
#if SAMPLE_COUNT > 1
	float4 visRaw = LoadTex2DMS(vbPassTexture, clampMiplessLinearSampler, uint2(In.Position.xy), i);
#else
	float4 visRaw = LoadTex2D(vbPassTexture, clampMiplessLinearSampler, uint2(In.Position.xy), 0);
#endif

	uint alphaBitDrawIDTriID = packUnorm4x8(visRaw);

	// Early exit if this pixel doesn't contain triangle data
	if (alphaBitDrawIDTriID == ~0u)
	{
		discard;
	}

	//extract packed data
	uint drawID = (alphaBitDrawIDTriID >> 23) & 0x000000FF;
	uint triangleID = (alphaBitDrawIDTriID & 0x007FFFFF);
	uint alpha1_opaque0 = (alphaBitDrawIDTriID >> 31);

	//this is the start vertex of the current draw batch
	uint startIndexOffset = INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET + 2;

	uint startIndex = (alpha1_opaque0 == 0) ?
		indirectDrawArgs[0][drawID * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + startIndexOffset] :
		indirectDrawArgs[1][drawID * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + startIndexOffset];

	uint triIdx0 = (triangleID * 3 + 0) + startIndex;
	uint triIdx1 = (triangleID * 3 + 1) + startIndex;
	uint triIdx2 = (triangleID * 3 + 2) + startIndex;

	uint index0 = LoadByte(filteredIndexBuffer, triIdx0 << 2);
	uint index1 = LoadByte(filteredIndexBuffer, triIdx1 << 2);
	uint index2 = LoadByte(filteredIndexBuffer, triIdx2 << 2);

	float3 v0pos = asfloat(LoadByte4(vertexPos, index0 * 12)).xyz;
	float3 v1pos = asfloat(LoadByte4(vertexPos, index1 * 12)).xyz;
	float3 v2pos = asfloat(LoadByte4(vertexPos, index2 * 12)).xyz;

	float4 pos0 = mul(WorldViewProjMat, float4(v0pos, 1));
	float4 pos1 = mul(WorldViewProjMat, float4(v1pos, 1));
	float4 pos2 = mul(WorldViewProjMat, float4(v2pos, 1));

	float4 wPos0 = mul(InvViewProject,pos0);
	float4 wPos1 = mul(InvViewProject,pos1);
	float4 wPos2 = mul(InvViewProject,pos2);

	float3 oneOverW = 1.0 / float3(pos0.w, pos1.w, pos2.w);

	pos0 *= oneOverW.x;
	pos1 *= oneOverW.y;
	pos2 *= oneOverW.z;

	float2 posSrc[3] = {pos0.xy, pos1.xy, pos2.xy};

	//Compute partial derivatives. This is necessary to interpolate triangle attributes per pixel.
	BarycentricDeriv derivativesOut = CalcFullBary(pos0,pos1,pos2,In.ScreenPos,WindowDimension.xy);
	
	//interpoalte the 1/w (oneOverW) for all 3 vertices of the triangle
	//using the barycentric coordinates and the delta vector
	float interpolatedW = 1.0f / dot(oneOverW,derivativesOut.m_lambda);

	//reconstruct the z value at this screen point
	float zVal = interpolatedW * getElem(Project, 2, 2) + getElem(Project, 3, 2);

	// Calculate the world position coordinates:
	// First the projected coordinates at this point are calculated using Screen Position and the Z value
	// Then by multiplying the perspective projected coordinates by the inverse view projection matrix, it produces world coord
	float3 WorldPos = mul(InvViewProject, float4(In.ScreenPos * interpolatedW, zVal, interpolatedW)).xyz;

	//Texture coord interpolation
#if defined(USE_RAY_DIFFERENTIALS)
	// We don't apply perspective correction to texture coordinates in case of ray differentials
	f3x2 texCoords = make_f3x2_cols(
			unpack2Floats(LoadByte(vertexTexCoord, index0 << 2)) ,
			unpack2Floats(LoadByte(vertexTexCoord, index1 << 2)) ,
			unpack2Floats(LoadByte(vertexTexCoord, index2 << 2)) 
	);
	float3 positionDX = mul(InvViewProject, float4((In.ScreenPos+mTwoOverRes.x/2) * interpolatedW, zVal, interpolatedW)).xyz;
	float3 positionDY = mul(InvViewProject, float4((In.ScreenPos+mTwoOverRes.y/2) * interpolatedW, zVal, interpolatedW)).xyz;

	derivativesOut = CalcRayBary(wPos0.xyz,wPos1.xyz,wPos2.xyz,WorldPos,positionDX,positionDY,
												mCameraPos.xyz);
#else
	// Apply perspective correction to texture coordinates
	f3x2 texCoords = make_f3x2_cols(
			unpack2Floats(LoadByte(vertexTexCoord, index0 << 2)) * one_over_w[0] ,
			unpack2Floats(LoadByte(vertexTexCoord, index1 << 2)) * one_over_w[1],
			unpack2Floats(LoadByte(vertexTexCoord, index2 << 2)) * one_over_w[2]
	);
#endif
	// Interpolate texture coordinates and calculate the gradients for 
	// texture sampling with mipmapping support
	GradientInterpolationResults results = Interpolate2DWithDeriv(derivativesOut,texCoords);
	

	float farValue = mFarNearDiff + mNear;
	float linearZ = depthLinearization(zVal / interpolatedW, mNear, farValue);

	float mip = pow(pow(linearZ, 0.9f) * 5.0f, 1.5f);

	
	float2 texCoordDX = results.dx * mip;
	float2 texCoordDY = results.dy * mip;
	float2 texCoord = results.interp ;

#if !defined(USE_RAY_DIFFERENTIALS)
	//Perspective correct in case of screen differentials
	texCoord *= w;
	texCoordDX *= w;
	texCoordDY *= w;
#endif

	/////////////LOAD///////////////////////////////
	// TANGENT INTERPOLATION
	// Apply perspective division to tangents

#if defined(USE_RAY_DIFFERENTIALS)
	// We don't apply perspective correction in case of ray differentials
	float3x3 tangents = make_f3x3_rows(
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index0 << 2))) ,
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index1 << 2))),
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index2 << 2))) 
	);
#else
	float3x3 tangents = make_f3x3_rows(
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index0 << 2))) * one_over_w[0],
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index1 << 2))) * one_over_w[1],
			decodeDir(unpackUnorm2x16(LoadByte(vertexTangent, index2 << 2))) * one_over_w[2]
	);

#endif

	float3 tangent = normalize(InterpolateWithDeriv(derivativesOut,tangents));;

	// BaseMaterialBuffer returns constant offset values
	// The following value defines the maximum amount of indirect draw calls that will be 
	// drawn at once. This value depends on the number of submeshes or individual objects 
	// in the scene. Changing a scene will require to change this value accordingly.
	// #define MAX_DRAWS_INDIRECT 300 
	//
	// These values are offsets used to point to the material data depending on the 
	// type of geometry and on the culling view
	// #define MATERIAL_BASE_ALPHA0 0
	// #define MATERIAL_BASE_NOALPHA0 MAX_DRAWS_INDIRECT
	// #define MATERIAL_BASE_ALPHA1 (MAX_DRAWS_INDIRECT*2)
	// #define MATERIAL_BASE_NOALPHA1 (MAX_DRAWS_INDIRECT*3)

	uint materialBaseSlot = BaseMaterialBuffer(alpha1_opaque0 == 1, VIEW_CAMERA);

	// potential results for materialBaseSlot + drawID are
	// 0 - 299 - shadow alpha
	// 300 - 599 - shadow no alpha
	// 600 - 899 - camera alpha
	uint materialID = LoadByte(indirectMaterialBuffer, (materialBaseSlot + drawID) << 2);

	//Calculate pixel color using interpolated attributes
	//reconstruct normal map Z from X and Y

	float4 normalMapRG   = f4(0);
	float4 diffuseColor  = f4(0);
	float4 specularColor = f4(0);
	BeginNonUniformResourceIndex(materialID, MAX_TEXTURE_UNITS);
		normalMapRG   = SampleGradTex2D(normalMaps[materialID],   textureSampler, texCoord, texCoordDX, texCoordDY);
		diffuseColor  = SampleGradTex2D(diffuseMaps[materialID],  textureSampler, texCoord, texCoordDX, texCoordDY);
		specularColor = SampleGradTex2D(specularMaps[materialID], textureSampler, texCoord, texCoordDX, texCoordDY);
	EndNonUniformResourceIndex();

	float3 reconstructedNormalMap;
	reconstructedNormalMap.xy = normalMapRG.ga * 2.f - 1.f;
	reconstructedNormalMap.z = sqrt(1.f - dot(reconstructedNormalMap.xy, reconstructedNormalMap.xy));

	//Normal interpolation
#if defined(USE_RAY_DIFFERENTIALS)
	// We don't apply perspective correction in case of ray differentials
	float3x3 normals = make_f3x3_rows(
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index0 << 2))) ,
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index1 << 2))) ,
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index2 << 2))) 
	);
#else
	float3x3 normals = make_f3x3_rows(
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index0 << 2))) * one_over_w[0],
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index1 << 2))) * one_over_w[1],
		decodeDir(unpackUnorm2x16(LoadByte(vertexNormal, index2 << 2))) * one_over_w[2]
	);
#endif
	float3 normal = normalize(InterpolateWithDeriv(derivativesOut, normals));;
	
	float3 binormal = normalize(cross(tangent, normal));

	normal = reconstructedNormalMap.x * tangent + 
		reconstructedNormalMap.y * binormal + reconstructedNormalMap.z * normal;

	float shadowFactor = 1.0f;

	if(ShadowType == SHADOW_TYPE_ASM)
	{
		shadowFactor = SampleShadowFactor(WorldPos);
	}
	else if(ShadowType == SHADOW_TYPE_ESM)
	{
		shadowFactor = calcESMShadowFactor(WorldPos);
	}
	else if(ShadowType == SHADOW_TYPE_MESH_BAKED_SDF)
	{
		shadowFactor = LoadTex2D(SDFShadowTexture, clampMiplessNearSampler, uint2(In.Position.xy), 0).r;
	}

	float Roughness = clamp(specularColor.a, 0.05f, 0.99f);
	float Metallic = specularColor.b;

	float3 camPos = mCameraPos.xyz;

	float3 ViewVec = normalize(mCameraPos.xyz - WorldPos.xyz);

	bool isTwoSided = (alpha1_opaque0 == 1) && (meshConstantsBuffer[materialID].twoSided == 1);
	bool isBackFace = false;

	if(isTwoSided && dot(normal, ViewVec) < 0.0)
	{
		//flip normal
		normal = -normal;
		isBackFace = true;
	}

	float3 lightDir = -mLightDir;

	float3 HalfVec = normalize(ViewVec - lightDir);
	float3 ReflectVec = reflect(-ViewVec, normal);
	float NoV = saturate(dot(normal, ViewVec));

	float NoL = dot(normal, -lightDir);	

	// Deal with two faced materials
	NoL = (isTwoSided ? abs(NoL) : saturate(NoL));

	float3 shadedColor;

	// float3 F0 = specularColor.xyz;
	float3 DiffuseColor = diffuseColor.xyz;
	
	float fLightingMode = 1.f;

	shadedColor = calculateIllumination(
		    normal,
		    ViewVec,
			HalfVec,
			ReflectVec,
			NoL,
			NoV,
			camPos.xyz,
			lightDir.xyz,
			WorldPos,
			DiffuseColor,
			DiffuseColor,
			Roughness,
			Metallic,
			isBackFace,
			fLightingMode,
			shadowFactor);

	shadedColor = shadedColor * lightColor.rgb * lightColor.a * NoL;

	float ambientIntencity = 0.05f;
    float3 ambient = diffuseColor.xyz * ambientIntencity;

	shadedColor += ambient;
    
    Out.FinalColor = float4(shadedColor.xyz, 1.0f);
	RETURN(Out);
}
