
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"
#include "Geometry.h"

#include <map>

// Define different geometry sets (opaque and alpha tested geometry)
const uint32_t gNumGeomSets        = 2;
const uint32_t GEOMSET_OPAQUE      = 0;
const uint32_t GEOMSET_ALPHATESTED = 1;

namespace Diligent
{

struct ObjectConstants
{
    float4x4 World;
    float4   Color;
};

struct PointLight
{
    float4 mPos;
    float4 mCol;
    float  mRadius;
    float  mIntensity;
    char   _pad[8];
};

struct DirectionalLight
{
    float4 mPos;
    float4 mCol; //alpha is the intesity
    float4 mDir;
};

const static int MaxLights = 16;

struct PassConstants
{
    float4x4 View{};
    float4x4 InvView{};
    float4x4 Proj{};
    float4x4 InvProj{};
    float4x4 ViewProj{};
    float4x4 PreViewProj{};
    float4x4 InvViewProj{};
    float4   EyePosW;          // xyz
    float4   RenderTargetSize; // xy : size, zw : InvRenderTargetSize
    float    NearZ     = 0.0f;
    float    FarZ      = 0.0f;
    float    TotalTime = 0.0f;
    float    DeltaTime = 0.0f;
    float4   ViewPortSize;
    float4   AmbientLight = {0.0f, 0.0f, 0.0f, 1.0f};

    DirectionalLight DLights[MaxLights];
    PointLight       PLights[MaxLights];
    int              amountOfDLights;
    int              amountOfPLights;
};

struct VisibilityBufferConstants
{
    float4x4        mWorldViewProjMat[NUM_CULLING_VIEWPORTS];
    CullingViewPort mCullingViewports[NUM_CULLING_VIEWPORTS];
};

struct FillBatchData
{
    SmallBatchData data[BATCH_COUNT];
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

struct MeshInfoStruct
{
    float4   mColor;
    float3   mTranslation;
    float3   mOffsetTranslation;
    float3   mScale;
    float4x4 mTranslationMat;
    float4x4 mScaleMat;
} gMeshInfoData;

class Tutorial31_GpuDriver2 final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial31 GpuDriver2"; }

private:
    void InitConstData();
    void CreatePipelineState();
    void LoadModel();
    void CreateBuffer();
    void ThreadLoadModel(int idx);

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    RefCntAutoPtr<IBuffer> mBufferFilterIndirectMaterial;
    RefCntAutoPtr<IBuffer> mBufferFilteredIndex[NUM_CULLING_VIEWPORTS];
    RefCntAutoPtr<IBuffer> mBufferUncompactedDrawArguments[NUM_CULLING_VIEWPORTS];
    RefCntAutoPtr<IBuffer> mBufferFilteredIndirectDrawArguments[gNumGeomSets][NUM_CULLING_VIEWPORTS];

    RefCntAutoPtr<IBuffer> mBufferMaterialProps;
    RefCntAutoPtr<IBuffer> mBufferVertexData;
    RefCntAutoPtr<IBuffer> mBufferIndexData;
    RefCntAutoPtr<IBuffer> mBufferMeshConstants;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth;
    uint32_t mHeight;
    
    GLTF::Model* m_Model;

    Scene  mScene;
    Uint32 mMeshCount;
    Uint32 mMaterialCount;

    std::vector<MeshConstants> mMeshConstants;

    std::vector<ObjectConstants> mModelPassConstants;
    PassConstants                mPassCbData;

    // uniform data
    LightUniformBlock          mLightUniformBlock;
    CameraUniform              mCameraUniform;
    VisibilityBufferConstants  mVisibilityBufferCB;
    MeshInfoUniformBlock       mMeshInfoUniformData;
    std::vector<FillBatchData> mBatchDatas;

};

} // namespace Diligent
