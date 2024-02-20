
#include "Tutorial29_TBDR.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include <random>

namespace Diligent
{

static Diligent::float3 gCamPos{-1.8f, 13.0f, -22.0f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const char* gStrGBuffer       = "GBuffer";
static const char* gStrGBufferFloor  = "GBufferFloor";
static const char* gStrGBufferSphere = "GBufferSphere";
static const char* gStrGBufferColumn = "GBufferColumn";

static const char* gStrGBufferA     = "GBufferA";
static const char* gStrGBufferB     = "GBufferB";
static const char* gStrGBufferC     = "GBufferC";
static const char* gStrGBufferD     = "GBufferD";
static const char* gStrGBufferE     = "GBufferE";
static const char* gStrGBufferF     = "GBufferF";
static const char* gStrGBufferDepth = "GBufferDepth";

static const char* gStrGBufferASRV     = "GBufferASRV";
static const char* gStrGBufferBSRV     = "GBufferBSRV";
static const char* gStrGBufferCSRV     = "GBufferCSRV";
static const char* gStrGBufferDSRV     = "GBufferDSRV";
static const char* gStrGBufferESRV     = "GBufferESRV";
static const char* gStrGBufferFSRV     = "GBufferFSRV";
static const char* gStrGBufferDepthSRV = "GBufferDepthSRV";

static const char* gStrGBufferARTV     = "GBufferARTV";
static const char* gStrGBufferBRTV     = "GBufferBRTV";
static const char* gStrGBufferCRTV     = "GBufferCRTV";
static const char* gStrGBufferDRTV     = "GBufferDRTV";
static const char* gStrGBufferERTV     = "GBufferERTV";
static const char* gStrGBufferFRTV     = "GBufferFRTV";
static const char* gStrGBufferDepthDSV = "GBufferDepthDSV";

static const char* gStrGBufferCBV  = "GBufferCBV";
static const char* gStrCameraCBV   = "CameraCBV";
static const char* gStrMaterialCBV = "MaterialCBV";

static const char* gStrSSAO            = "SSAO";
static const char* gStrSSAOCBV         = "SSAOCBV";
static const char* gStrSSAOCameraCBV   = "SSAOCameraCBV";
static const char* gStrSSAOSettingHCBV = "SSAOSettingHCBV";
static const char* gStrSSAOSettingVCBV = "SSAOSettingVCBV";

static const char* gStrFloorVB        = "strFloorVB";
static const char* gStrFloorIB        = "strFloorIB";
static const char* gStrFloorBase      = "strFloorBase";
static const char* gStrFloorNormal    = "strFloorNormal";
static const char* gStrFloorMetallic  = "strFloorMetallic";
static const char* gStrFloorRoughness = "strFloorRoughness";

static const char* gStrSphereVB        = "strSphereVB";
static const char* gStrSphereIB        = "strSphereIB";
static const char* gStrSphereBase      = "strSphereBase";
static const char* gStrSphereNormal    = "strSphereNormal";
static const char* gStrSphereMetallic  = "strSphereMetallic";
static const char* gStrSphereRoughness = "strSphereRoughness";

static const char* gStrColumnVB        = "strColumnVB";
static const char* gStrColumnIB        = "strColumnIB";
static const char* gStrColumnBase      = "strColumnBase";
static const char* gStrColumnNormal    = "strColumnNormal";
static const char* gStrColumnMetallic  = "strColumnMetallic";
static const char* gStrColumnRoughness = "strColumnRoughness";

static const char* gStrSSAOBuffer    = "SSAOBuffer";
static const char* gStrSSAOBufferSRV = "SSAOBufferSRV";
static const char* gStrSSAOBufferRTV = "SSAOBufferRTV";
static const char* gStrSSAOBufferUAV = "SSAOBufferUAV";

static const char* gStrSSAOTempBuffer    = "SSAOTempBuffer";
static const char* gStrSSAOTempBufferSRV = "SSAOTempBufferSRV";
static const char* gStrSSAOTempBufferUAV = "SSAOTempBufferUAV";

static const char* gStrRenderSceneBuffer     = "RenderSceneBuffer";
static const char* gStrRenderSceneBufferSRV  = "RenderSceneBufferSRV";
static const char* gStrRenderSceneBufferRTV  = "RenderSceneBufferRTV";
static const char* gStrRenderSceneCameraCBV  = "RenderSceneCameraCBV";
static const char* gStrRenderSceneLightCBV   = "RenderSceneLightCBV";
static const char* gStrTiledBaseLightCullRSV = "TiledBaseLightCullingRSV";

static const char* gStrHorzBlur = "HorzBlur";
static const char* gStrVertBlur = "VertBlur";

static const char* gStrLightParameters           = "LightParameters";
static const char* gStrLightCommonData           = "LightCommonData";
static const char* gStrTiledBaseLightCulling     = "TiledBaseLightCulling";
static const char* gStrTiledDepthDebugTexture    = "TiledDepthDebugTexture";
static const char* gStrTiledDepthDebugTextureSRV = "TiledDepthDebugTextureSRV";
static const char* gStrTiledDepthDebugTextureUAV = "TiledDepthDebugTextureUAV";

static const char* gStrTAAPass = "TAAPass";
static const char* gStrTAACBV  = "gStrTAACBV";

static TEXTURE_FORMAT gGBufferFormatA          = TEX_FORMAT_RGBA32_FLOAT;
static TEXTURE_FORMAT gGBufferFormatB          = TEX_FORMAT_RGBA8_SNORM;
static TEXTURE_FORMAT gGBufferFormatC          = TEX_FORMAT_RGBA32_FLOAT;
static TEXTURE_FORMAT gGBufferFormatD          = TEX_FORMAT_RGBA8_UNORM;
static TEXTURE_FORMAT gGBufferFormatE          = TEX_FORMAT_RG16_FLOAT;
static TEXTURE_FORMAT gGBufferFormatF          = TEX_FORMAT_RGBA8_UNORM;
static TEXTURE_FORMAT gGBufferFormatDepth      = TEX_FORMAT_D24_UNORM_S8_UINT;
static TEXTURE_FORMAT gRenderSceneBufferFormat = TEX_FORMAT_RGBA32_FLOAT;
static TEXTURE_FORMAT gSSAOFormat              = TEX_FORMAT_R16_UNORM;
static TEXTURE_FORMAT gTileDebugFormat         = TEX_FORMAT_RG16_UNORM;

SampleBase* CreateSample()
{
    return new Tutorial29_TBDR();
}

void Tutorial29_TBDR::InitLightData()
{
    std::random_device                    rd;
    std::default_random_engine            eng(rd());
    std::uniform_real_distribution<float> distr(0.0f, 1.0f);

    for (int i = -3; i < 3; i++)
    {
        for (int j = -3; j < 3; j++)
        {
            float ColorX = distr(eng);
            float ColorY = distr(eng);
            float ColorZ = distr(eng);

            LightParameters LightShaderParameter;
            LightShaderParameter.Color        = {ColorX, ColorY, ColorZ};
            LightShaderParameter.Intensity    = 10.0f;
            LightShaderParameter.Position     = float3(i * 4.0f + 1.0f, 1.0f, j * 4.0f);
            LightShaderParameter.Range        = 3.0f;
            LightShaderParameter.LightType    = ELightType::PointLight;
            LightShaderParameter.ShadowMapIdx = -1;

            mLightParameters.push_back(LightShaderParameter);
        }
    }

    mLightCommonData.LightCount = (uint)mLightParameters.size();
}

void Tutorial29_TBDR::CreatePipelineState()
{
    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrGBuffer;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 6;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = gGBufferFormatA;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[1]                = gGBufferFormatB;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[2]                = gGBufferFormatC;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[3]                = gGBufferFormatD;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[4]                = gGBufferFormatE;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[5]                = gGBufferFormatF;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = gGBufferFormatDepth;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "BasePassDefault VS";
            ShaderCI.FilePath        = "BasePassDefault.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "cbPerObject CB", &m_Buffers[gStrGBufferCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrCameraCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(MaterialData), "MaterialData CB", &m_Buffers[gStrMaterialCBV]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "BasePassDefault PS";
            ShaderCI.FilePath        = "BasePassDefault.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - normal
            LayoutElement{1, 0, 3, VT_FLOAT32, False},
            // Attribute 2 - Tangent
            LayoutElement{2, 0, 3, VT_FLOAT32, False},
            // Attribute 3 - texture coordinates
            LayoutElement{3, 0, 2, VT_FLOAT32, False},
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbMaterialData", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbMaterialData", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "BaseColorTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "NormalTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "MetallicTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "RoughnessTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "BaseColorTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "NormalTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "MetallicTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "RoughnessTexture", SamLinearWrapDesc},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrGBuffer]);

        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrGBufferCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbMaterialData")->Set(m_Buffers[gStrMaterialCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbMaterialData")->Set(m_Buffers[gStrMaterialCBV]);
        m_pPSOs[gStrGBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrGBufferFloor], true);
        m_pPSOs[gStrGBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrGBufferSphere], true);
        m_pPSOs[gStrGBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrGBufferColumn], true);
    }

    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                       = gStrSSAO;
        PSOCreateInfo.PSODesc.PipelineType                               = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets                  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                     = gSSAOFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "SSAO VS";
            ShaderCI.FilePath        = "SSAO.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "SSAO PS";
            ShaderCI.FilePath        = "SSAO.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrSSAOCameraCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(SSAOPass), "cbSSAO CB", &m_Buffers[gStrSSAOCBV]);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbSSAO", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "NormalGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "DepthGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        SamplerDesc SamPointWrapDesc{
            FILTER_TYPE_POINT, FILTER_TYPE_POINT, FILTER_TYPE_POINT,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "NormalGbuffer", SamPointWrapDesc},
            {SHADER_TYPE_PIXEL, "DepthGbuffer", SamLinearWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrSSAO]);

        m_pPSOs[gStrSSAO]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbSSAO")->Set(m_Buffers[gStrSSAOCBV]);
        m_pPSOs[gStrSSAO]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrSSAOCameraCBV]);
        m_pPSOs[gStrSSAO]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSSAO], true);
    }

    {
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pHorzBlurCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "HorzBlurCS";
            ShaderCI.Desc.Name       = "HorzBlur CS";
            ShaderCI.FilePath        = "Blur.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pHorzBlurCS);
        }
        RefCntAutoPtr<IShader> pVertBlurCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "VertBlurCS";
            ShaderCI.Desc.Name       = "VertBlur CS";
            ShaderCI.FilePath        = "Blur.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVertBlurCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        PSODesc.Name                            = gStrHorzBlur;
        ShaderResourceVariableDesc TexMapVars[] = {
            {SHADER_TYPE_COMPUTE, "cbBlurSettings", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "InputTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "OutputTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = TexMapVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(TexMapVars);
        PSOCreateInfo.pCS                                 = pHorzBlurCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrHorzBlur]);

        PSODesc.Name                                      = gStrVertBlur;
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = TexMapVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(TexMapVars);
        PSOCreateInfo.pCS                                 = pHorzBlurCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrVertBlur]);

        BufferDesc BuffDesc;
        BuffDesc.Name           = "Setting Constants buffer";
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(CbBlurSettings);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrSSAOSettingHCBV]);
        m_pPSOs[gStrHorzBlur]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "cbBlurSettings")->Set(m_Buffers[gStrSSAOSettingHCBV]);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrSSAOSettingVCBV]);
        m_pPSOs[gStrVertBlur]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "cbBlurSettings")->Set(m_Buffers[gStrSSAOSettingVCBV]);

        m_pPSOs[gStrHorzBlur]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrHorzBlur], true);
        m_pPSOs[gStrVertBlur]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrVertBlur], true);
    }

    {
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pTiledBaseLightCullingCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "CS";
            ShaderCI.Desc.Name       = "TiledBaseLightCulling CS";
            ShaderCI.FilePath        = "TiledBaseLightCulling.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pTiledBaseLightCullingCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        PSODesc.Name                            = gStrTiledBaseLightCulling;
        ShaderResourceVariableDesc TexMapVars[] = {
            {SHADER_TYPE_COMPUTE, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "Lights", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "LightCommonData", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "TileLightInfoList", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "TiledDepthDebugTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = TexMapVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(TexMapVars);
        PSOCreateInfo.pCS                                 = pTiledBaseLightCullingCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrTiledBaseLightCulling]);


        BufferDesc LightParametersBuffDesc;
        LightParametersBuffDesc.Name              = "LightParameters Constants buffer";
        LightParametersBuffDesc.Usage             = USAGE_DEFAULT;
        LightParametersBuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        LightParametersBuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
        LightParametersBuffDesc.ElementByteStride = sizeof(LightParameters);
        LightParametersBuffDesc.Size              = sizeof(LightParameters) * mLightParameters.size();
        BufferData LPData;
        LPData.pData    = mLightParameters.data();
        LPData.DataSize = sizeof(LightParameters) * static_cast<Uint32>(mLightParameters.size());
        m_pDevice->CreateBuffer(LightParametersBuffDesc, &LPData, &m_Buffers[gStrLightParameters]);
        m_BufferViews[gStrLightParameters] = m_Buffers[gStrLightParameters]->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE);

        BufferDesc LightCommonDataBuffDesc;
        LightCommonDataBuffDesc.Name           = "LightCommonData Constants buffer";
        LightCommonDataBuffDesc.Usage          = USAGE_DYNAMIC;
        LightCommonDataBuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        LightCommonDataBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        LightCommonDataBuffDesc.Size           = sizeof(LightCommonData);
        m_pDevice->CreateBuffer(LightCommonDataBuffDesc, nullptr, &m_Buffers[gStrLightCommonData]);


        Uint32 MaxTileCountX = (Uint32)ceilf(mWidth / float(TILE_BLOCK_SIZE));
        Uint32 MaxTileCountY = (Uint32)ceilf(mHeight / float(TILE_BLOCK_SIZE));

        BufferDesc BuffDesc;
        BuffDesc.Name              = "TileLightInfo Constants buffer";
        BuffDesc.Usage             = USAGE_DEFAULT;
        BuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        BuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
        BuffDesc.ElementByteStride = sizeof(TileLightInfo);
        BuffDesc.Size              = sizeof(TileLightInfo) * MaxTileCountX * MaxTileCountY;
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrTiledBaseLightCulling]);
        IBufferView* pUAV                        = m_Buffers[gStrTiledBaseLightCulling]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS);
        m_BufferViews[gStrTiledBaseLightCullRSV] = m_Buffers[gStrTiledBaseLightCulling]->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE);

        m_pPSOs[gStrTiledBaseLightCulling]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrTiledBaseLightCulling]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "LightCommonData")->Set(m_Buffers[gStrLightCommonData]);
        m_pPSOs[gStrTiledBaseLightCulling]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "Lights")->Set(m_BufferViews[gStrLightParameters]);
        m_pPSOs[gStrTiledBaseLightCulling]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "TileLightInfoList")->Set(pUAV);

        m_pPSOs[gStrTiledBaseLightCulling]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrTiledBaseLightCulling], true);
    }

    // deffered light pass
    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                       = gStrRenderSceneBuffer;
        PSOCreateInfo.PSODesc.PipelineType                               = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets                  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                     = gRenderSceneBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "DeferredLighting VS";
            ShaderCI.FilePath        = "DeferredLighting.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "DeferredLighting PS";
            ShaderCI.FilePath        = "DeferredLighting.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrRenderSceneCameraCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(DeferredLighting), "DeferredLighting CB", &m_Buffers[gStrRenderSceneLightCBV]);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "LightInfoList", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},

            {SHADER_TYPE_PIXEL, "BaseColorGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "NormalGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "WorldPosGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "OrmGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        SamplerDesc SamPointWrapDesc{
            FILTER_TYPE_POINT, FILTER_TYPE_POINT, FILTER_TYPE_POINT,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "BaseColorGbuffer", SamPointWrapDesc},
            {SHADER_TYPE_PIXEL, "NormalGbuffer", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "WorldPosGbuffer", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "OrmGbuffer", SamLinearWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrRenderSceneBuffer]);

        m_pPSOs[gStrRenderSceneBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Lights")->Set(m_BufferViews[gStrLightParameters]);
        m_pPSOs[gStrRenderSceneBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrRenderSceneCameraCBV]);
        m_pPSOs[gStrRenderSceneBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "LightInfoList")->Set(m_BufferViews[gStrTiledBaseLightCullRSV]);
        m_pPSOs[gStrRenderSceneBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrRenderSceneBuffer], true);
    }

    // TAA
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                       = gStrTAAPass;
        PSOCreateInfo.PSODesc.PipelineType                               = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets                  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                     = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "TAA VS";
            ShaderCI.FilePath        = "TAA.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "TAA PS";
            ShaderCI.FilePath        = "TAA.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pPS);
            
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrTAACBV]);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "ColorTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "PrevColorTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "VelocityGBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "DepthGbuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "ColorTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "PrevColorTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "VelocityGBuffer", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "DepthGbuffer", SamLinearWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrTAAPass]);

        m_pPSOs[gStrTAAPass]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrTAACBV]);
        m_pPSOs[gStrTAAPass]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrTAAPass], true);
    }
}

