
#include "Tutorial34_SH.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

#include <fstream>

namespace Diligent
{
static Diligent::float3 gCamPos{2.0f, 0.2f, 0.9f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 100.0f;

static const char* gStrSkyBox   = "SkyBox";
static const char* gStrSH       = "SH";
static const char* gStrSkyBoxCB = "SkyBoxCB";
static const char* gStrCameraCB = "gStrCameraCB";
static const char* gStrSHCB     = "gStrSHCB";

static const char* gStrSkyBoxVB = "SkyBoxVB";
static const char* gStrSkyBoxIB = "SkyBoxIB";

static const char* gStrBRDF = "gStrBRDF";
static const char* gStrTex1 = "gStrTex1";
static const char* gStrTex2 = "gStrTex2";
static const char* gStrTex3 = "gStrTex3";

SampleBase* CreateSample()
{
    return new Tutorial34_SH();
}

void Tutorial34_SH::CreatePipelineState()
{
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name = gStrSkyBox;

        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc   = COMPARISON_FUNC_LESS_EQUAL;   // skybox

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "SkyBox VS";
            ShaderCI.FilePath        = "SkyBox.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(SkyBoxConstants), "VS constants CB", &m_Buffers[gStrSkyBoxCB]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "SkyBox PS";
            ShaderCI.FilePath        = "SkyBox.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            LayoutElement{1, 0, 2, VT_FLOAT32, False}
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        // Shader variables should typically be mutable, which means they are expected
        // to change on a per-instance basis
        ShaderResourceVariableDesc Vars[] = 
        {
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

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrSkyBox]);

        m_pPSOs[gStrSkyBox]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_Buffers[gStrSkyBoxCB]);

        m_pPSOs[gStrSkyBox]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSkyBox], true);
    }

    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name = gStrSH;

        PSOCreateInfo.PSODesc.PipelineType = PIPELINE_TYPE_GRAPHICS;

        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = True;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthFunc   = COMPARISON_FUNC_LESS_EQUAL; // skybox

        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage = SHADER_SOURCE_LANGUAGE_HLSL;

        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;
        // Create a vertex shader
        RefCntAutoPtr<IShader> pVS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_VERTEX;
            ShaderCI.EntryPoint      = "VS";
            ShaderCI.Desc.Name       = "SH VS";
            ShaderCI.FilePath        = "SH.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(SkyBoxConstants), "VS constants CB", &m_Buffers[gStrCameraCB]);
            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "ObjectConstants constants CB", &m_Buffers[gStrSHCB]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "SH PS";
            ShaderCI.FilePath        = "SH.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
            LayoutElement{2, 0, 3, VT_FLOAT32, False}
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        // Shader variables should typically be mutable, which means they are expected
        // to change on a per-instance basis
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "BRDFLut", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "Texture1", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "Texture2", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "Texture3", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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
            {SHADER_TYPE_PIXEL, "BRDFLut", SamLinearClampDesc},
            {SHADER_TYPE_PIXEL, "Texture1", SamLinearClampDesc},
            {SHADER_TYPE_PIXEL, "Texture2", SamLinearClampDesc},
            {SHADER_TYPE_PIXEL, "Texture3", SamLinearClampDesc},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrSH]);

        m_pPSOs[gStrSH]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "Constants")->Set(m_Buffers[gStrCameraCB]);
        m_pPSOs[gStrSH]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "ObjectConstants")->Set(m_Buffers[gStrSHCB]);
        m_pPSOs[gStrSH]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "ObjectConstants")->Set(m_Buffers[gStrSHCB]);

        m_pPSOs[gStrSH]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSH], true);
    }
}

