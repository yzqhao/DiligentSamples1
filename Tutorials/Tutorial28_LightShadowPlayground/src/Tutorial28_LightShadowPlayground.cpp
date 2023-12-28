
#include "Tutorial28_LightShadowPlayground.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

namespace Diligent
{

static uint32_t gWidth  = 1440;
static uint32_t gHeight = 810;

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const char* gStrClearVisibilityBuffers = "ClearVisibilityBuffers";

SampleBase* CreateSample()
{
    return new Tutorial28_LightShadowPlayground();
}

void Tutorial28_LightShadowPlayground::CreatePipelineState()
{
    { //Triangle Filtering Pass
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pclearVisibilityBuffersCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "clearVisibilityBuffers CS";
            ShaderCI.FilePath        = "clearVisibilityBuffers.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pclearVisibilityBuffersCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        PSODesc.Name      = gStrClearVisibilityBuffers;
        PSOCreateInfo.pCS = pclearVisibilityBuffersCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrClearVisibilityBuffers]);
    }
}

void Tutorial28_LightShadowPlayground::LoadTexture()
{
}


void Tutorial28_LightShadowPlayground::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    CreatePipelineState();
    LoadTexture();
}

// Render a frame
void Tutorial28_LightShadowPlayground::Render()
{
}

void Tutorial28_LightShadowPlayground::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);
}

} // namespace Diligent