void Tutorial29_TBDR::CreateBuffer()
{
    { // Gbuffer
        TextureDesc GBufferATexDesc;
        GBufferATexDesc.Name      = gStrGBufferA;
        GBufferATexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferATexDesc.Width     = mWidth;
        GBufferATexDesc.Height    = mHeight;
        GBufferATexDesc.MipLevels = 1;
        GBufferATexDesc.Format    = gGBufferFormatA;
        GBufferATexDesc.Usage     = USAGE_DEFAULT;
        GBufferATexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferATexDesc, nullptr, &m_Textures[gStrGBufferA]);
        m_TextureViews[gStrGBufferARTV] = m_Textures[gStrGBufferA]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferASRV] = m_Textures[gStrGBufferA]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferBTexDesc;
        GBufferBTexDesc.Name      = gStrGBufferB;
        GBufferBTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferBTexDesc.Width     = mWidth;
        GBufferBTexDesc.Height    = mHeight;
        GBufferBTexDesc.MipLevels = 1;
        GBufferBTexDesc.Format    = gGBufferFormatB;
        GBufferBTexDesc.Usage     = USAGE_DEFAULT;
        GBufferBTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferBTexDesc, nullptr, &m_Textures[gStrGBufferB]);
        m_TextureViews[gStrGBufferBRTV] = m_Textures[gStrGBufferB]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferBSRV] = m_Textures[gStrGBufferB]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferCTexDesc;
        GBufferCTexDesc.Name      = gStrGBufferC;
        GBufferCTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferCTexDesc.Width     = mWidth;
        GBufferCTexDesc.Height    = mHeight;
        GBufferCTexDesc.MipLevels = 1;
        GBufferCTexDesc.Format    = gGBufferFormatC;
        GBufferCTexDesc.Usage     = USAGE_DEFAULT;
        GBufferCTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferCTexDesc, nullptr, &m_Textures[gStrGBufferC]);
        m_TextureViews[gStrGBufferCRTV] = m_Textures[gStrGBufferC]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferCSRV] = m_Textures[gStrGBufferC]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferDTexDesc;
        GBufferDTexDesc.Name      = gStrGBufferD;
        GBufferDTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferDTexDesc.Width     = mWidth;
        GBufferDTexDesc.Height    = mHeight;
        GBufferDTexDesc.MipLevels = 1;
        GBufferDTexDesc.Format    = gGBufferFormatD;
        GBufferDTexDesc.Usage     = USAGE_DEFAULT;
        GBufferDTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferDTexDesc, nullptr, &m_Textures[gStrGBufferD]);
        m_TextureViews[gStrGBufferDRTV] = m_Textures[gStrGBufferD]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferDSRV] = m_Textures[gStrGBufferD]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferETexDesc;
        GBufferETexDesc.Name      = gStrGBufferE;
        GBufferETexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferETexDesc.Width     = mWidth;
        GBufferETexDesc.Height    = mHeight;
        GBufferETexDesc.MipLevels = 1;
        GBufferETexDesc.Format    = gGBufferFormatE;
        GBufferETexDesc.Usage     = USAGE_DEFAULT;
        GBufferETexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferETexDesc, nullptr, &m_Textures[gStrGBufferE]);
        m_TextureViews[gStrGBufferERTV] = m_Textures[gStrGBufferE]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferESRV] = m_Textures[gStrGBufferE]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferFTexDesc;
        GBufferFTexDesc.Name      = gStrGBufferF;
        GBufferFTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferFTexDesc.Width     = mWidth;
        GBufferFTexDesc.Height    = mHeight;
        GBufferFTexDesc.MipLevels = 1;
        GBufferFTexDesc.Format    = gGBufferFormatF;
        GBufferFTexDesc.Usage     = USAGE_DEFAULT;
        GBufferFTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GBufferFTexDesc, nullptr, &m_Textures[gStrGBufferF]);
        m_TextureViews[gStrGBufferFRTV] = m_Textures[gStrGBufferF]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrGBufferFSRV] = m_Textures[gStrGBufferF]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GBufferDepthTexDesc;
        GBufferDepthTexDesc.Name      = gStrGBufferDepth;
        GBufferDepthTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferDepthTexDesc.Width     = mWidth;
        GBufferDepthTexDesc.Height    = mHeight;
        GBufferDepthTexDesc.MipLevels = 1;
        GBufferDepthTexDesc.Format    = gGBufferFormatDepth;
        GBufferDepthTexDesc.Usage     = USAGE_DEFAULT;
        GBufferDepthTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(GBufferDepthTexDesc, nullptr, &m_Textures[gStrGBufferDepth]);
        m_TextureViews[gStrGBufferDepthDSV] = m_Textures[gStrGBufferDepth]->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        m_TextureViews[gStrGBufferDepthSRV] = m_Textures[gStrGBufferDepth]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc SSAOTexDesc;
        SSAOTexDesc.Name      = gStrSSAO;
        SSAOTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        SSAOTexDesc.Width     = mWidth;
        SSAOTexDesc.Height    = mHeight;
        SSAOTexDesc.MipLevels = 1;
        SSAOTexDesc.Format    = gSSAOFormat;
        SSAOTexDesc.Usage     = USAGE_DEFAULT;
        SSAOTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(SSAOTexDesc, nullptr, &m_Textures[gStrSSAOBuffer]);
        m_TextureViews[gStrSSAOBufferRTV] = m_Textures[gStrSSAOBuffer]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrSSAOBufferSRV] = m_Textures[gStrSSAOBuffer]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        m_TextureViews[gStrSSAOBufferUAV] = m_Textures[gStrSSAOBuffer]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);

        TextureDesc SSAOTexDesc1;
        SSAOTexDesc1.Name      = gStrSSAO;
        SSAOTexDesc1.Type      = RESOURCE_DIM_TEX_2D;
        SSAOTexDesc1.Width     = mWidth;
        SSAOTexDesc1.Height    = mHeight;
        SSAOTexDesc1.MipLevels = 1;
        SSAOTexDesc1.Format    = gSSAOFormat;
        SSAOTexDesc1.Usage     = USAGE_DEFAULT;
        SSAOTexDesc1.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(SSAOTexDesc1, nullptr, &m_Textures[gStrSSAOTempBuffer]);
        m_TextureViews[gStrSSAOTempBufferUAV] = m_Textures[gStrSSAOTempBuffer]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);
        m_TextureViews[gStrSSAOTempBufferSRV] = m_Textures[gStrSSAOTempBuffer]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc TiledDepthDebugTextureDesc;
        TiledDepthDebugTextureDesc.Name      = gStrTiledDepthDebugTexture;
        TiledDepthDebugTextureDesc.Type      = RESOURCE_DIM_TEX_2D;
        TiledDepthDebugTextureDesc.Width     = mWidth;
        TiledDepthDebugTextureDesc.Height    = mHeight;
        TiledDepthDebugTextureDesc.MipLevels = 1;
        TiledDepthDebugTextureDesc.Format    = gTileDebugFormat;
        TiledDepthDebugTextureDesc.Usage     = USAGE_DEFAULT;
        TiledDepthDebugTextureDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(TiledDepthDebugTextureDesc, nullptr, &m_Textures[gStrTiledDepthDebugTexture]);
        m_TextureViews[gStrTiledDepthDebugTextureUAV] = m_Textures[gStrTiledDepthDebugTexture]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);
        m_TextureViews[gStrTiledDepthDebugTextureSRV] = m_Textures[gStrTiledDepthDebugTexture]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc DeferredLightingTextureDesc;
        DeferredLightingTextureDesc.Name      = gStrRenderSceneBuffer;
        DeferredLightingTextureDesc.Type      = RESOURCE_DIM_TEX_2D;
        DeferredLightingTextureDesc.Width     = mWidth;
        DeferredLightingTextureDesc.Height    = mHeight;
        DeferredLightingTextureDesc.MipLevels = 1;
        DeferredLightingTextureDesc.Format    = gRenderSceneBufferFormat;
        DeferredLightingTextureDesc.Usage     = USAGE_DEFAULT;
        DeferredLightingTextureDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(DeferredLightingTextureDesc, nullptr, &m_Textures[gStrRenderSceneBuffer]);
        m_TextureViews[gStrRenderSceneBufferRTV] = m_Textures[gStrRenderSceneBuffer]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrRenderSceneBufferSRV] = m_Textures[gStrRenderSceneBuffer]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    {
        // clang-format off
        float gScreenQuadPoints[] = {
		    -1.0f, 3.0f, 0.5f, 0.0f, -1.0f,
		    -1.0f, -1.0f, 0.5f, 0.0f, 1.0f,
		    3.0f, -1.0f, 0.5f, 2.0f, 1.0f,
	    };
        // clang-format on
        BufferDesc VertBuffDesc;
        VertBuffDesc.Name      = "ScreenQuad vertex buffer";
        VertBuffDesc.Usage     = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size      = sizeof(gScreenQuadPoints);
        BufferData VBData;
        VBData.pData    = gScreenQuadPoints;
        VBData.DataSize = sizeof(gScreenQuadPoints);
        m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrRenderSceneBuffer]);
    }
}

