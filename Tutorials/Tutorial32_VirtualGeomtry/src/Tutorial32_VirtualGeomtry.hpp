
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "VirtualMesh.h"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{
struct FrameContext
{
    float4x4 vp_mat;
    float4x4 world_mat;
    Uint32   view_mode{}; // 0:tri 1:cluster 2:group
    Uint32   level{};
    Uint32   display_ext_edge{};
};

class Tutorial32_VirtualGeomtry final : public SampleBase
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

    RefCntAutoPtr<IPipelineState>         m_pPSO;
    RefCntAutoPtr<IBuffer>                m_CubeVertexBuffer;
    RefCntAutoPtr<IBuffer>                m_CubeIndexBuffer;
    RefCntAutoPtr<IBuffer>                m_VSConstants;
    RefCntAutoPtr<ITextureView>           m_TextureSRV;
    RefCntAutoPtr<IShaderResourceBinding> m_SRB;
    float4x4                              m_WorldViewProjMatrix;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    VirtualMesh mVirtualMesh;

    std::vector<Uint32>              m_clusterOffsets;
    FrameContext                     mframe;
};

} // namespace Diligent
