
#include "Tutorial32_VirtualGeomtry.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include "MeshSTL.h"
#include "Cluster.h"
#include "VirtualMesh.h"
#include <fstream>

namespace Diligent
{

static float3 gCamPos{22.0f, 1.9f, 5.4f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

SampleBase* CreateSample()
{
    return new Tutorial32_VirtualGeomtry();
}

void Tutorial32_VirtualGeomtry::BuildCluster()
{
    MeshSTL mesh;

    mesh.readSTL_Binary("Models/kitten.stl");

    std::vector<Cluster> clusters;

    mVirtualMesh.build(mesh);
}

void Tutorial32_VirtualGeomtry::CreatePipelineState()
{
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

            CreateUniformBuffer(m_pDevice, sizeof(FrameContext), "VS constants CB", &m_VSConstants);
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

        // clang-format off
        // Define vertex shader input layout
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False}
        };
        // clang-format on

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
            {SHADER_TYPE_VERTEX, "ClusterOffsets", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSO);

        m_pPSO->GetStaticVariableByName(SHADER_TYPE_VERTEX, "FrameContext")->Set(m_VSConstants);
    }
}

Uint32 as_uint(float x)
{
    return *((Uint32*)&x);
};

void Tutorial32_VirtualGeomtry::CreateBuffer()
{
    std::vector<Uint32> packed_data;
    for (size_t idx = 0; idx < mVirtualMesh.clusters.size(); ++idx)
    {
        m_clusterOffsets.push_back((Uint32)packed_data.size());
        auto&                cluster     = mVirtualMesh.clusters[idx];

        packed_data.push_back((Uint32)cluster.indexes.size() / 3); // num_tri
        packed_data.push_back((Uint32)cluster.verts.size());       // num_vert
        packed_data.push_back(cluster.group_id);
        packed_data.push_back(cluster.mip_level);

        packed_data.push_back(as_uint(cluster.sphere_bounds.center.x));
        packed_data.push_back(as_uint(cluster.sphere_bounds.center.y));
        packed_data.push_back(as_uint(cluster.sphere_bounds.center.z));
        packed_data.push_back(as_uint(cluster.sphere_bounds.radius));

        for (size_t i = 0; i < cluster.indexes.size() / 3; i++)
        { //tri data
            Uint32 i0 = cluster.indexes[i * 3];
            Uint32 i1 = cluster.indexes[i * 3 + 1];
            Uint32 i2 = cluster.indexes[i * 3 + 2];
            //assert(i0 < 256 && i1 < 256 && i2 < 256);

            Uint32 packed_tri = (i0 | (i1 << 8) | (i2 << 16));
            packed_data.push_back(packed_tri);
        }
        for (float3& p : cluster.verts)
        {
            packed_data.push_back(as_uint(p.x));
            packed_data.push_back(as_uint(p.y));
            packed_data.push_back(as_uint(p.z));
        }
        std::vector<Uint32> t(cluster.indexes.size());
        for (Uint32 i : cluster.external_edges) t[i] = 1;
        for (Uint32 x : t) packed_data.push_back(x);
    }
 #if 0
    std::fstream myFile;                                                               //实例化一个文件对象
    myFile.open("packed_data.txt", std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc); //打开文件firstFile.txt，可选择三种模式
    if (myFile.is_open())                                                                          //检测open()是否成功
    {
        myFile.write(reinterpret_cast<char*>(packed_data.data()), sizeof(Uint32) * packed_data.size()); // binary output
        myFile.close(); //关闭文件流以保存其内容，这一步不能忘
    }

    std::fstream myFile2;                                                                                                    //实例化一个文件对象
    myFile2.open("offset.txt", std::ios_base::binary | std::ios_base::in | std::ios_base::out | std::ios_base::trunc); //打开文件firstFile.txt，可选择三种模式
    if (myFile2.is_open())                                                                                                   //检测open()是否成功
    {
        myFile2.write(reinterpret_cast<char*>(m_clusterOffsets.data()), sizeof(Uint32) * m_clusterOffsets.size()); // binary output
        myFile2.close();                                 //关闭文件流以保存其内容，这一步不能忘
    }
 #endif
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
    BuffDesc1.Name              = "offset buffer";
    BuffDesc1.Usage             = USAGE_IMMUTABLE;
    BuffDesc1.BindFlags         = BIND_SHADER_RESOURCE;
    BuffDesc1.Mode              = BUFFER_MODE_FORMATTED;
    BuffDesc1.CPUAccessFlags    = CPU_ACCESS_NONE;
    BuffDesc1.ElementByteStride = sizeof(Uint32);
    BuffDesc1.Size              = sizeof(Uint32) * m_clusterOffsets.size();
    BufferData VBData1;
    VBData1.pData    = m_clusterOffsets.data();
    VBData1.DataSize = sizeof(Uint32) * m_clusterOffsets.size();
    m_pDevice->CreateBuffer(BuffDesc1, &VBData1, &m_Buffers["offset"]);

    RefCntAutoPtr<IBufferView> pDataSRV, pOffsetSRV;
    BufferViewDesc             ViewDesc;
    ViewDesc.ViewType             = BUFFER_VIEW_SHADER_RESOURCE;
    ViewDesc.Format.ValueType     = VT_UINT32;
    ViewDesc.Format.NumComponents = 1;
    m_Buffers["cluster"]->CreateView(ViewDesc, &pDataSRV);
    m_Buffers["offset"]->CreateView(ViewDesc, &pOffsetSRV);

    m_pPSO->CreateShaderResourceBinding(&m_SRB, true);

    m_SRB->GetVariableByName(SHADER_TYPE_VERTEX, "data")->Set(pDataSRV);
    m_SRB->GetVariableByName(SHADER_TYPE_VERTEX, "ClusterOffsets")->Set(pOffsetSRV);
}