void Tutorial29_TBDR::CreateMesh()
{
    m_SphereMesh.CreateSphere(0.5f, 20, 20);
    m_SphereMesh.MeshName = "SphereMesh";
    m_SphereMesh.GenerateBoundingBox();

    m_GridMesh.CreateGrid(20.0f, 30.0f, 60, 40);
    m_GridMesh.MeshName = "GridMesh";
    m_GridMesh.GenerateBoundingBox();

    BufferDesc VertBuffDesc;
    VertBuffDesc.Name      = "Floor vertex buffer";
    VertBuffDesc.Usage     = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size      = sizeof(TVertex) * m_GridMesh.Vertices.size();
    BufferData VBData;
    VBData.pData    = m_GridMesh.Vertices.data();
    VBData.DataSize = sizeof(TVertex) * m_GridMesh.Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrFloorVB]);

    VertBuffDesc.Name      = "Sphere vertex buffer";
    VertBuffDesc.Usage     = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size      = sizeof(TVertex) * m_SphereMesh.Vertices.size();
    VBData.pData           = m_SphereMesh.Vertices.data();
    VBData.DataSize        = sizeof(TVertex) * m_SphereMesh.Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrSphereVB]);

    BufferDesc IndBuffDesc;
    IndBuffDesc.Name      = "Floor index buffer";
    IndBuffDesc.Usage     = USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.Size      = sizeof(uint16_t) * m_GridMesh.Indices16.size();
    BufferData IBData;
    IBData.pData    = m_GridMesh.Indices16.data();
    IBData.DataSize = sizeof(uint16_t) * m_GridMesh.Indices16.size();
    m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_Buffers[gStrFloorIB]);

    IndBuffDesc.Name      = "Sphere index buffer";
    IndBuffDesc.Usage     = USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.Size      = sizeof(uint16_t) * m_SphereMesh.Indices16.size();
    IBData.pData          = m_SphereMesh.Indices16.data();
    IBData.DataSize       = sizeof(uint16_t) * m_SphereMesh.Indices16.size();
    m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_Buffers[gStrSphereIB]);

    mObjectCbData.resize(53);

    int             idx = 0;
    ObjectConstants floor;
    floor.World          = float4x4::Scale(float3(1.5f, 1.0f, 1.5f));
    floor.PreWorld       = floor.World;
    floor.TexTransform   = float4x4::Scale(float3(4.f, 4.f, 1.f));
    mObjectCbData[idx++] = floor;

    for (int i = -3; i < 3; i++)
    {
        for (int j = -3; j < 3; j++)
        {
            ObjectConstants sphere;
            sphere.World         = float4x4::Translation(float3(i * 4.0f, 1.0f, j * 4.0f)).Transpose();
            sphere.PreWorld      = sphere.World;
            sphere.TexTransform  = float4x4::Identity();
            mObjectCbData[idx++] = sphere;
        }
    }

    for (int i = -2; i < 2; i++)
    {
        for (int j = -2; j < 2; j++)
        {
            ObjectConstants column;
            column.World         = (float4x4::Scale(10.0f) * float4x4::Translation(float3(i * 5.0f + 0.5f, -0.1f, j * 5.0f))).Transpose();
            column.PreWorld      = column.World;
            column.TexTransform  = float4x4::Identity();
            mObjectCbData[idx++] = column;
        }
    }


    static constexpr char PositionAttributeName[]  = "POSITION";
    static constexpr char NormalAttributeName[]    = "NORMAL";
    static constexpr char TangentAttributeName[]   = "TANGENT";
    static constexpr char Texcoord0AttributeName[] = "TEXCOORD_0";

    static constexpr std::array<GLTF::VertexAttributeDesc, 4> DefaultVertexAttributes =
        {
            GLTF::VertexAttributeDesc{PositionAttributeName, 0, VT_FLOAT32, 3},
            GLTF::VertexAttributeDesc{NormalAttributeName, 0, VT_FLOAT32, 3},
            GLTF::VertexAttributeDesc{TangentAttributeName, 0, VT_FLOAT32, 3},
            GLTF::VertexAttributeDesc{Texcoord0AttributeName, 0, VT_FLOAT32, 2},
        };

    GLTF::ModelCreateInfo ModelCI;
    ModelCI.FileName             = "models/column.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model                      = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);
}

