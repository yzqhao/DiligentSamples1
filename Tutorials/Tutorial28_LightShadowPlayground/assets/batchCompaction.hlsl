
#include "Shader_Defs.h.hlsl"
#include "cull_resources.h.hlsl"

inline void StoreByte(RWByteAddressBuffer buff, int address, int val)
{ 
	buff.Store(address, val); 
}

[numthreads(CLEAR_THREAD_COUNT, 1, 1)]
void main(uint3 threadID : SV_DispatchThreadID) 
{
    if (threadID.x >= MAX_DRAWS_INDIRECT - 1)
	{
		return;
	}

	uint numIndices[NUM_CULLING_VIEWPORTS];
	uint sum = 0;

	[unroll(NUM_CULLING_VIEWPORTS)]
	for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
	{
		numIndices[i] = uncompactedDrawArgs[i][threadID.x].mNumIndices;
		sum += numIndices[i];
	}

	if (sum == 0)
	{
		return;
	}
	uint slot = 0;


	[unroll(NUM_CULLING_VIEWPORTS)]
	for (uint j = 0; j < NUM_CULLING_VIEWPORTS; ++j)
	{
		if (numIndices[j] > 0)
		{
			uint matID = uncompactedDrawArgs[j][threadID.x].mMaterialID;
			bool hasAlpha = (materialProps[matID] == 1);
			uint baseMatSlot = BaseMaterialBuffer(hasAlpha, j);

			uint indexCountOffset = INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET + 0;
			uint startIndexOffset = INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET + 2;
			uint startInstanceOffset = INDIRECT_DRAW_ARGUMENTS_STRUCT_OFFSET + 4;
			uint startIndex = uncompactedDrawArgs[j][threadID.x].mStartIndex;

			if (hasAlpha)
			{
				InterlockedAdd(indirectDrawArgsBufferAlpha[j][DRAW_COUNTER_SLOT_POS], 1, slot);
				indirectDrawArgsBufferAlpha[j][slot * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + indexCountOffset] = numIndices[j];
				indirectDrawArgsBufferAlpha[j][slot * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + startIndexOffset] = startIndex;
			}
			else
			{
				InterlockedAdd(indirectDrawArgsBufferNoAlpha[j][DRAW_COUNTER_SLOT_POS], 1, slot);
				indirectDrawArgsBufferNoAlpha[j][slot * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + indexCountOffset] = numIndices[j];
				indirectDrawArgsBufferNoAlpha[j][slot * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS + startIndexOffset] = startIndex;
			}
			indirectMaterialBuffer[baseMatSlot + slot] = matID;
		}
	}
}