void Tutorial32_VirtualGeomtry::CreateIndexBuffer()
{
    
}

void Tutorial32_VirtualGeomtry::LoadTexture()
{
    
}


void Tutorial32_VirtualGeomtry::Initialize(const SampleInitInfo& InitInfo)
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
void Tutorial32_VirtualGeomtry::Render()
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
        MapHelper<FrameContext> CBConstants(m_pImmediateContext, m_VSConstants, MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = mframe;
    }

    // Set the pipeline state
    m_pImmediateContext->SetPipelineState(m_pPSO);
    // Commit shader resources. RESOURCE_STATE_TRANSITION_MODE_TRANSITION mode
    // makes sure that resources are transitioned to required states.
    m_pImmediateContext->CommitShaderResources(m_SRB, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    DrawAttribs DrawAttrs;     // This is an indexed draw call
    DrawAttrs.NumVertices = 128 * 3;
    DrawAttrs.NumInstances = mVirtualMesh.clusters.size();
    m_pImmediateContext->Draw(DrawAttrs);
}

void Tutorial32_VirtualGeomtry::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
    mframe.vp_mat  = view * proj;
    mframe.world_mat = float4x4::Scale(0.05f);

    if (m_InputController.IsKeyDown(InputKeys::ShiftDown))
        mframe.view_mode = (mframe.view_mode + 1) % 4;
    if (m_InputController.IsKeyDown(InputKeys::AltDown))
        mframe.level = (mframe.level + 1) % mVirtualMesh.num_mip_level;
    if (m_InputController.IsKeyDown(InputKeys::ControlDown))
        mframe.display_ext_edge ^= 1;
}

void Tutorial32_VirtualGeomtry::WindowResize(Uint32 Width, Uint32 Height)
{
    float NearPlane   = gCamearNear;
    float FarPlane    = gCamearFar;
    float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    m_Camera.SetProjAttribs(NearPlane, FarPlane, AspectRatio, PI_F / 4.f,
                            m_pSwapChain->GetDesc().PreTransform, m_pDevice->GetDeviceInfo().IsGLDevice());
}

} // namespace Diligent
