
#include "Tutorial37_SDF.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

#include <fstream>

#define USE_SDF_TEXTURE 1

namespace Diligent
{
static Diligent::float3 gCamPos{0.0f, -0.0f, -4.0f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const Uint32 sdfResolution = 96;

static const char* gStrSDFTex3D = "SDFTex3D";
static const char* gStrSDFTex3DBuffer = "SDFTex3DBuffer";

static const char* gStrSDFRender         = "SDFRender";
static const char* gStrSDFRenderPSParams = "SDFRenderPSParams";

SampleBase* CreateSample()
{
    return new Tutorial37_SDF();
}

void Tutorial37_SDF::CreatePipelineState()
{
    {
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pBRDFIntegrationCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "IntegrationTex3D CS";
            ShaderCI.FilePath        = "IntegrationTex3D.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pBRDFIntegrationCS);
        }
        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        ShaderResourceVariableDesc Vars[] =
            {
                {SHADER_TYPE_COMPUTE, "data", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
                {SHADER_TYPE_COMPUTE, "dstTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        PSODesc.Name      = gStrSDFTex3D;
        PSOCreateInfo.pCS = pBRDFIntegrationCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrSDFTex3D]);

        m_pPSOs[gStrSDFTex3D]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSDFTex3D], true);
    }

    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrSDFRender;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
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
            ShaderCI.Desc.Name       = "SDFRenderer VS";
            ShaderCI.FilePath        = "SDFRenderer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(PSParams), "PSParams CB", &m_Buffers[gStrSDFRenderPSParams]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "SDFRenderer PS";
            ShaderCI.FilePath        = "SDFRenderer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "PSParams", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "SDF", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP};
        ImmutableSamplerDesc ImtblSamplers[] = {
             {SHADER_TYPE_PIXEL, "SDF", SamLinearWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrSDFRender]);

        m_pPSOs[gStrSDFRender]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "PSParams")->Set(m_Buffers[gStrSDFRenderPSParams]);
        m_pPSOs[gStrSDFRender]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSDFRender], true);
    }
}

void Tutorial37_SDF::CreateVertexBuffer()
{
    
}

void Tutorial37_SDF::LoadModel()
{
    static constexpr char PositionAttributeName[]  = "POSITION";
    static constexpr char NormalAttributeName[]    = "NORMAL";
    static constexpr char Texcoord0AttributeName[] = "TEXCOORD_0";

    static constexpr std::array<GLTF::VertexAttributeDesc, 3> DefaultVertexAttributes =
        {
            GLTF::VertexAttributeDesc{PositionAttributeName, 0, VT_FLOAT32, 3},
            GLTF::VertexAttributeDesc{Texcoord0AttributeName, 0, VT_FLOAT32, 2},
            GLTF::VertexAttributeDesc{NormalAttributeName, 0, VT_FLOAT32, 3},
        };

    GLTF::ModelCreateInfo ModelCI;
    ModelCI.FileName             = "bunny.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model                      = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);

    mSDFMesh.pGeometry = m_Model;
}

void Tutorial37_SDF::LoadTexture()
{
#if USE_SDF_TEXTURE
    TextureLoadInfo loadInfoMesh;

    std::string Filename = "dragon.dds";

    RefCntAutoPtr<ITexture> MeshTex;
    CreateTextureFromFile(Filename.c_str(), loadInfoMesh, m_pDevice, &MeshTex);

    m_ShaderResourceBindings[gStrSDFRender]->GetVariableByName(SHADER_TYPE_PIXEL, "SDF")->Set(MeshTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
#else 
    std::ifstream is("SDF/twosided_kinda_doorwall_04", std::ifstream::binary);
    if (!is)
    {
        return ;
    }

    int x, y, z;
    is.read((char*)&x, sizeof(float));
    is.read((char*)&y, sizeof(float));
    is.read((char*)&z, sizeof(float));

    size_t datasize   = x * y * z;
    float* data       = new float[datasize]; // 4 = r,g,b,a (@Cleanup)
    is.read((char*)data, datasize);

    BoundBox aabb;
    is.read((char*)&aabb.Min, sizeof(float3));
    is.read((char*)&aabb.Max, sizeof(float3));

    std::vector<TextureSubResData> m_SubResources(1);
    m_SubResources[0].pData  = data;
    m_SubResources[0].Stride = datasize;

    {
        BufferDesc BuffDesc;
        BuffDesc.Name              = "tex3D buffer";
        BuffDesc.Usage             = USAGE_IMMUTABLE;
        BuffDesc.BindFlags         = BIND_SHADER_RESOURCE;
        BuffDesc.Mode              = BUFFER_MODE_FORMATTED;
        BuffDesc.CPUAccessFlags    = CPU_ACCESS_NONE;
        BuffDesc.ElementByteStride = sizeof(float);
        BuffDesc.Size              = sizeof(float) * datasize;
        BufferData VBData;
        VBData.pData    = data;
        VBData.DataSize = sizeof(float) * datasize;
        m_pDevice->CreateBuffer(BuffDesc, &VBData, &m_Buffers[gStrSDFTex3DBuffer]);
    }

    TextureDesc Tex3DDesc;
    Tex3DDesc.Name      = gStrSDFRender;
    Tex3DDesc.Type      = RESOURCE_DIM_TEX_3D;
    Tex3DDesc.Width     = x;
    Tex3DDesc.Height    = y;
    Tex3DDesc.Depth     = z;
    Tex3DDesc.MipLevels = 1;
    Tex3DDesc.Format    = TEX_FORMAT_R32_FLOAT;
    Tex3DDesc.Usage     = USAGE_DEFAULT;
    Tex3DDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

    TextureData InitData{m_SubResources.data(), (Uint32)1};
    m_pDevice->CreateTexture(Tex3DDesc, nullptr, &m_Textures[gStrSDFRender]);

    RefCntAutoPtr<IBufferView> pDataSRV;
    BufferViewDesc             ViewDesc;
    ViewDesc.ViewType             = BUFFER_VIEW_SHADER_RESOURCE;
    ViewDesc.Format.ValueType     = VT_FLOAT32;
    ViewDesc.Format.NumComponents = 1;
    m_Buffers[gStrSDFTex3DBuffer]->CreateView(ViewDesc, &pDataSRV);

    m_ShaderResourceBindings[gStrSDFTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "data")->Set(pDataSRV);
    m_ShaderResourceBindings[gStrSDFTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->Set(m_Textures[gStrSDFRender]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS));

    m_ShaderResourceBindings[gStrSDFRender]->GetVariableByName(SHADER_TYPE_PIXEL, "SDF")->Set(m_Textures[gStrSDFRender]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE));
#endif
}


