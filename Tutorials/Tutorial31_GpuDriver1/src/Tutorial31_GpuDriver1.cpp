
#include "Tutorial31_GpuDriver1.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

namespace Diligent
{

static float3 gCamPos{22.0f, 1.9f, 5.4f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const char* gStrModelSimple          = "gStrModelSimple";
static const char* gStrModelSimpleCBV       = "gStrModelSimpleCBV";
static const char* gStrModelSimpleCameraCBV = "gStrModelSimpleCameraCBV";

static const char* gStlModelVB        = "gStlModelVB";
static const char* gStlModelIB        = "gStlModelIB";

static const char* gStrModel          = "gStrModel";
static const char* gStrModelCBV       = "gStrModelCBV";
static const char* gStrModelCameraCBV = "gStrModelCameraCBV";

const char* gStlModelPath[] = {
    "Models/kitten.stl",
    //"Models/hulugai.stl",
    //"Models/hulushen.stl",
    //"Models/PikachuThick_tail.stl",
};

//"Models/biyunlong.stl",
//"Models/kitten.stl",

const float4 gColorModel[] = {
    float4(0.99f, 0.88f, 0.08f, 1.0f), // long
    float4(0.59f, 0.67f, 0.66f, 1.0f), // hulugai
    float4(0.36f, 0.42f, 0.04f, 1.0f), // hulushen
    //float4(0.57f, 0.32f, 0.04f, 1.0f), // pikachu
};

SampleBase* CreateSample()
{
    return new Tutorial31_GpuDriver1();
}

void Tutorial31_GpuDriver1::InitConstData()
{
    Uint32 idx = 0;
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            float4x4 mat = float4x4::Scale(0.05f);//  * float4x4::Translation(float3(i * 4.0f, 0.0f, j * 4.0f));
            mModelPassConstants.push_back({mat, gColorModel[idx++]});
        }
    }
}

void Tutorial31_GpuDriver1::CreatePipelineStateSimple()
{
    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    PSOCreateInfo.PSODesc.Name         = gStrModelSimple;
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

    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "modelSimple VS";
        ShaderCI.FilePath        = "modelSimple.vsh";
        m_pDevice->CreateShader(ShaderCI, &pVS);

        CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "object constants CB", &m_Buffers[gStrModelSimpleCBV]);
        CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "camera constants CB", &m_Buffers[gStrModelSimpleCameraCBV]);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "modelSimple PS";
        ShaderCI.FilePath        = "modelSimple.psh";
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    LayoutElement LayoutElems[] =
    {
        LayoutElement{0, 0, 3, VT_FLOAT32, False},  // pos
    };
    // clang-format on

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrModelSimple]);

    m_pPSOs[gStrModelSimple]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrModelSimpleCBV]);
    m_pPSOs[gStrModelSimple]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrModelSimpleCameraCBV]);

    m_pPSOs[gStrModelSimple]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrModelSimple], true);
}

void Tutorial31_GpuDriver1::CreatePipelineState()
{
    GraphicsPipelineStateCreateInfo PSOCreateInfo;

    PSOCreateInfo.PSODesc.Name         = gStrModel;
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

    ShaderCI.Desc.UseCombinedTextureSamplers = true;

    // Create a shader source stream factory to load shaders from files.
    RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
    m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
    ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
    // Create a vertex shader
    RefCntAutoPtr<IShader> pVS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "model VS";
        ShaderCI.FilePath        = "model.vsh";
        m_pDevice->CreateShader(ShaderCI, &pVS);

        CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "object constants CB", &m_Buffers[gStrModelCBV]);
        CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "camera constants CB", &m_Buffers[gStrModelCameraCBV]);
    }

    // Create a pixel shader
    RefCntAutoPtr<IShader> pPS;
    {
        ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
        ShaderCI.EntryPoint      = "main";
        ShaderCI.Desc.Name       = "model PS";
        ShaderCI.FilePath        = "model.psh";
        m_pDevice->CreateShader(ShaderCI, &pPS);
    }

    // clang-format off
    LayoutElement LayoutElems[] =
    {
        LayoutElement{0, 0, 3, VT_FLOAT32, False},  // pos
        LayoutElement{1, 0, 3, VT_FLOAT32, False},  // normal
        LayoutElement{2, 0, 2, VT_FLOAT32, False},  // uv
    };
    // clang-format on

    PSOCreateInfo.pVS = pVS;
    PSOCreateInfo.pPS = pPS;

    PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
    PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

    // Define variable type that will be used by default
    PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

    // clang-format off
    ShaderResourceVariableDesc Vars[] = {
        {SHADER_TYPE_PIXEL, "g_Texture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
    PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

    // clang-format off
    // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
    SamplerDesc SamLinearClampDesc
    {
        FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, 
        TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP, TEXTURE_ADDRESS_CLAMP
    };
    ImmutableSamplerDesc ImtblSamplers[] = 
    {
        {SHADER_TYPE_PIXEL, "g_Texture", SamLinearClampDesc}
    };
    // clang-format on
    PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
    PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

    m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrModel]);

    m_pPSOs[gStrModel]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrModelCBV]);
    m_pPSOs[gStrModel]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrModelCameraCBV]);

    m_pPSOs[gStrModel]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrModel], true);
}