void Tutorial34_SH::CreateVertexBuffer()
{
    // Layout of this structure matches the one we defined in the pipeline state
    struct Vertex
    {
        float3 pos;
        float2 uv;
    };

    // Cube vertices

    //      (-1,+1,+1)________________(+1,+1,+1)
    //               /|              /|
    //              / |             / |
    //             /  |            /  |
    //            /   |           /   |
    //(-1,-1,+1) /____|__________/(+1,-1,+1)
    //           |    |__________|____|
    //           |   /(-1,+1,-1) |    /(+1,+1,-1)
    //           |  /            |   /
    //           | /             |  /
    //           |/              | /
    //           /_______________|/
    //        (-1,-1,-1)       (+1,-1,-1)
    //

    // This time we have to duplicate verices because texture coordinates cannot
    // be shared
    constexpr Vertex CubeVerts[] =
        {
            {float3{-1, -1, -1}, float2{0, 1}},
            {float3{-1, +1, -1}, float2{0, 0}},
            {float3{+1, +1, -1}, float2{1, 0}},
            {float3{+1, -1, -1}, float2{1, 1}},

            {float3{-1, -1, -1}, float2{0, 1}},
            {float3{-1, -1, +1}, float2{0, 0}},
            {float3{+1, -1, +1}, float2{1, 0}},
            {float3{+1, -1, -1}, float2{1, 1}},

            {float3{+1, -1, -1}, float2{0, 1}},
            {float3{+1, -1, +1}, float2{1, 1}},
            {float3{+1, +1, +1}, float2{1, 0}},
            {float3{+1, +1, -1}, float2{0, 0}},

            {float3{+1, +1, -1}, float2{0, 1}},
            {float3{+1, +1, +1}, float2{0, 0}},
            {float3{-1, +1, +1}, float2{1, 0}},
            {float3{-1, +1, -1}, float2{1, 1}},

            {float3{-1, +1, -1}, float2{1, 0}},
            {float3{-1, +1, +1}, float2{0, 0}},
            {float3{-1, -1, +1}, float2{0, 1}},
            {float3{-1, -1, -1}, float2{1, 1}},

            {float3{-1, -1, +1}, float2{1, 1}},
            {float3{+1, -1, +1}, float2{0, 1}},
            {float3{+1, +1, +1}, float2{0, 0}},
            {float3{-1, +1, +1}, float2{1, 0}},
        };

    BufferDesc VertBuffDesc;
    VertBuffDesc.Name      = "Cube vertex buffer";
    VertBuffDesc.Usage     = USAGE_IMMUTABLE;
    VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
    VertBuffDesc.Size      = sizeof(CubeVerts);
    BufferData VBData;
    VBData.pData    = CubeVerts;
    VBData.DataSize = sizeof(CubeVerts);
    m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrSkyBoxVB]);
}

void Tutorial34_SH::CreateIndexBuffer()
{
    // clang-format off
    constexpr Uint32 Indices[] =
    {
        2,0,1,    2,3,0,
        4,6,5,    4,7,6,
        8,10,9,   8,11,10,
        12,14,13, 12,15,14,
        16,18,17, 16,19,18,
        20,21,22, 20,22,23
    };
    // clang-format on

    BufferDesc IndBuffDesc;
    IndBuffDesc.Name      = "Cube index buffer";
    IndBuffDesc.Usage     = USAGE_IMMUTABLE;
    IndBuffDesc.BindFlags = BIND_INDEX_BUFFER;
    IndBuffDesc.Size      = sizeof(Indices);
    BufferData IBData;
    IBData.pData    = Indices;
    IBData.DataSize = sizeof(Indices);
    m_pDevice->CreateBuffer(IndBuffDesc, &IBData, &m_Buffers[gStrSkyBoxIB]);
}


void Tutorial34_SH::LoadTexture()
{
    {
        TextureLoadInfo loadInfo;
        loadInfo.IsSRGB = true;
        RefCntAutoPtr<ITexture> Tex;
        CreateTextureFromFile("cubemap.dds", loadInfo, m_pDevice, &Tex);
        m_TextureViews[gStrSkyBox] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureLoadInfo         loadInfo;
        RefCntAutoPtr<ITexture> Tex;
        CreateTextureFromFile("BRDFLut.dds", loadInfo, m_pDevice, &Tex);
        m_TextureViews[gStrBRDF] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureLoadInfo         loadInfo;
        RefCntAutoPtr<ITexture> Tex;
        CreateTextureFromFile("Tex1.dds", loadInfo, m_pDevice, &Tex);
        m_TextureViews[gStrTex1] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureLoadInfo         loadInfo;
        RefCntAutoPtr<ITexture> Tex;
        CreateTextureFromFile("Tex2.dds", loadInfo, m_pDevice, &Tex);
        m_TextureViews[gStrTex2] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureLoadInfo         loadInfo;
        RefCntAutoPtr<ITexture> Tex;
        CreateTextureFromFile("Tex3.dds", loadInfo, m_pDevice, &Tex);
        m_TextureViews[gStrTex3] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    m_ShaderResourceBindings[gStrSkyBox]->GetVariableByName(SHADER_TYPE_PIXEL, "g_Texture")->Set(m_TextureViews[gStrSkyBox]);
    m_ShaderResourceBindings[gStrSH]->GetVariableByName(SHADER_TYPE_PIXEL, "BRDFLut")->Set(m_TextureViews[gStrBRDF]);
    m_ShaderResourceBindings[gStrSH]->GetVariableByName(SHADER_TYPE_PIXEL, "Texture1")->Set(m_TextureViews[gStrTex1]);
    m_ShaderResourceBindings[gStrSH]->GetVariableByName(SHADER_TYPE_PIXEL, "Texture2")->Set(m_TextureViews[gStrTex2]);
    m_ShaderResourceBindings[gStrSH]->GetVariableByName(SHADER_TYPE_PIXEL, "Texture3")->Set(m_TextureViews[gStrTex3]);
}


void Tutorial34_SH::LoadModel()
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
    ModelCI.FileName             = "dragon.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model                      = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);
}

