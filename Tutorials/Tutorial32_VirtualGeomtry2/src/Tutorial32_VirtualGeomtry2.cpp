
#include "Tutorial32_VirtualGeomtry2.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include "MeshSTL.h"
#include "Cluster.h"
#include "VirtualMesh.h"

namespace Diligent
{

static float3 gCamPos{22.0f, 1.9f, 5.4f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

SampleBase* CreateSample()
{
    return new Tutorial32_VirtualGeomtry2();
}

void Tutorial32_VirtualGeomtry2::BuildCluster()
{
    MeshSTL mesh;

    mesh.readSTL_Binary("Models/kitten.stl");

    std::vector<Cluster> clusters;

    mVirtualMesh.build(mesh);
}

void Tutorial32_VirtualGeomtry2::CreatePipelineState()
{
    { 
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pCullCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "culling CS";
            ShaderCI.FilePath        = "culling.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pCullCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        // clang-format off
        ShaderResourceVariableDesc ProjectionVars[] = {
            {SHADER_TYPE_COMPUTE, "FrameContext", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "data", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "arg", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "visibility", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = ProjectionVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(ProjectionVars);

        PSODesc.Name      = "culling";
        PSOCreateInfo.pCS = pCullCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOCull);

        BufferDesc BuffDesc;
        BuffDesc.Name           = "Constants buffer";
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(FrameContextCull);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers["Frame"]);
        m_pPSOCull->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "FrameContext")->Set(m_Buffers["Frame"]);
    }
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name         = "virtual geomtry PSO";
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_BACK;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        // clang-format on

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        // OpenGL backend requires emulated combined HLSL texture samplers (g_Texture + g_Texture_sampler combination)
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
            ShaderCI.Desc.Name       = "viewer VS";
            ShaderCI.FilePath        = "viewer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(FrameContextGraphic), "VS constants CB", &m_VSConstants);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "viewer PS";
            ShaderCI.FilePath        = "viewer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        //PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        //PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = {
            {SHADER_TYPE_VERTEX, "FrameContext", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "data", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, "visibility", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOGraphic);

        m_pPSOGraphic->GetStaticVariableByName(SHADER_TYPE_VERTEX, "FrameContext")->Set(m_VSConstants);
    }
}

Uint32 as_uint(float x)
{
    return *((Uint32*)&x);
};

