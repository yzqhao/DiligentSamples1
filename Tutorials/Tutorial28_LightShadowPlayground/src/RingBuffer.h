
#pragma once

#include "AdvancedMath.hpp"

#include <cassert>

namespace Diligent
{
/************************************************************************/
/* RING BUFFER MANAGEMENT											  */
/************************************************************************/
typedef struct GPURingBuffer
{
    IBuffer* pBuffer;

    uint32_t mBufferAlignment;
    uint64_t mMaxBufferSize;
    uint64_t mCurrentBufferOffset;
} GPURingBuffer;


typedef struct GPURingBufferOffset
{
    IBuffer* pBuffer;
    uint64_t        mOffset;
} GPURingBufferOffset;
/*
static inline void addGPURingBuffer(const BufferDesc* pBufferDesc, GPURingBuffer** ppRingBuffer)
{
	GPURingBuffer* pRingBuffer = (GPURingBuffer*)malloc(sizeof(GPURingBuffer));
	pRingBuffer->mMaxBufferSize = pBufferDesc->mSize;
	pRingBuffer->mBufferAlignment = sizeof(float[4]);
	BufferLoadDesc loadDesc = {};
	loadDesc.mDesc = *pBufferDesc;
	loadDesc.ppBuffer = &pRingBuffer->pBuffer;
	addResource(&loadDesc, NULL);

	*ppRingBuffer = pRingBuffer;
}
*/
static inline void addUniformGPURingBuffer(uint32_t requiredUniformBufferSize, GPURingBuffer** ppRingBuffer)
{
    GPURingBuffer* pRingBuffer = (GPURingBuffer*)malloc(sizeof(GPURingBuffer));

    //const uint32_t uniformBufferAlignment = D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT;
    const uint32_t maxUniformBufferSize   = requiredUniformBufferSize;
    //pRingBuffer->mBufferAlignment         = uniformBufferAlignment;
    pRingBuffer->mMaxBufferSize           = maxUniformBufferSize;

    *ppRingBuffer = pRingBuffer;
}

static inline void removeGPURingBuffer(GPURingBuffer* pRingBuffer)
{
    //removeResource(pRingBuffer->pBuffer);
    //tf_free(pRingBuffer);
}

static inline void resetGPURingBuffer(GPURingBuffer* pRingBuffer)
{
    pRingBuffer->mCurrentBufferOffset = 0;
}

static inline uint32_t round_up(uint32_t value, uint32_t multiple) { return ((value + multiple - 1) / multiple) * multiple; }

static inline GPURingBufferOffset getGPURingBufferOffset(GPURingBuffer* pRingBuffer, uint32_t memoryRequirement, uint32_t alignment = 0)
{
    uint32_t alignedSize = round_up(memoryRequirement, alignment ? alignment : pRingBuffer->mBufferAlignment);

    if (alignedSize > pRingBuffer->mMaxBufferSize)
    {
        assert(false && "Ring Buffer too small for memory requirement");
        return {NULL, 0};
    }

    if (pRingBuffer->mCurrentBufferOffset + alignedSize >= pRingBuffer->mMaxBufferSize)
    {
        pRingBuffer->mCurrentBufferOffset = 0;
    }

    GPURingBufferOffset ret = {pRingBuffer->pBuffer, pRingBuffer->mCurrentBufferOffset};
    pRingBuffer->mCurrentBufferOffset += alignedSize;

    return ret;
}
} // namespace Diligent
