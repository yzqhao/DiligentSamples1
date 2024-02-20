
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
    float4x4 m_mat;
    float4x4 v_mat;
    float4x4 p_mat;
};

struct FrameContextGraphic
{
    float4x4 vp_mat;
    float4x4 m_mat;
    float4x4 v_mat;
    float4x4 p_mat;
    float4x4 vp_mat2;
    Uint32   view_mode; // 0:tri 1:cluster 2:group 3:level
    Uint32   is_post_pass;
};

struct HzbConst
{
    Uint32 lst_level;
    Uint32 tex_width;
    Uint32 width;
    Uint32 height;
};

class Tutorial32_VirtualGeomtry3 final : public SampleBase
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
    RefCntAutoPtr<IPipelineState>         m_pPSOHzb;
    RefCntAutoPtr<IPipelineState>         m_pPSOGraphic;
    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_HzbConstants;
    RefCntAutoPtr<IBuffer>                m_VSConstants;
    RefCntAutoPtr<IShaderResourceBinding> m_SRBCull;
    RefCntAutoPtr<IShaderResourceBinding> m_SRBGraphic;
    RefCntAutoPtr<IShaderResourceBinding> m_SRBHzb;

    RefCntAutoPtr<ITexture>                  m_TexDepth;
    RefCntAutoPtr<ITexture>                  m_TexDepthDSV;
    RefCntAutoPtr<ITexture>                  m_TexHzb;
    std::vector<RefCntAutoPtr<ITextureView>> m_TextureHzbRTVs;
    RefCntAutoPtr<ITextureView>              m_TextureHzbUAV;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth  = 1280;
    uint32_t mHeight = 720;

    VirtualMesh mVirtualMesh;

    std::vector<Uint32> m_clusterOffsets;
    FrameContextCull    mframeCull;
    FrameContextGraphic mframeGraphic;
    HzbConst            mHzbConst;
};

} // namespace Diligent
