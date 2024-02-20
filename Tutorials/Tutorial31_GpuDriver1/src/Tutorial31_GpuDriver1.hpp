
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"
#include "MeshSTL.h"

#include <map>

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

class Tutorial31_GpuDriver1 final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial31 GpuDriver1"; }

private:
    void InitConstData();
    void CreatePipelineStateSimple();
    void CreatePipelineState();
    void CreateBuffer();
    void LoadModel();
    void ThreadLoadModel(int idx);

    void RernderStl();
    void RenderGltf();

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth;
    uint32_t mHeight;
    
    GLTF::Model* m_Model;

    std::vector<MeshSTL> mStlModel;

    std::vector<ObjectConstants> mModelPassConstants;
    PassConstants                mPassCbData;

};

} // namespace Diligent