void Tutorial37_SDF::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    CreatePipelineState();
    CreateVertexBuffer();
    LoadModel();
    LoadTexture();

#if !USE_SDF_TEXTURE
    {
        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSDFTex3D]);
        DispatchComputeAttribs DispatAttribs;
        DispatAttribs.ThreadGroupCountX = (Uint32)ceilf(sdfResolution / 16.0f);
        DispatAttribs.ThreadGroupCountY = (Uint32)ceilf(sdfResolution / 16.0f);
        DispatAttribs.ThreadGroupCountZ = sdfResolution;
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSDFTex3D], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribs);
    }
#endif
}

// Render a frame
void Tutorial37_SDF::Render()
{
    auto* pRTV = m_pSwapChain->GetCurrentBackBufferRTV();
    auto* pDSV = m_pSwapChain->GetDepthBufferDSV();
    // Clear the back buffer
    float4 ClearColor = {0.350f, 0.350f, 0.350f, 1.0f};
    if (m_ConvertPSOutputToGamma)
    {
        // If manual gamma correction is required, we need to clear the render target with sRGB color
        ClearColor = LinearToSRGB(ClearColor);
    }
    m_pImmediateContext->ClearRenderTarget(pRTV, ClearColor.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    {
        MapHelper<PSParams> CBConstants(m_pImmediateContext, m_Buffers[gStrSDFRenderPSParams], MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = mPSParams;
    }

    // Set the pipeline state
    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSDFRender]);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSDFRender], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawAttribs DrawAttrs; // This is an indexed draw call
    DrawAttrs.NumVertices  = 6;
    m_pImmediateContext->Draw(DrawAttrs);
}

void Tutorial37_SDF::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
    float4x4 viewproj = view * proj;

    const float4x4 invViewProj = viewproj.Inverse();

    const float4 A0 = float4(-1, 1, 0.2f, 1) * invViewProj;
    const float4 A1 = float4(-1, 1, 0.5f, 1) * invViewProj;

    const float4 B0 = float4(1, 1, 0.2f, 1) * invViewProj;
    const float4 B1 = float4(1, 1, 0.5f, 1) * invViewProj;

    const float4 C0 = float4(-1, -1, 0.2f, 1) * invViewProj;
    const float4 C1 = float4(-1, -1, 0.5f, 1) * invViewProj;

    const float4 D0 = float4(1, -1, 0.2f, 1) * invViewProj;
    const float4 D1 = float4(1, -1, 0.5f, 1) * invViewProj;

    mPSParams.FrustumA = normalize((A1) / A1.w - (A0) / A0.w);
    mPSParams.FrustumB = normalize((B1) / B1.w - (B0) / B0.w);
    mPSParams.FrustumC = normalize((C1) / C1.w - (C0) / C0.w);
    mPSParams.FrustumD = normalize((D1) / D1.w - (D0) / D0.w);
    mPSParams.Eye      = m_Camera.GetPos();

    mPSParams.MaxTraceSteps = 64;
    mPSParams.AbsThreshold  = 0.01f;

    mPSParams.SDFLower = float3(-1.2f, -1.2f, -1.2f);
    mPSParams.SDFUpper   = float3(1.2f, 1.2f, 1.2f);
    mPSParams.SDFExtent = mPSParams.SDFUpper - mPSParams.SDFLower;
}

void Tutorial37_SDF::WindowResize(Uint32 Width, Uint32 Height)
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