void Tutorial29_TBDR::LoadTexture()
{
    // clang-format off
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    RefCntAutoPtr<ITexture> Tex;
    CreateTextureFromFile("texture/hardwood-brown-planks-albedo.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrFloorBase] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);  Tex = nullptr;
    CreateTextureFromFile("texture/column_albedo.jpg", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrColumnBase] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE); Tex = nullptr;
    CreateTextureFromFile("texture/rustediron2_basecolor.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrSphereBase] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE); Tex = nullptr;
    
    loadInfo.IsSRGB = false;
    CreateTextureFromFile("texture/hardwood-brown-planks-normal-dx.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrFloorNormal] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);    Tex = nullptr;
    CreateTextureFromFile("texture/column_normal.png", loadInfo, m_pDevice, &Tex);  
    m_TextureViews[gStrColumnNormal] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);   Tex = nullptr;
    CreateTextureFromFile("texture/rustediron2_normal.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrSphereNormal] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);   Tex = nullptr;
    
    CreateTextureFromFile("texture/hardwood-brown-planks-metallic.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrFloorMetallic] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);  Tex = nullptr;
    CreateTextureFromFile("texture/white1x1.dds", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrColumnMetallic] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE); Tex = nullptr;
    CreateTextureFromFile("texture/rustediron2_metallic.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrSphereMetallic] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE); Tex = nullptr;

    CreateTextureFromFile("texture/hardwood-brown-planks-roughness.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrFloorRoughness] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);     Tex = nullptr;
    CreateTextureFromFile("texture/column_roughness.jpg", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrColumnRoughness] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);    Tex = nullptr;
    CreateTextureFromFile("texture/rustediron2_roughness.png", loadInfo, m_pDevice, &Tex);
    m_TextureViews[gStrSphereRoughness] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);    Tex = nullptr;
    // clang-format on
}


void Tutorial29_TBDR::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    InitLightData();
    CreatePipelineState();
    CreateBuffer();
    CreateMesh();
    LoadTexture();
}

// Render a frame
void Tutorial29_TBDR::Render()
{
    {
        ITextureView*           pRTVs[6] = {m_TextureViews[gStrGBufferARTV], m_TextureViews[gStrGBufferBRTV],
                                            m_TextureViews[gStrGBufferCRTV], m_TextureViews[gStrGBufferDRTV], m_TextureViews[gStrGBufferERTV], m_TextureViews[gStrGBufferFRTV]};
        SetRenderTargetsAttribs RTAttrs;
        RTAttrs.NumRenderTargets    = 6;
        RTAttrs.ppRenderTargets     = pRTVs;
        RTAttrs.pDepthStencil       = m_TextureViews[gStrGBufferDepthDSV];
        RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

        constexpr float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pRTVs[0], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[1], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[2], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[3], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[4], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[5], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearDepthStencil(m_TextureViews[gStrGBufferDepthDSV], CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrGBuffer]);

        int idx = 0;
        {
            MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrGBufferCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mObjectCbData[idx++];
        }
        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }
        {
            MapHelper<MaterialData> CBConstants(m_pImmediateContext, m_Buffers[gStrMaterialCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mMaterialData;
        }

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrFloorVB]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pImmediateContext->SetIndexBuffer(m_Buffers[gStrFloorIB], 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_ShaderResourceBindings[gStrGBufferFloor]->GetVariableByName(SHADER_TYPE_PIXEL, "BaseColorTexture")->Set(m_TextureViews[gStrFloorBase]);
        m_ShaderResourceBindings[gStrGBufferFloor]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTexture")->Set(m_TextureViews[gStrFloorNormal]);
        m_ShaderResourceBindings[gStrGBufferFloor]->GetVariableByName(SHADER_TYPE_PIXEL, "MetallicTexture")->Set(m_TextureViews[gStrFloorMetallic]);
        m_ShaderResourceBindings[gStrGBufferFloor]->GetVariableByName(SHADER_TYPE_PIXEL, "RoughnessTexture")->Set(m_TextureViews[gStrFloorRoughness]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrGBufferFloor], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs DrawAttrs;
        DrawAttrs.IndexType  = VT_UINT16;
        DrawAttrs.NumIndices = (Uint32)m_GridMesh.Indices16.size();
        DrawAttrs.Flags      = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->DrawIndexed(DrawAttrs);

        // draw sphere
        pBuffs[0] = {m_Buffers[gStrSphereVB]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pImmediateContext->SetIndexBuffer(m_Buffers[gStrSphereIB], 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_ShaderResourceBindings[gStrGBufferSphere]->GetVariableByName(SHADER_TYPE_PIXEL, "BaseColorTexture")->Set(m_TextureViews[gStrSphereBase]);
        m_ShaderResourceBindings[gStrGBufferSphere]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTexture")->Set(m_TextureViews[gStrSphereNormal]);
        m_ShaderResourceBindings[gStrGBufferSphere]->GetVariableByName(SHADER_TYPE_PIXEL, "MetallicTexture")->Set(m_TextureViews[gStrSphereMetallic]);
        m_ShaderResourceBindings[gStrGBufferSphere]->GetVariableByName(SHADER_TYPE_PIXEL, "RoughnessTexture")->Set(m_TextureViews[gStrSphereRoughness]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrGBufferSphere], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        for (int i = -3; i < 3; i++)
        {
            for (int j = -3; j < 3; j++)
            {
                {
                    MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrGBufferCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                    *CBConstants = mObjectCbData[idx++];
                }
                DrawAttrs.NumIndices = (Uint32)m_SphereMesh.Indices16.size();
                m_pImmediateContext->DrawIndexed(DrawAttrs);
            }
        }

        //draw column
        std::vector<IBuffer*> pVBs;
        const auto            NumVBs = static_cast<Uint32>(m_Model->GetVertexBufferCount());
        pVBs.resize(NumVBs);
        for (Uint32 i = 0; i < NumVBs; ++i)
            pVBs[i] = m_Model->GetVertexBuffer(i);
        m_pImmediateContext->SetVertexBuffers(0, NumVBs, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        if (auto* pIndexBuffer = m_Model->GetIndexBuffer())
        {
            m_pImmediateContext->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        }

        m_ShaderResourceBindings[gStrGBufferColumn]->GetVariableByName(SHADER_TYPE_PIXEL, "BaseColorTexture")->Set(m_TextureViews[gStrColumnBase]);
        m_ShaderResourceBindings[gStrGBufferColumn]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTexture")->Set(m_TextureViews[gStrColumnNormal]);
        m_ShaderResourceBindings[gStrGBufferColumn]->GetVariableByName(SHADER_TYPE_PIXEL, "MetallicTexture")->Set(m_TextureViews[gStrColumnMetallic]);
        m_ShaderResourceBindings[gStrGBufferColumn]->GetVariableByName(SHADER_TYPE_PIXEL, "RoughnessTexture")->Set(m_TextureViews[gStrColumnRoughness]);
        const auto FirstIndexLocation = m_Model->GetFirstIndexLocation();
        const auto BaseVertex         = m_Model->GetBaseVertex();
        for (int i = 0; i < 16; ++i)
        {
            {
                MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrGBufferCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                *CBConstants = mObjectCbData[idx++];
            }
            for (const auto* pNode : m_Model->Scenes[m_Model->DefaultSceneId].LinearNodes)
            {
                if (!pNode->pMesh)
                    continue;
                for (const auto& primitive : pNode->pMesh->Primitives)
                {
                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrGBufferColumn], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                    if (primitive.HasIndices())
                    {
                        DrawIndexedAttribs drawAttrsColumn{primitive.IndexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
                        drawAttrsColumn.FirstIndexLocation = FirstIndexLocation + primitive.FirstIndex;
                        drawAttrsColumn.BaseVertex         = BaseVertex;
                        m_pImmediateContext->DrawIndexed(drawAttrsColumn);
                    }
                    else
                    {
                        DrawAttribs drawAttrsColumn{primitive.VertexCount, DRAW_FLAG_VERIFY_ALL};
                        drawAttrsColumn.StartVertexLocation = BaseVertex;
                        m_pImmediateContext->Draw(drawAttrsColumn);
                    }
                }
            }
        }
    }

    { // ssao
        ITextureView*           pQuadRTVs[1] = {m_TextureViews[gStrSSAOBufferRTV]};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSSAO]);
        m_ShaderResourceBindings[gStrSSAO]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalGbuffer")->Set(m_TextureViews[gStrGBufferBSRV]);
        m_ShaderResourceBindings[gStrSSAO]->GetVariableByName(SHADER_TYPE_PIXEL, "DepthGbuffer")->Set(m_TextureViews[gStrGBufferDepthSRV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSSAO], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrSSAOCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }
        {
            MapHelper<SSAOPass> CBConstants(m_pImmediateContext, m_Buffers[gStrSSAOCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mSSAOPass;
        }

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);

        // blur
        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrHorzBlur]);
        {
            MapHelper<CbBlurSettings> CBConstants(m_pImmediateContext, m_Buffers[gStrSSAOSettingHCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mCbBlurSettings;
        }
        DispatchComputeAttribs DispatAttribs;
        DispatAttribs.ThreadGroupCountX = (Uint32)ceilf(mWidth / 256.0f);
        DispatAttribs.ThreadGroupCountY = (Uint32)mHeight;
        m_ShaderResourceBindings[gStrHorzBlur]->GetVariableByName(SHADER_TYPE_COMPUTE, "InputTexture")->Set(m_TextureViews[gStrSSAOBufferSRV]);
        m_ShaderResourceBindings[gStrHorzBlur]->GetVariableByName(SHADER_TYPE_COMPUTE, "OutputTexture")->Set(m_TextureViews[gStrSSAOTempBufferUAV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrHorzBlur], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribs);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrVertBlur]);
        {
            MapHelper<CbBlurSettings> CBConstants(m_pImmediateContext, m_Buffers[gStrSSAOSettingVCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mCbBlurSettings;
        }
        DispatAttribs.ThreadGroupCountX = (Uint32)mWidth;
        DispatAttribs.ThreadGroupCountY = (Uint32)ceilf(mHeight / 256.0f);
        m_ShaderResourceBindings[gStrVertBlur]->GetVariableByName(SHADER_TYPE_COMPUTE, "InputTexture")->Set(m_TextureViews[gStrSSAOTempBufferSRV]);
        m_ShaderResourceBindings[gStrVertBlur]->GetVariableByName(SHADER_TYPE_COMPUTE, "OutputTexture")->Set(m_TextureViews[gStrSSAOBufferUAV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrVertBlur], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribs);
    }

    { // TiledBaseLightCulling
        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrTiledBaseLightCulling]);
        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }
        {
            MapHelper<LightCommonData> CBConstants(m_pImmediateContext, m_Buffers[gStrLightCommonData], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mLightCommonData;
        }
        DispatchComputeAttribs DispatAttribs;
        DispatAttribs.ThreadGroupCountX = (Uint32)ceilf((float)mWidth / TILE_BLOCK_SIZE);
        DispatAttribs.ThreadGroupCountY = (Uint32)ceilf((float)mHeight / TILE_BLOCK_SIZE);
        m_ShaderResourceBindings[gStrTiledBaseLightCulling]->GetVariableByName(SHADER_TYPE_COMPUTE, "DepthTexture")->Set(m_TextureViews[gStrGBufferDepthSRV]);
        m_ShaderResourceBindings[gStrTiledBaseLightCulling]->GetVariableByName(SHADER_TYPE_COMPUTE, "TiledDepthDebugTexture")->Set(m_TextureViews[gStrTiledDepthDebugTextureUAV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrTiledBaseLightCulling], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribs);
    }

    { // Defferred Lighting
        ITextureView*           pQuadRTVs[1] = {m_TextureViews[gStrRenderSceneBufferRTV]};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrRenderSceneBuffer]);
        m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "BaseColorGbuffer")->Set(m_TextureViews[gStrGBufferASRV]);
        m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalGbuffer")->Set(m_TextureViews[gStrGBufferBSRV]);
        m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "WorldPosGbuffer")->Set(m_TextureViews[gStrGBufferCSRV]);
        m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "OrmGbuffer")->Set(m_TextureViews[gStrGBufferDSRV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrRenderSceneBuffer], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);


        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrRenderSceneCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }
        {
            MapHelper<DeferredLighting> CBConstants(m_pImmediateContext, m_Buffers[gStrRenderSceneLightCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mDeferredLighting;
        }

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }

    { // TAA
        ITextureView*           pQuadRTVs[1] = {m_pSwapChain->GetCurrentBackBufferRTV()};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrTAACBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }
        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrTAAPass]);
        m_ShaderResourceBindings[gStrTAAPass]->GetVariableByName(SHADER_TYPE_PIXEL, "ColorTexture")->Set(m_TextureViews[gStrRenderSceneBufferSRV]);
        m_ShaderResourceBindings[gStrTAAPass]->GetVariableByName(SHADER_TYPE_PIXEL, "PrevColorTexture")->Set(m_TextureViews[gStrRenderSceneBufferSRV]);
        m_ShaderResourceBindings[gStrTAAPass]->GetVariableByName(SHADER_TYPE_PIXEL, "VelocityGBuffer")->Set(m_TextureViews[gStrGBufferESRV]);
        m_ShaderResourceBindings[gStrTAAPass]->GetVariableByName(SHADER_TYPE_PIXEL, "DepthGbuffer")->Set(m_TextureViews[gStrGBufferDepthSRV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrTAAPass], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }
}

