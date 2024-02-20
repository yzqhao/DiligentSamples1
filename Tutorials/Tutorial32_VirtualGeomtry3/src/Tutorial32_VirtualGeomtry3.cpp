
#include "Tutorial32_VirtualGeomtry3.hpp"
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
    return new Tutorial32_VirtualGeomtry3();
}

void Tutorial32_VirtualGeomtry3::BuildCluster()
{
    MeshSTL mesh;

    mesh.readSTL_Binary("Models/kitten.stl");

    std::vector<Cluster> clusters;

    mVirtualMesh.build(mesh);
}

void Tutorial32_VirtualGeomtry3::CreatePipelineState()
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
            {SHADER_TYPE_COMPUTE, "instance", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "level_deps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = ProjectionVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(ProjectionVars);

        // clang-format off
        SamplerDesc SamLinearWarpDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP
        };
        ImmutableSamplerDesc ImtblSamplers[] = {
            {SHADER_TYPE_COMPUTE, "level_deps", SamLinearWarpDesc}
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

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

        PSOCreateInfo.PSODesc.Name         = "hzb PSO";
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 0;
        //PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat; 
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = TEX_FORMAT_D32_FLOAT;
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
            ShaderCI.Desc.Name       = "hzb VS";
            ShaderCI.FilePath        = "hzb.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(HzbConst), "hzb constants CB", &m_HzbConstants);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "hzb PS";
            ShaderCI.FilePath        = "hzb.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
        };
        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
            {
                {SHADER_TYPE_PIXEL, "level_deps", SamLinearWrapDesc},
            };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        ShaderResourceVariableDesc Vars[] = {
            {SHADER_TYPE_VERTEX, "HzbConst", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "level_deps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOHzb);

        m_pPSOHzb->GetStaticVariableByName(SHADER_TYPE_PIXEL, "HzbConst")->Set(m_HzbConstants);
    }
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name         = "virtual geomtry PSO";
        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        // clang-format off
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = TEX_FORMAT_D32_FLOAT;
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
            {SHADER_TYPE_VERTEX, "visibility", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_VERTEX, "instance", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
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
float as_float(Uint32 x)
{
    return *((float*)&x);
}

Uint32 high_bit(Uint32 x)
{
    Uint32 res = 0, t = 16, y = 0;
    y = -((x >> t) != 0), res += y & t, x >>= y & t, t >>= 1;
    y = -((x >> t) != 0), res += y & t, x >>= y & t, t >>= 1;
    y = -((x >> t) != 0), res += y & t, x >>= y & t, t >>= 1;
    y = -((x >> t) != 0), res += y & t, x >>= y & t, t >>= 1;
    y = (x >> t) != 0, res += y;
    return res;
}

void Tutorial32_VirtualGeomtry3::CreateBuffer()
{
    std::vector<Uint32> packed_data;
    packed_data.push_back(mVirtualMesh.clusters.size()); //num cluster
    packed_data.push_back(mVirtualMesh.cluster_groups.size()); //num group
    packed_data.push_back(0);                        //group data ofs
    packed_data.push_back(0);
    for (size_t idx = 0; idx < mVirtualMesh.clusters.size(); ++idx)
    {
        auto& cluster = mVirtualMesh.clusters[idx];

        packed_data.push_back(cluster.verts.size());       //num vert
        packed_data.push_back(0);                          //v data ofs
        packed_data.push_back(cluster.indexes.size() / 3); //num tri
        packed_data.push_back(0);                          //t data ofs

        packed_data.push_back(as_uint(cluster.sphere_bounds.center.x)); //bounds
        packed_data.push_back(as_uint(cluster.sphere_bounds.center.y));
        packed_data.push_back(as_uint(cluster.sphere_bounds.center.z));
        packed_data.push_back(as_uint(cluster.sphere_bounds.radius));

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
    packed_data[2] = packed_data.size(); //group data ofs
    for (auto& group : mVirtualMesh.cluster_groups)
    {
        packed_data.push_back(group.clusters.size()); //num cluster
        packed_data.push_back(0);                     //cluter data ofs
        packed_data.push_back(as_uint(group.max_parent_lod_error));
        packed_data.push_back(0);

        packed_data.push_back(as_uint(group.lod_bounds.center.x)); //lod bounds
        packed_data.push_back(as_uint(group.lod_bounds.center.y));
        packed_data.push_back(as_uint(group.lod_bounds.center.z));
        packed_data.push_back(as_uint(group.lod_bounds.radius));
    }
    Uint32 i = 0;
    for (auto& cluster : mVirtualMesh.clusters)
    {
        Uint32 ofs              = 4 + 20 * i;
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
    i = 0;
    for (auto& group : mVirtualMesh.cluster_groups)
    {
        Uint32 ofs              = packed_data[2] + 8 * i;
        packed_data[ofs + 1] = packed_data.size();
        for (Uint32 x : group.clusters)
        {
            packed_data.push_back(x);
        }
        i++;
    }

    std::vector<float3> instance_data;
    for (Uint32 i = 0; i < 15; i++)
    {
        for (Uint32 j = 0; j < 15; j++)
        {
            for (Uint32 k = 0; k < 4; k++)
                instance_data.push_back({i * 5.0f, k * 7.0f, j * 5.0f});
        }
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

    BufferDesc BuffDescIns;
    BuffDescIns.Name              = "cluster buffer";
    BuffDescIns.Usage             = USAGE_IMMUTABLE;
    BuffDescIns.BindFlags         = BIND_SHADER_RESOURCE;
    BuffDescIns.Mode              = BUFFER_MODE_FORMATTED;
    BuffDescIns.CPUAccessFlags    = CPU_ACCESS_NONE;
    BuffDescIns.ElementByteStride = sizeof(Uint32);
    BuffDescIns.Size              = sizeof(float3) * instance_data.size();
    BufferData InsData;
    InsData.pData    = instance_data.data();
    InsData.DataSize = sizeof(float3) * instance_data.size();
    m_pDevice->CreateBuffer(BuffDescIns, &InsData, &m_Buffers["instance"]);

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

    RefCntAutoPtr<IBufferView> pDataSRV, pInsSRV, pVisSRV, pVisUAV, pArgUAV;
    BufferViewDesc             ViewDesc;
    ViewDesc.ViewType             = BUFFER_VIEW_SHADER_RESOURCE;
    ViewDesc.Format.ValueType     = VT_UINT32;
    ViewDesc.Format.NumComponents = 1;
    m_Buffers["cluster"]->CreateView(ViewDesc, &pDataSRV);
    m_Buffers["instance"]->CreateView(ViewDesc, &pInsSRV);
    m_Buffers["visibility"]->CreateView(ViewDesc, &pVisSRV);
    ViewDesc.ViewType = BUFFER_VIEW_UNORDERED_ACCESS;
    m_Buffers["arg"]->CreateView(ViewDesc, &pArgUAV);
    m_Buffers["visibility"]->CreateView(ViewDesc, &pVisUAV);

    {
        Uint32 hzb_mip_levels = high_bit(1024) + 1;

        TextureDesc DepthTexDesc;
        DepthTexDesc.Name      = "depth Tex";
        DepthTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        DepthTexDesc.Width     = mWidth;
        DepthTexDesc.Height    = mHeight;
        DepthTexDesc.Format    = TEX_FORMAT_R32_TYPELESS;
        DepthTexDesc.Usage     = USAGE_DEFAULT;
        DepthTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(DepthTexDesc, nullptr, &m_TexDepth);

        TextureDesc TexDesc;
        TexDesc.Name      = "hzb Tex";
        TexDesc.Type      = RESOURCE_DIM_TEX_2D;
        TexDesc.Width     = 1024;
        TexDesc.Height    = 1024;
        TexDesc.MipLevels = hzb_mip_levels;
        TexDesc.Format    = TEX_FORMAT_R32_TYPELESS;
        TexDesc.Usage     = USAGE_DEFAULT;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(TexDesc, nullptr, &m_TexHzb);
        m_TextureHzbUAV = m_TexHzb->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);

        m_TextureHzbRTVs.resize(hzb_mip_levels + 1);
        for (int i = 0; i < hzb_mip_levels; ++i)
        {
            TextureViewDesc ViewDesc;
            ViewDesc.TextureDim      = RESOURCE_DIM_TEX_2D;
            ViewDesc.ViewType        = TEXTURE_VIEW_DEPTH_STENCIL;
            ViewDesc.AccessFlags     = UAV_ACCESS_FLAG_READ | UAV_ACCESS_FLAG_WRITE;
            ViewDesc.Format          = TEX_FORMAT_D32_FLOAT;
            if (i == 0)
            {
                ViewDesc.MostDetailedMip = 0;
                m_TexDepth->CreateView(ViewDesc, &m_TextureHzbRTVs[i]);
            }
            else
            {
                ViewDesc.MostDetailedMip = i-1;
                m_TexHzb->CreateView(ViewDesc, &m_TextureHzbRTVs[i]);
            }
        }
    }

    m_pPSOCull->CreateShaderResourceBinding(&m_SRBCull, true);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "data")->Set(pDataSRV);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "instance")->Set(pInsSRV);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "visibility")->Set(pVisUAV);
    m_SRBCull->GetVariableByName(SHADER_TYPE_COMPUTE, "arg")->Set(pArgUAV);

    m_pPSOGraphic->CreateShaderResourceBinding(&m_SRBGraphic, true);
    m_SRBGraphic->GetVariableByName(SHADER_TYPE_VERTEX, "data")->Set(pDataSRV);
    m_SRBGraphic->GetVariableByName(SHADER_TYPE_VERTEX, "instance")->Set(pInsSRV);
    m_SRBGraphic->GetVariableByName(SHADER_TYPE_VERTEX, "visibility")->Set(pVisSRV);

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
    BufferData QuadData;
    QuadData.pData  = gScreenQuadPoints;
    QuadData.DataSize = sizeof(gScreenQuadPoints);
    m_pDevice->CreateBuffer(VertBuffDesc, &QuadData, &m_Buffers["ScreenQuad"]);

    m_pPSOHzb->CreateShaderResourceBinding(&m_SRBHzb, true);
}

void Tutorial32_VirtualGeomtry3::CreateIndexBuffer()
{
    
}

void Tutorial32_VirtualGeomtry3::LoadTexture()
{
    
}


void Tutorial32_VirtualGeomtry3::Initialize(const SampleInitInfo& InitInfo)
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
void Tutorial32_VirtualGeomtry3::Render()
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
    auto pDSV = m_TextureHzbRTVs[0];
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

    #if 0
    for (u32 i = 0, cw = 1024, ch = 1024; i < hzb_mip_levels; i++, cw >>= 1, ch >>= 1)
    {
        //ITextureView*           pQuadRTVs[1] = {m_TextureViews[gStrRenderSceneBufferRTV]};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 0;
        //QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = m_TextureHzbRTVs[i];
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        //constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        //m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers["ScreenQuad"]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOHzb);
        //m_SRBHzb->GetVariableByName(SHADER_TYPE_PIXEL, "tex")->Set(pVisSRV);
        m_pImmediateContext->CommitShaderResources(m_SRBHzb, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }
    #endif
}

void Tutorial32_VirtualGeomtry3::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
    mframeCull.vp_mat  = view * proj;
    mframeCull.m_mat = float4x4::Scale(0.05f);
    mframeCull.v_mat    = view;
    mframeCull.p_mat    = proj;

    mframeGraphic.vp_mat = view * proj;
    mframeGraphic.m_mat  = float4x4::Scale(0.05f);
    mframeGraphic.v_mat  = view;
    mframeGraphic.p_mat  = proj;
}

void Tutorial32_VirtualGeomtry3::WindowResize(Uint32 Width, Uint32 Height)
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