void Tutorial34_SH::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    {
        std::string   path = "coefficients.txt";
        std::ifstream ifs(path);
        if (!ifs)
            throw std::runtime_error("open " + path + " failed");
        int   i = 0;
        float r, g, b;
        while (ifs >> r >> g >> b)
        {
            mObjectCB.Coef[i++] = float4(r, g, b, 0.0f);
        } 
    }

    CreatePipelineState();
    CreateVertexBuffer();
    CreateIndexBuffer();
    LoadTexture();
    LoadModel();
}

// Render a frame
void Tutorial34_SH::Render()
{
    if (true)
    {
        ITextureView* pRTVs[1] = {m_pSwapChain->GetCurrentBackBufferRTV()};
        auto*         pDSV     = m_pSwapChain->GetDepthBufferDSV();

        SetRenderTargetsAttribs RTAttrs;
        RTAttrs.NumRenderTargets    = 1;
        RTAttrs.ppRenderTargets     = pRTVs;
        RTAttrs.pDepthStencil       = pDSV;
        RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
        m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

        // Clear the back buffer
        float4 ClearColor = {0.350f, 0.350f, 0.350f, 1.0f};
        if (m_ConvertPSOutputToGamma)
        {
            // If manual gamma correction is required, we need to clear the render target with sRGB color
            ClearColor = LinearToSRGB(ClearColor);
        }
        m_pImmediateContext->ClearRenderTarget(pRTVs[0], ClearColor.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSkyBox]);

        {
            MapHelper<SkyBoxConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrSkyBoxCB], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mSkyBoxPassCB;
        }

        const Uint64 offset   = 0;
        IBuffer* pBuffs[] = {m_Buffers[gStrSkyBoxVB]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
        m_pImmediateContext->SetIndexBuffer(m_Buffers[gStrSkyBoxIB], 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSkyBox], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
        DrawAttrs.IndexType  = VT_UINT32; // Index type
        DrawAttrs.NumIndices = 36; 
        DrawAttrs.Flags = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->DrawIndexed(DrawAttrs);
    }

    if (true)
    {
        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSH]);

        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSH], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        std::array<GLTF::Model*, 1> models = {m_Model};
        for (const auto* model : models)
        {
            std::vector<IBuffer*> pVBs;
            const auto            NumVBs = static_cast<Uint32>(model->GetVertexBufferCount());
            pVBs.resize(NumVBs);
            for (Uint32 i = 0; i < NumVBs; ++i)
                pVBs[i] = model->GetVertexBuffer(i);
            m_pImmediateContext->SetVertexBuffers(0, NumVBs, pVBs.data(), nullptr, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

            if (auto* pIndexBuffer = model->GetIndexBuffer())
            {
                m_pImmediateContext->SetIndexBuffer(pIndexBuffer, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            }

            const auto FirstIndexLocation = model->GetFirstIndexLocation();
            const auto BaseVertex         = model->GetBaseVertex();
            for (const auto* pNode : model->Scenes[model->DefaultSceneId].LinearNodes)
            {
                if (!pNode->pMesh)
                    continue;
                for (const auto& primitive : pNode->pMesh->Primitives)
                {
                    {
                        MapHelper<SkyBoxConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrCameraCB], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mSkyBoxPassCB;
                    }
                    {
                        MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrSHCB], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mObjectCB;
                    }

                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSH], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

                    if (primitive.HasIndices())
                    {
                        DrawIndexedAttribs drawAttrs{primitive.IndexCount, VT_UINT32, DRAW_FLAG_VERIFY_ALL};
                        drawAttrs.FirstIndexLocation = FirstIndexLocation + primitive.FirstIndex;
                        drawAttrs.BaseVertex         = BaseVertex;
                        m_pImmediateContext->DrawIndexed(drawAttrs);
                    }
                    else
                    {
                        DrawAttribs drawAttrs{primitive.VertexCount, DRAW_FLAG_VERIFY_ALL};
                        drawAttrs.StartVertexLocation = BaseVertex;
                        m_pImmediateContext->Draw(drawAttrs);
                    }
                }
            }
        }
    }
}

void Tutorial34_SH::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();

    mSkyBoxPassCB.View = view;
    mSkyBoxPassCB.Proj = proj;
    mSkyBoxPassCB.gEyePosW = float4(m_Camera.GetPos(), 1.0);

    mObjectCB.World = float4x4::Identity();
}

void Tutorial34_SH::WindowResize(Uint32 Width, Uint32 Height)
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