std::vector<float> CalcGaussWeights(float sigma)
{
    float twoSigma2 = 2.0f * sigma * sigma;

    // Estimate the blur radius based on sigma since sigma controls the "width" of the bell curve.
    // For example, for sigma = 3, the width of the bell curve is
    int blurRadius = (int)ceil(2.0f * sigma);

    //const int MaxBlurRadius = 5;
    //assert(blurRadius <= MaxBlurRadius);

    std::vector<float> weights;
    weights.resize(2 * blurRadius + 1);

    float weightSum = 0.0f;

    for (int i = -blurRadius; i <= blurRadius; ++i)
    {
        float x = (float)i;

        weights[i + blurRadius] = expf(-x * x / twoSigma2);

        weightSum += weights[i + blurRadius];
    }

    // Divide by the sum so all the weights add up to 1.0.
    for (int i = 0; i < weights.size(); ++i)
    {
        weights[i] /= weightSum;
    }

    return weights;
}

void Tutorial29_TBDR::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }

#if 1
    float4x4 view = float4x4(1, 0, 0, 0,
                             0.00000000f, 0.819152057f, -0.573576450f, 0.00000000f,
                             0.00000000f, 0.573576450f, 0.819152057f, 0.00000000f,
                             1.79999995f, 1.96970558f, 25.4778385f, 1.00000000f);
    float4x4 proj = float4x4(1.35799503f, 0.00000000f, 0.00000000f, 0.00000000f,
                             0.00000000f, 2.41421342f, 0.00000000f, 0.00000000f,
                             0.00000000f, 0.00000000f, 1.00100100f, 1.00000000f,
                             0.00000000f, 0.00000000f, -0.100100100f, 0.00000000f);
    view          = view.Transpose();
    proj          = proj.Transpose();
 #else
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();

    view = view.Transpose();
    proj = proj.Transpose();
