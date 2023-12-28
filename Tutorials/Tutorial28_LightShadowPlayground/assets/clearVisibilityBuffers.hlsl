#include "Shader_Defs.h.hlsl"
#include "cull_resources.h.hlsl"


[numthreads(CLEAR_THREAD_COUNT, 1, 1)]
void main(int3 threadID : SV_DispatchThreadID) 
{
    if (threadID.x >= MAX_DRAWS_INDIRECT - 1)
	{        
		return;
	}
	
	[unroll(NUM_CULLING_VIEWPORTS)]
	for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
	{
		uncompactedDrawArgsRW[i][threadID.x].mNumIndices = 0;
	}

    if (threadID.x == 0)
    {
		[unroll(NUM_CULLING_VIEWPORTS)]
		for (uint i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
		{
			indirectDrawArgsBufferAlpha[i][DRAW_COUNTER_SLOT_POS] = 0;
			indirectDrawArgsBufferNoAlpha[i][DRAW_COUNTER_SLOT_POS] = 0;
		}
    }
}