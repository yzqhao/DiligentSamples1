
// Confetti changes
// Culling triangles that intersect the near plane
// Small primitive culling now supports MSAA
// Multi-viewport culling

#include "Shader_Defs.h.hlsl"
#include "cull_resources.h.hlsl"

groupshared uint workGroupOutputSlot[NUM_CULLING_VIEWPORTS];
groupshared uint workGroupIndexCount[NUM_CULLING_VIEWPORTS];

#define ENABLE_CULL_INDEX				1
#define ENABLE_CULL_BACKFACE			1
#define ENABLE_CULL_FRUSTUM				1
#define ENABLE_CULL_SMALL_PRIMITIVES	1
#define ENABLE_GUARD_BAND				0

#define GreaterThan(A, B)      ((A) > (B))
#define LessThan(A, B)         ((A) < (B))

bool2 And(const bool2 a, const bool2 b)
{ return a && b; }
inline int LoadByte(ByteAddressBuffer buff, int address)
{ return buff.Load(address);}
inline int4 LoadByte4(ByteAddressBuffer buff, int address)
{ return buff.Load4(address);}
inline int LoadByte(RWByteAddressBuffer buff, int address)
{ return buff.Load(address);}
inline int4 LoadByte4(RWByteAddressBuffer buff, int address)
{ return buff.Load4(address);}
inline void StoreByte(RWByteAddressBuffer buff, int address, int val)
{ buff.Store(address, val); }

float4 LoadVertex(uint index)
{
	return float4(asfloat(LoadByte4(vertexDataBuffer, index * 12)).xyz, 1);		
}

bool EqualF4(float4 f1, float4 f2)
{
	return f1.x == f2.x && f1.y == f2.y && f1.z == f2.z && f1.w == f2.w;
}

// Performs all the culling tests given 3 vertices
bool FilterTriangle(uint indices[3], float4 vertices[3], bool cullBackFace, float2 windowSize, uint samples)
{
#if ENABLE_CULL_INDEX
	if (indices[0] == indices[1]
		|| indices[1] == indices[2]
		|| indices[0] == indices[2])
	{
		return true;
	}
#endif
#if ENABLE_CULL_BACKFACE
	if (cullBackFace)
	{
		// Culling in homogeneus coordinates.
		// Read: "Triangle Scan Conversion using 2D Homogeneus Coordinates"
		//       by Marc Olano, Trey Greer
		float3x3 m = float3x3(vertices[0].xyw, vertices[1].xyw, vertices[2].xyw);
		if (determinant(m) > 0)
			return true;
	}
#endif

#if ENABLE_CULL_FRUSTUM || ENABLE_CULL_SMALL_PRIMITIVES
	int verticesInFrontOfNearPlane = 0;

	for (uint i = 0; i < 3; i++)
	{
		if (vertices[i].w < 0)
		{
			++verticesInFrontOfNearPlane;

			// Flip the w so that any triangle that straddles the plane won't be projected onto
			// two sides of the screen
			vertices[i].w *= (-1.0);
		}
		// Transform vertices[i].xy into the normalized 0..1 screen space
		// this is for the following stages ...
		vertices[i].xy /= vertices[i].w * 2;
		vertices[i].xy += float2(0.5, 0.5);
	}
#endif

#if ENABLE_CULL_FRUSTUM
	if (verticesInFrontOfNearPlane == 3)
		return true;

	float minx = min(min(vertices[0].x, vertices[1].x), vertices[2].x);
	float miny = min(min(vertices[0].y, vertices[1].y), vertices[2].y);
	float maxx = max(max(vertices[0].x, vertices[1].x), vertices[2].x);
	float maxy = max(max(vertices[0].y, vertices[1].y), vertices[2].y);

	if ((maxx < 0) || (maxy < 0) || (minx > 1) || (miny > 1))
		return true;
#endif

	// not precise enough to handle more than 4 msaa samples
#if ENABLE_CULL_SMALL_PRIMITIVES
	if (verticesInFrontOfNearPlane == 0)
	{
		const uint SUBPIXEL_BITS = 8;
		const uint SUBPIXEL_MASK = 0xFF;
		const uint SUBPIXEL_SAMPLES = 1U << SUBPIXEL_BITS;

		/*
		Computing this in float-point is not precise enough.
		We switch to a 23.8 representation here which shold match the
		HW subpixel resolution.
		We use a 8-bit wide guard-band to avoid clipping. If
		a triangle is outside the guard-band, it will be ignored.
		That is, the actual viewport supported here is 31 bit, one bit is
		unused, and the guard band is 1 << 23 bit large (8388608 pixels)
		*/

		int2 minBB = int2(1 << 30, 1 << 30);
		int2 maxBB = -minBB;
#if ENABLE_GUARD_BAND			
		bool insideGuardBand = true;
#endif
		for (uint i = 0; i < 3; i++)
		{
			float2 screenSpacePositionFP = vertices[i].xy * windowSize;
#if ENABLE_GUARD_BAND			
			// Check if we should overflow after conversion
			if (screenSpacePositionFP.x < -(1 << 23) ||
				screenSpacePositionFP.x > (1 << 23) ||
				screenSpacePositionFP.y < -(1 << 23) ||
				screenSpacePositionFP.y > (1 << 23))
			{
				insideGuardBand = false;
			}
#endif
			// Scale based on distance from center to msaa sample point
			int2 screenSpacePosition = int2(screenSpacePositionFP * (SUBPIXEL_SAMPLES * samples));
			minBB = min(screenSpacePosition, minBB);
			maxBB = max(screenSpacePosition, maxBB);
		}
#if ENABLE_GUARD_BAND			
		if (insideGuardBand)
#endif
		{
			const uint SUBPIXEL_SAMPLE_CENTER = SUBPIXEL_SAMPLES / 2;
			const uint SUBPIXEL_SAMPLE_SIZE = SUBPIXEL_SAMPLES - 1;
			/* Test is:
			Is the minimum of the bounding box right or above the sample
			point and is the width less than the pixel width in samples in
			one direction.

			This will also cull very long triagles which fall between
			multiple samples.
			*/

			bool2 b1 = GreaterThan( minBB & SUBPIXEL_MASK, SUBPIXEL_SAMPLE_CENTER );
			bool2 b2 = LessThan( maxBB - ((minBB & ~SUBPIXEL_MASK) + SUBPIXEL_SAMPLE_CENTER), SUBPIXEL_SAMPLE_SIZE );
			if ((b1.x && b2.x) || (b1.y && b2.y))
			{
				return true;
			}
		}
	}
#endif

	return false;
}