#endif;

    mPassCbData.PreViewProj = mPassCbData.ViewProj;
    mPassCbData.View        = view;
    mPassCbData.Proj        = proj;
    mPassCbData.ViewProj    = proj * view;
    mPassCbData.InvView     = mPassCbData.View.Inverse();
    mPassCbData.InvProj     = mPassCbData.Proj.Inverse();
    mPassCbData.InvViewProj = mPassCbData.ViewProj.Inverse();

    mPassCbData.EyePosW          = gCamPos;
    mPassCbData.RenderTargetSize = float4((float)mWidth, (float)mHeight, 1.0f / mWidth, 1.0f / mHeight);
    mPassCbData.NearZ            = gCamearNear;
    mPassCbData.FarZ             = gCamearFar;
    mPassCbData.TotalTime        = 0;
    mPassCbData.DeltaTime        = 0;
    mPassCbData.ViewPortSize     = {(float)mWidth, (float)mWidth, 0.0f, 0.0f};

    mMaterialData.DiffuseAlbedo = float4(1.0f);
    mMaterialData.FresnelR0     = float3(0.1f);
    mMaterialData.Roughness     = 64;
    mMaterialData.MatTransform  = float4x4::Identity();
    mMaterialData.EmissiveColor = float3(0.0f);
    mMaterialData.ShadingModel  = 0;

    // ssao
    mSSAOPass.gOcclusionRadius    = 0.03f;
    mSSAOPass.gOcclusionFadeStart = 0.01f;
    mSSAOPass.gOcclusionFadeEnd   = 0.03f;
    mSSAOPass.gSurfaceEpsilon     = 0.001f;

    auto Weights    = CalcGaussWeights(2.5f);
    int  BlurRadius = (int)Weights.size() / 2;

    mCbBlurSettings.gBlurRadius = BlurRadius;
    size_t DataSize             = Weights.size() * sizeof(float);
    memcpy_s(&(mCbBlurSettings.w0), DataSize, Weights.data(), DataSize);

    mDeferredLighting.EnableSSAO = 1;
}

void Tutorial29_TBDR::WindowResize(Uint32 Width, Uint32 Height)
{
    float NearPlane   = gCamearNear;
    float FarPlane    = gCamearFar;
    float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    m_Camera.SetProjAttribs(NearPlane, FarPlane, AspectRatio, PI_F / 4.f,
                            m_pSwapChain->GetDesc().PreTransform, m_pDevice->GetDeviceInfo().IsGLDevice());
}

} // namespace Diligent