void Tutorial31_GpuDriver1::CreateBuffer()
{
    for (int i = 0; i < mStlModel.size(); ++i)
    {
        BufferDesc VertBuffDesc;
        VertBuffDesc.Name      = "st1 vertex buffer";
        VertBuffDesc.Usage     = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size      = mStlModel[i].vtx.size() * sizeof(Vertex);
        BufferData VBData;
        VBData.pData    = mStlModel[i].vtx.data();
        VBData.DataSize = VertBuffDesc.Size;
        m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStlModelVB + std::to_string(i)]);
    }
    
    for (int i = 0; i < mStlModel.size(); ++i)
    {
        BufferDesc IndBuffDesc;
        IndBuffDesc.Name      = "stl index buffer";
        IndBuffDesc.Usage     = USAGE_IMMUTABLE;
        IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
        IndBuffDesc.Size      = mStlModel[i].tris.size() * sizeof(Uint32);
        BufferData IBData;
        IBData.pData    = mStlModel[i].tris.data();
        IBData.DataSize = IndBuffDesc.Size;
        m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_Buffers[gStlModelIB + std::to_string(i)]);
    }
}

void Tutorial31_GpuDriver1::LoadModel()
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
    ModelCI.FileName             = "Models/3dpea.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model                      = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);

    mStlModel.resize(_countof(gStlModelPath));
    std::vector<std::thread> threads;
    for (int i = 0; i < mStlModel.size(); ++i)
    {
        threads.push_back(std::thread(&Tutorial31_GpuDriver1::ThreadLoadModel, this, i));
    }

    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }
}

void Tutorial31_GpuDriver1::ThreadLoadModel(int idx)
{
    mStlModel[idx].readSTL_Binary(std::string(gStlModelPath[idx]));
}

void Tutorial31_GpuDriver1::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    InitConstData();
    CreatePipelineStateSimple();
    CreatePipelineState();
    LoadModel();
    CreateBuffer();
}

// Render a frame
void Tutorial31_GpuDriver1::Render()
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

    RernderStl();
    //RenderGltf();
}

void Tutorial31_GpuDriver1::RernderStl()
{
    {
        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrModelSimpleCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = mPassCbData;
    }

    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrModelSimple]);
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrModelSimple], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

    for (int i = 0; i < mStlModel.size(); ++i)
    {
        {
            MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrModelSimpleCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mModelPassConstants[i];
        }
        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStlModelVB + std::to_string(i)]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pImmediateContext->SetIndexBuffer(m_Buffers[gStlModelIB + std::to_string(i)], 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs drawAttrsColumn{(Uint32)mStlModel[i].tris.size(), VT_UINT32, DRAW_FLAG_VERIFY_ALL};
        drawAttrsColumn.FirstIndexLocation = 0;
        drawAttrsColumn.BaseVertex         = 0;
        m_pImmediateContext->DrawIndexed(drawAttrsColumn);
    }
}

void Tutorial31_GpuDriver1::RenderGltf()
{
    {
        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrModelCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
        *CBConstants = mPassCbData;
    }

    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrModel]);
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrModel], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

    const auto FirstIndexLocation = m_Model->GetFirstIndexLocation();
    const auto BaseVertex         = m_Model->GetBaseVertex();
    for (int i = 0; i < mModelPassConstants.size(); ++i)
    {
        {
            MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrModelCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mModelPassConstants[i];
        }
        for (const auto* pNode : m_Model->Scenes[m_Model->DefaultSceneId].LinearNodes)
        {
            if (!pNode->pMesh)
                continue;
            for (const auto& primitive : pNode->pMesh->Primitives)
            {
                m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrModel], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

void Tutorial31_GpuDriver1::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }

    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();

    mPassCbData.PreViewProj = mPassCbData.ViewProj;
    mPassCbData.View        = view;
    mPassCbData.Proj        = proj;
    mPassCbData.ViewProj    = view * proj;
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
}

void Tutorial31_GpuDriver1::WindowResize(Uint32 Width, Uint32 Height)
{
    mWidth = Width;
    mHeight = Height;

    float NearPlane   = gCamearNear;
    float FarPlane    = gCamearFar;
    float AspectRatio = static_cast<float>(Width) / static_cast<float>(Height);
    m_Camera.SetProjAttribs(NearPlane, FarPlane, AspectRatio, PI_F / 4.f,
                            m_pSwapChain->GetDesc().PreTransform, m_pDevice->GetDeviceInfo().IsGLDevice());
}

} // namespace Diligent