// inGroupId : in(CLUSTER_SIZE, 1, 1)
// groupId : in(2048, 1, 1)
[numthreads(CLUSTER_SIZE, 1, 1)]
void main(uint3 inGroupId : SV_GroupThreadID, uint3 groupId : SV_GroupID)
{
	if (inGroupId.x == 0)
	{
		[unroll(NUM_CULLING_VIEWPORTS)]
		for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
			workGroupIndexCount[i] = 0u;
	}

	GroupMemoryBarrier();

	bool cull[NUM_CULLING_VIEWPORTS];
	uint threadOutputSlot[NUM_CULLING_VIEWPORTS];

	[unroll(NUM_CULLING_VIEWPORTS)]
	for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
	{
		threadOutputSlot[i] = 0;
		cull[i] = true;
	}

	uint batchMeshIndex = smallBatchDataBuffer[groupId.x].mMeshIndex;
	uint batchInputIndexOffset = (meshConstantsBuffer[batchMeshIndex].indexOffset + smallBatchDataBuffer[groupId.x].mIndexOffset);
	bool twoSided = (meshConstantsBuffer[batchMeshIndex].twoSided == 1);

	uint indices[3] = { 0, 0, 0 };
	if (inGroupId.x < smallBatchDataBuffer[groupId.x].mFaceCount)
	{
		indices[0] = LoadByte(indexDataBuffer, (inGroupId.x * 3 + 0 + batchInputIndexOffset) << 2);
		indices[1] = LoadByte(indexDataBuffer, (inGroupId.x * 3 + 1 + batchInputIndexOffset) << 2);
		indices[2] = LoadByte(indexDataBuffer, (inGroupId.x * 3 + 2 + batchInputIndexOffset) << 2);

		float4 vert[3] =
		{
			LoadVertex(indices[0]),
			LoadVertex(indices[1]),
			LoadVertex(indices[2])
		};

    	[unroll(NUM_CULLING_VIEWPORTS)]
		for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
		{
			float4x4 worldViewProjection = mWorldViewProjMat[i];
			float4 vertices[3] =
			{
				mul(worldViewProjection, vert[0]),
				mul(worldViewProjection, vert[1]),
				mul(worldViewProjection, vert[2])
			};

			CullingViewPort viewport = mCullingViewports[i];
			cull[i] = FilterTriangle(indices, vertices, !twoSided, viewport.mWindowSize, viewport.mSampleCount);
			if (!cull[i])
				InterlockedAdd(workGroupIndexCount[i], 3, threadOutputSlot[i]);
		}
	}

    GroupMemoryBarrier();

	uint accumBatchDrawIndex = smallBatchDataBuffer[groupId.x].mAccumDrawIndex;

	if (inGroupId.x == 0)
	{
		[unroll(NUM_CULLING_VIEWPORTS)]
		for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
		{
			InterlockedAdd(uncompactedDrawArgsRW[i][accumBatchDrawIndex].mNumIndices, workGroupIndexCount[i], workGroupOutputSlot[i]);
		}
	}

	AllMemoryBarrier();

	[unroll(NUM_CULLING_VIEWPORTS)]
	for (uint j = 0; j < NUM_CULLING_VIEWPORTS; ++j)
	{
		if (!cull[j])
		{
            uint index = workGroupOutputSlot[j];
			StoreByte(filteredIndicesBuffer[j], (index + smallBatchDataBuffer[groupId.x].mOutputIndexOffset + threadOutputSlot[j] + 0) << 2, indices[0]);
			StoreByte(filteredIndicesBuffer[j], (index + smallBatchDataBuffer[groupId.x].mOutputIndexOffset + threadOutputSlot[j] + 1) << 2, indices[1]);
			StoreByte(filteredIndicesBuffer[j], (index + smallBatchDataBuffer[groupId.x].mOutputIndexOffset + threadOutputSlot[j] + 2) << 2, indices[2]);
		}
	}

	if (inGroupId.x == 0 && groupId.x == smallBatchDataBuffer[groupId.x].mDrawBatchStart)
	{
		uint outIndexOffset = smallBatchDataBuffer[groupId.x].mOutputIndexOffset;

		[unroll(NUM_CULLING_VIEWPORTS)]
		for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
		{
			uncompactedDrawArgsRW[i][accumBatchDrawIndex].mStartIndex = outIndexOffset;
			uncompactedDrawArgsRW[i][accumBatchDrawIndex].mMaterialID = meshConstantsBuffer[batchMeshIndex].materialID;
		}
	}
}
