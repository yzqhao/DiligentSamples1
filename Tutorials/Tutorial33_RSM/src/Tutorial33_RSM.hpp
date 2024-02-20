
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

#define SAMPLE_NUMBER 120

namespace Diligent
{

struct ObjectConstants
{
    float4x4 World{};
    float4x4 PreWorld{};
    Uint32   MaterialIndex;
    float    roughness;
    float    metalness;
    int      pbrMaterials;
};

struct DirectionalLight
{
    float4 mPos;
    float4 mCol; //alpha is the intesity
    float4 mDir;
};

struct PointLight
{
    float4 mPos;
    float4 mCol;
    float  mRadius;
    float  mIntensity;
    char   _pad[8];
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
    float4x4 LightView{};
    float4x4 LightProj{};
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
    int              bRSM;
    int              filterRange;

    float4 randomArray[SAMPLE_NUMBER];
};

class Tutorial33_RSM final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial33 : RSM"; }

private:
    void CreatePipelineState();
    void CreateBuffers();
    void CreateIndexBuffer();
    void LoadTexture();
    void LoadModel();

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth  = 1280;
    uint32_t mHeight = 720;

    GLTF::Model* m_Model;

    
    std::vector<ObjectConstants> mObjectCbData;
    PassConstants                mPassCbData;

    bool m_bRSM = true;
};

} // namespace Diligent
