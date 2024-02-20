

#include "Tutorial30_VirtualTexture.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

namespace Diligent
{

const char* gPlanetName[] = {"8k_sun", "8k_mercury", "8k_venus", "8k_earth", "16k_moon", "8k_mars", "8k_jupiter", "8k_saturn" };

const uint32_t			    gImageCount = 3;
const int					gSphereResolution = 30;    // Increase for higher resolution spheres
const float					gSphereDiameter = 0.5f;
const uint					gNumPlanets = 8;        // Sun, Mercury -> Neptune, Pluto, Moon
const uint					gTimeOffset = 1000000;    // For visually better starting locations
const float					gRotSelfScale = 0.0004f;
const float					gRotOrbitYScale = 0.001f;
const float					gRotOrbitZScale = 0.00001f;

uint								gFrequency = 1;
bool								gDebugMode = false;
float								gTimeScale = 1.0f;
bool                                gPlay        = true;


static const char*                  gStrSphereVB = "strSphereVB";
static const char*                  gStrSphereIB = "strSphereIB";

static const char* gStrPlanet = "Planet";
static const char* gStrUniformVirtualTextureInfoVS = "UniformVirtualTextureInfoVS";
static const char* gStrUniformVirtualTextureInfoPS = "UniformVirtualTextureInfoPS";
static const char* gStrUniformBlockVS              = "UniformBlockVS";
static const char* gStrVTBufferInfoPS              = "VTBufferInfoPS";

SampleBase* CreateSample()
{
    return new Tutorial03_Texturing();
}

void Tutorial03_Texturing::CreatePipelineState()
{
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrPlanet;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 4;
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
            ShaderCI.Desc.Name       = "basic VS";
            ShaderCI.FilePath        = "basic.vert.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(UniformVirtualTextureInfo), "UniformVirtualTextureInfo CB", &m_Buffers[gStrUniformVirtualTextureInfoVS]);
            CreateUniformBuffer(m_pDevice, sizeof(UniformBlock), "UniformBlock CB", &m_Buffers[gStrUniformBlockVS]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "basic PS";
            ShaderCI.FilePath        = "basic.frag.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(UniformVirtualTextureInfo), "UniformVirtualTextureInfo CB", &m_Buffers[gStrUniformVirtualTextureInfoPS]);
            CreateUniformBuffer(m_pDevice, sizeof(uint) * 2, "VTBufferInfo CB", &m_Buffers[gStrVTBufferInfoPS]);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False},
                // Attribute 1 - normal
                LayoutElement{2, 0, 3, VT_FLOAT32, False}};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "SparseTextureInfo", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "uniformBlock", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "SparseTextureInfo", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "VTBufferInfo", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "VTVisBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "SparseTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
            {
                {SHADER_TYPE_PIXEL, "SparseTexture", SamLinearWrapDesc}};
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrPlanet]);

        m_pPSOs[gStrPlanet]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "SparseTextureInfo")->Set(m_Buffers[gStrUniformVirtualTextureInfoVS]);
        m_pPSOs[gStrPlanet]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "uniformBlock")->Set(m_Buffers[gStrUniformBlockVS]);
        m_pPSOs[gStrPlanet]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "SparseTextureInfo")->Set(m_Buffers[gStrUniformVirtualTextureInfoPS]);
        m_pPSOs[gStrPlanet]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "VTBufferInfo")->Set(m_Buffers[gStrVTBufferInfoPS]);
        m_pPSOs[gStrPlanet]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrPlanet], true);
    }
}

