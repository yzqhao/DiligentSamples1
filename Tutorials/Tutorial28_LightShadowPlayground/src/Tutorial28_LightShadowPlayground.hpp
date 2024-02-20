
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "Geometry.h"
#include "ShaderDef.h"
#include "FirstPersonCamera.hpp"
#include "RingBuffer.h"

#include <map>

// Define different geometry sets (opaque and alpha tested geometry)
const uint32_t gNumGeomSets        = 2;
const uint32_t GEOMSET_OPAQUE      = 0;
const uint32_t GEOMSET_ALPHATESTED = 1;

namespace Diligent
{

enum ShadowType
{
	SHADOW_TYPE_ESM,    //Exponential Shadow Map
	SHADOW_TYPE_ASM, //Adaptive Shadow Map, has Parallax Corrected Cache algorithm that approximate moving sun's shadow
	SHADOW_TYPE_MESH_BAKED_SDF, // Signed Distance field shadow for mesh using generated baked data
	SHADOW_TYPE_COUNT
};

struct VisibilityBufferConstants
{
    float4x4        mWorldViewProjMat[NUM_CULLING_VIEWPORTS];
    CullingViewPort mCullingViewports[NUM_CULLING_VIEWPORTS];
};

struct LightUniformBlock
{
    float4x4 mLightViewProj;
    float4   mLightPosition;
    float4   mLightColor = {1, 0, 0, 1};
    float4   mLightUpVec;
    float4   mTanLightAngleAndThresholdValue;
    float3   mLightDir;
};

struct CameraUniform
{
    float4x4 mView;
    float4x4 mProject;
    float4x4 mViewProject;
    float4x4 mInvView;
    float4x4 mInvProj;
    float4x4 mInvViewProject;
    float4   mCameraPos;
    float    mNear;
    float    mFarNearDiff;
    float    mFarNear;
    float    paddingForAlignment0;
    float2   mTwoOverRes;
    float    _pad1;
    float    _pad2;
    float2   mWindowSize;
    float    _pad3;
    float    _pad4;
    float4   mDeviceZToWorldZ;
};

struct MeshInfoUniformBlock
{
    float4x4        mWorldViewProjMat;
    float4x4        mWorldMat;
    CullingViewPort cullingViewports[2];
};

struct FillBatchData
{
	SmallBatchData data[BATCH_COUNT];
};

struct MeshInfoStruct
{
    float4 mColor;
    float3 mTranslation;
    float3 mOffsetTranslation;
    float3 mScale;
    float4x4 mTranslationMat;
    float4x4 mScaleMat;
} gMeshInfoData;

class Tutorial28_LightShadowPlayground final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial28 LightShadowPlayground"; }

private:
    void CreatePipelineState();
    void LoadScene();
    void LoadBuffers();
    void LoadTexture();

    void TriangleFilteringPass();
    void filterTriangles(FilterBatchChunk* batchChunk, uint64_t batchidx);

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;
    std::map<std::string, RefCntAutoPtr<IBufferView>>            m_BufferViews;

	RefCntAutoPtr<IBuffer> mBufferFilterIndirectMaterial;
    RefCntAutoPtr<IBuffer> mBufferFilteredIndex[NUM_CULLING_VIEWPORTS];
    RefCntAutoPtr<IBuffer> mBufferUncompactedDrawArguments[NUM_CULLING_VIEWPORTS];
    RefCntAutoPtr<IBuffer> mBufferFilteredIndirectDrawArguments[gNumGeomSets][NUM_CULLING_VIEWPORTS];
    RefCntAutoPtr<IBufferView> mBufferViewFilteredIndirectDrawArguments[gNumGeomSets][NUM_CULLING_VIEWPORTS];

	RefCntAutoPtr<IBuffer> mBufferMaterialProps;
    RefCntAutoPtr<IBuffer> mBufferVertexData;
    RefCntAutoPtr<IBuffer> mBufferIndexData;
    RefCntAutoPtr<IBuffer> mBufferMeshConstants;

    ShadowType mCurrentShadowType = SHADOW_TYPE_ESM;

    static const uint32_t gSmallBatchChunkCount = std::max(1U, 512U / CLUSTER_SIZE) * 16U;
	FilterBatchChunk* pFilterBatchChunk[gSmallBatchChunkCount] = { NULL };

	GPURingBuffer* pBufferFilterBatchData = NULL;

    Scene* mScene;
    Uint32 mMeshCount;
    Uint32 mMaterialCount;

	ClusterContainer* pMeshes = nullptr;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth = 1280;
    uint32_t mHeight = 720;

    // uniform data
	LightUniformBlock mLightUniformBlock;
	CameraUniform mCameraUniform;
	VisibilityBufferConstants mVisibilityBufferCB;
	MeshInfoUniformBlock mMeshInfoUniformData;
	std::vector<FillBatchData> mBatchDatas;
};

} // namespace Diligent
