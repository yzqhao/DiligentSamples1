
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{

struct SkyBoxConstants
{
    float4x4 View{};
    float4x4 Proj{};
    float4   gEyePosW;
};

struct ObjectConstants
{
    float4x4 World{};
    float4   Coef[16];
};

class Tutorial34_SH final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial34: SH"; }

private:
    void CreatePipelineState();
    void CreateVertexBuffer();
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

    SkyBoxConstants mSkyBoxPassCB;
    ObjectConstants mObjectCB;
};

} // namespace Diligent
