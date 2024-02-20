
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "BVHTree.h"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{

struct PSParams
{
    float3 FrustumA;
    int    MaxTraceSteps;
    float3 FrustumB;
    float  AbsThreshold;
    float3 FrustumC;
    float  _pad0;
    float3 FrustumD;
    float  _pad1;

    float3 Eye;
    float  _pad2;

    float3 SDFLower;
    float  _pad3;
    float3 SDFUpper;
    float  _pad4;
    float3 SDFExtent;
    float  _pad5;
};


class Tutorial37_SDF final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial37: SDF"; }

private:
    void CreatePipelineState();
    void CreateVertexBuffer();
    void LoadModel();
    void LoadTexture();

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
    SDFMesh      mSDFMesh;

    PSParams mPSParams;
};

} // namespace Diligent