void Tutorial03_Texturing::CreateMesh()
{
    m_SphereMesh.CreateSphere(1.0f, 20, 20);
    m_SphereMesh.MeshName = "SphereMesh";
    m_SphereMesh.GenerateBoundingBox();

    BufferDesc VertBuffDesc;
    VertBuffDesc.Name      = "Sphere vertex buffer";
    VertBuffDesc.Usage     = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size      = sizeof(TVertex) * m_SphereMesh.Vertices.size();
    BufferData VBData;
    VBData.pData           = m_SphereMesh.Vertices.data();
    VBData.DataSize        = sizeof(TVertex) * m_SphereMesh.Vertices.size();
    m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrSphereVB]);

    BufferDesc IndBuffDesc;
    IndBuffDesc.Name      = "Sphere index buffer";
    IndBuffDesc.Usage     = USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.Size      = sizeof(uint16_t) * m_SphereMesh.Indices16.size();
    BufferData IBData;
    IBData.pData          = m_SphereMesh.Indices16.data();
    IBData.DataSize       = sizeof(uint16_t) * m_SphereMesh.Indices16.size();
    m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_Buffers[gStrSphereIB]);
}

void Tutorial03_Texturing::InitPlanet()
{
    PlanetInfoStruct PlanetInfo;

    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 0; // Earth years for one orbit
    PlanetInfo.mZOrbitSpeed    = 0;
    PlanetInfo.mRotationSpeed  = 24.0f; // Earth days for one rotation
    PlanetInfo.mTranslationMat = float4x4::Identity();
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(10.0f));
    PlanetInfo.mColor          = float4(0.9f, 0.6f, 0.1f, 0.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Mercury
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 0.24f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 58.7f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(12.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(1.0f));
    PlanetInfo.mColor          = float4(0.7f, 0.3f, 0.1f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Venus
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 0.68f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 243.0f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(20.0f, 0, 5));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(2));
    PlanetInfo.mColor          = float4(0.8f, 0.6f, 0.1f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Earth
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 1.0f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 1.0f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(30.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(4));
    PlanetInfo.mColor          = float4(0.3f, 0.2f, 0.8f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Moon
    PlanetInfo.mParentIndex    = 3;
    PlanetInfo.mYOrbitSpeed    = 1.0f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 27.3f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(6.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(1));
    PlanetInfo.mColor          = float4(0.3f, 0.3f, 0.4f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Mars
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 1.9f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 1.03f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(40.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(3));
    PlanetInfo.mColor          = float4(0.9f, 0.3f, 0.1f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Jupiter
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 12.0f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 0.4f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(55.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(8));
    PlanetInfo.mColor          = float4(0.6f, 0.4f, 0.4f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Saturn
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 29.0f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 0.45f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(70.0f, 0, 0)) * float4x4::RotationZ(0.5f);
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(6));
    PlanetInfo.mColor          = float4(0.7f, 0.7f, 0.5f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Uranus
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 84.07f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 0.8f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(70.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(7));
    PlanetInfo.mColor          = float4(0.4f, 0.4f, 0.6f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Neptune
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 164.81f;
    PlanetInfo.mZOrbitSpeed    = 0.0f;
    PlanetInfo.mRotationSpeed  = 0.9f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(80.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(8));
    PlanetInfo.mColor          = float4(0.5f, 0.2f, 0.9f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);

    // Pluto - Not a planet XDD
    PlanetInfo.mParentIndex    = 0;
    PlanetInfo.mYOrbitSpeed    = 247.7f;
    PlanetInfo.mZOrbitSpeed    = 1.0f;
    PlanetInfo.mRotationSpeed  = 7.0f;
    PlanetInfo.mTranslationMat = float4x4::Translation(float3(90.0f, 0, 0));
    PlanetInfo.mScaleMat       = float4x4::Scale(float3(1.0f));
    PlanetInfo.mColor          = float4(0.7f, 0.5f, 0.5f, 1.0f);

    gPlanetInfoData.push_back(PlanetInfo);
}

void Tutorial03_Texturing::LoadTexture()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    RefCntAutoPtr<ITexture> Tex;
    CreateTextureFromFile("DGLogo.png", loadInfo, m_pDevice, &Tex);
    //m_TextureSRV = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
}


void Tutorial03_Texturing::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    CreatePipelineState();
    CreateMesh();
    LoadTexture();
}

// Render a frame
void Tutorial03_Texturing::Render()
{
    
}

void Tutorial03_Texturing::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    
}

} // namespace Diligent