void Tutorial32_VirtualGeomtry2::CreateBuffer()
{
    std::vector<Uint32> packed_data;
    packed_data.push_back(mVirtualMesh.clusters.size()); //num cluster
    packed_data.push_back(0);
    packed_data.push_back(0);
    packed_data.push_back(0);
    for (size_t idx = 0; idx < mVirtualMesh.clusters.size(); ++idx)
    {
        auto& cluster = mVirtualMesh.clusters[idx];

        packed_data.push_back(cluster.verts.size());       //num vert
        packed_data.push_back(0);                          //v data ofs
        packed_data.push_back(cluster.indexes.size() / 3); //num tri
        packed_data.push_back(0);                          //t data ofs

        packed_data.push_back(as_uint(cluster.lod_bounds.center.x)); //lod bounds
        packed_data.push_back(as_uint(cluster.lod_bounds.center.y));
        packed_data.push_back(as_uint(cluster.lod_bounds.center.z));
        packed_data.push_back(as_uint(cluster.lod_bounds.radius));

        Sphere parent_lod_bounds    = mVirtualMesh.cluster_groups[cluster.group_id].lod_bounds;
        float  max_parent_lod_error = mVirtualMesh.cluster_groups[cluster.group_id].max_parent_lod_error;
        packed_data.push_back(as_uint(parent_lod_bounds.center.x)); //parent lod bounds
        packed_data.push_back(as_uint(parent_lod_bounds.center.y));
        packed_data.push_back(as_uint(parent_lod_bounds.center.z));
        packed_data.push_back(as_uint(parent_lod_bounds.radius));

        packed_data.push_back(as_uint(cluster.lod_error));
        packed_data.push_back(as_uint(max_parent_lod_error));
        packed_data.push_back(cluster.group_id);
        packed_data.push_back(cluster.mip_level);
    }
    Uint32 i = 0;
    for (auto& cluster : mVirtualMesh.clusters)
    {
        Uint32 ofs              = 4 + 16 * i;
        packed_data[ofs + 1] = packed_data.size();
        for (float3& p : cluster.verts)
        {
            packed_data.push_back(as_uint(p.x));
            packed_data.push_back(as_uint(p.y));
            packed_data.push_back(as_uint(p.z));
        }

        packed_data[ofs + 3] = packed_data.size();
        for (Uint32 i = 0; i < cluster.indexes.size() / 3; i++)
        { //tri data
            Uint32 i0 = cluster.indexes[i * 3];
            Uint32 i1 = cluster.indexes[i * 3 + 1];
            Uint32 i2 = cluster.indexes[i * 3 + 2];
            //assert(i0 < 256 && i1 < 256 && i2 < 256);

            Uint32 packed_tri = (i0 | (i1 << 8) | (i2 << 16));
            packed_data.push_back(packed_tri);
        }
        i++;
    }

    BufferDesc BuffDesc;
    BuffDesc.Name              = "cluster buffer";
    BuffDesc.Usage             = USAGE_IMMUTABLE;
    BuffDesc.BindFlags         = BIND_SHADER_RESOURCE;
    BuffDesc.Mode              = BUFFER_MODE_FORMATTED;
    BuffDesc.CPUAccessFlags    = CPU_ACCESS_NONE;
    BuffDesc.ElementByteStride = sizeof(Uint32);
    BuffDesc.Size              = sizeof(Uint32) * packed_data.size();
    BufferData VBData;
    VBData.pData    = packed_data.data();
    VBData.DataSize = sizeof(Uint32) * packed_data.size();
    m_pDevice->CreateBuffer(BuffDesc, &VBData, &m_Buffers["cluster"]);

    BufferDesc BuffDesc1;
    BuffDesc1.Name              = "visibility buffer";
    BuffDesc1.Usage             = USAGE_DEFAULT;
    BuffDesc1.BindFlags         = BIND_SHADER_RESOURCE         | BIND_UNORDERED_ACCESS;
    BuffDesc1.Mode              = BUFFER_MODE_FORMATTED;
    BuffDesc1.CPUAccessFlags    = CPU_ACCESS_NONE;
    BuffDesc1.ElementByteStride = sizeof(Uint32);
    BuffDesc1.Size              = sizeof(Uint32) * packed_data.size();
    m_pDevice->CreateBuffer(BuffDesc1, nullptr, &m_Buffers["visibility"]);

    BufferDesc BuffDesc2;
    BuffDesc2.Name              = "arg buffer";
    BuffDesc2.Usage             = USAGE_DEFAULT;
    BuffDesc2.BindFlags         = BIND_INDIRECT_DRAW_ARGS | BIND_UNORDERED_ACCESS;
    BuffDesc2.Mode              = BUFFER_MODE_FORMATTED;
    BuffDesc2.CPUAccessFlags    = CPU_ACCESS_NONE;
    BuffDesc2.ElementByteStride = sizeof(Uint32);
    BuffDesc2.Size              = sizeof(Uint32) * 4;
    m_pDevice->CreateBuffer(BuffDesc2, nullptr, &m_Buffers["arg"]);

    RefCntAutoPtr<IBufferView> pDataSRV, pVisSRV, pVisUAV, pArgUAV;
    BufferViewDesc             ViewDesc;
    ViewDesc.ViewType             = BUFFER_VIEW_SHADER_RESOURCE;
    ViewDesc.Format.ValueType     = VT_UINT32;
    ViewDesc.Format.NumComponents = 1;
    m_Buffers["cluster"]->CreateView(ViewDesc, &pDataSRV);
    m_Buffers["visibility"]->CreateView(ViewDesc, &pVisSRV);
    ViewDesc.ViewType = BUFFER_VIEW_UNORDERED_ACCESS;
    m_Buffers["arg"]->CreateView(ViewDesc, &pArgUAV);
    m_Buffers["visibility"]->CreateView(ViewDesc, &pVisUAV);

    m_pPSOCull->CreateShaderResourceBinding(&m_SRBCull, true);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "data")->Set(pDataSRV);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "visibility")->Set(pVisUAV);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "arg")->Set(pArgUAV);

    m_pPSOGraphic->CreateShaderResourceBinding(&m_SRBGraphic, true);
    m_SRBGraphic->GetVariableByName(SHADER_TYPE_VERTEX, "data")->Set(pDataSRV);
    m_SRBGraphic->GetVariableByName(SHADER_TYPE_VERTEX, "visibility")->Set(pVisSRV);
}

void Tutorial32_VirtualGeomtry2::CreateIndexBuffer()
{
    
}

void Tutorial32_VirtualGeomtry2::LoadTexture()
{
    
}


void Tutorial32_VirtualGeomtry2::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    BuildCluster();
    CreatePipelineState();
    CreateBuffer();
    CreateIndexBuffer();
    LoadTexture();
}

// Render a frame
void Tutorial32_VirtualGeomtry2::Render()
{
    {
        MapHelper<FrameContextCull> ConstData(m_pImmediateContext, m_Buffers["Frame"], MAP_WRITE, MAP_FLAG_DISCARD);
        *ConstData = mframeCull;
    }
    m_pImmediateContext->SetPipelineState(m_pPSOCull);
    DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = (Uint32)mVirtualMesh.clusters.size() / 32 + 1;
    m_pImmediateContext->CommitShaderResources(m_SRBCull, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribs);

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
        MapHelper<FrameContextGraphic> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = mframeGraphic;
    }

    // Set the pipeline state
    m_pImmediateContext->SetPipelineState(m_pPSOGraphic);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    m_pImmediateContext->CommitShaderResources(m_SRBGraphic, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawIndirectAttribs DrawAttrs {m_Buffers["arg"], DRAW_FLAG_VERIFY_ALL}; 
    m_pImmediateContext->DrawIndirect(DrawAttrs);
    
}

void Tutorial32_VirtualGeomtry2::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
    mframeGraphic.vp_mat = view * proj;
    mframeGraphic.w_mat = float4x4::Scale(0.05f);
    mframeGraphic.view_mode = 0;

    mframeCull.vp_mat   = view * proj;
    mframeCull.w_mat    = float4x4::Scale(0.05f);
    mframeCull.v_mat    = view;
}

void Tutorial32_VirtualGeomtry2::WindowResize(Uint32 Width, Uint32 Height)
{
    float NearPlane   = gCamearNear;
    float FarPlane    = gCamearFar;
    float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    m_Camera.SetProjAttribs(NearPlane, FarPlane, AspectRatio, PI_F / 4.f,
                            m_pSwapChain->GetDesc().PreTransform, m_pDevice->GetDeviceInfo().IsGLDevice());
}

} // namespace Diligent
