
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "VirtualMesh.h"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{
struct FrameContextCull
{
    float4x4 vp_mat;
    float4x4 w_mat;
    float4x4 v_mat;
};

struct FrameContextGraphic
{
    float4x4 vp_mat;
    float4x4 w_mat;
    uint     view_mode;
};

class Tutorial32_VirtualGeomtry2 final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial32 : VirtualGeomtry"; }

private:
    void BuildCluster();
    void CreatePipelineState();
    void CreateBuffer();
    void CreateIndexBuffer();
    void LoadTexture();
    
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;

    RefCntAutoPtr<IPipelineState>         m_pPSOCull;
    RefCntAutoPtr<IPipelineState>         m_pPSOGraphic;
    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_VSConstants;
    RefCntAutoPtr<ITextureView>           m_TextureSRV;
    RefCntAutoPtr<IShaderResourceBinding> m_SRBCull;
    RefCntAutoPtr<IShaderResourceBinding> m_SRBGraphic;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    VirtualMesh mVirtualMesh;

    std::vector<Uint32> m_clusterOffsets;
    FrameContextCull    mframeCull;
    FrameContextGraphic mframeGraphic;
};

} // namespace Diligent
