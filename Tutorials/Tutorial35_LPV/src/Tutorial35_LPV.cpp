
#include "Tutorial35_LPV.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

namespace Diligent
{
static Diligent::float3 gCamPos{20.0f, -2.0f, 0.9f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

SampleBase* CreateSample()
{
    return new Tutorial35_LPV();
}

void Tutorial35_LPV::CreatePipelineState()
{
    
}

void Tutorial35_LPV::CreateVertexBuffer()
{
    
}

void Tutorial35_LPV::CreateIndexBuffer()
{
    
}

void Tutorial35_LPV::LoadTexture()
{
    
}


void Tutorial35_LPV::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    CreatePipelineState();
    CreateVertexBuffer();
    CreateIndexBuffer();
    LoadTexture();
}

// Render a frame
void Tutorial35_LPV::Render()
{
    
}

void Tutorial35_LPV::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
}

void Tutorial35_LPV::WindowResize(Uint32 Width, Uint32 Height)
{
    mWidth  = Width;
    mHeight = Height;

    float NearPlane   = gCamearNear;
    float FarPlane    = gCamearFar;
    float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    m_Camera.SetProjAttribs(NearPlane, FarPlane, AspectRatio, PI_F / 4.f,
                            m_pSwapChain->GetDesc().PreTransform, m_pDevice->GetDeviceInfo().IsGLDevice());
}

} // namespace Diligent
