
#include "Tutorial31_GpuDriver2.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

namespace Diligent
{

static float3 gCamPos{22.0f, 1.9f, 5.4f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;


#define SAN_MIGUEL_OFFSETX 150.f
#define MESH_COUNT         1
#define MESH_SCALE         10.f

#define ASM_SUN_SPEED 0.001f

static const char* gStrModelSimple          = "gStrModelSimple";
static const char* gStrModelSimpleCBV       = "gStrModelSimpleCBV";
static const char* gStrModelSimpleCameraCBV = "gStrModelSimpleCameraCBV";

static const char* gStlModelVB        = "gStlModelVB";
static const char* gStlModelIB        = "gStlModelIB";

static const char* gStrModel          = "gStrModel";
static const char* gStrModelCBV       = "gStrModelCBV";
static const char* gStrModelCameraCBV = "gStrModelCameraCBV";

static const char* gStrClearVisibilityBuffers = "ClearVisibilityBuffers";
static const char* gStrTriangleFiltering      = "triangleFiltering";
static const char* gStrBatchCompaction        = "batchCompaction";

static const char* gStrVisibilityBufferConstants = "VisibilityBufferConstants";
static const char* gStrBatchDataConstants        = "batchData_rootcbv";
static const char* gStrArg                       = "arg";

struct
{
    float mSourceAngle = 1.0f;
    //only used for ESM shadow
    //float2 mSunControl = { -2.1f, -0.213f };
    float2 mSunControl = {-2.1f, -0.961f};
    float  mSunSpeedY  = 0.025f;
    //only for SDF shadow now
    bool mAutomaticSunMovement = false;
} gLightCpuSettings;

struct
{
    bool mHoldFilteredTriangles = false;
    bool mIsGeneratingSDF       = false;

    uint     mMsaaLevel          = 1;
    uint32_t mMsaaIndex          = (uint32_t)log2((uint32_t)mMsaaLevel);
    uint32_t mMsaaIndexRequested = mMsaaIndex;
} gAppSettings;

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

static float4x4 orthographic(float left, float right, float bottom, float top, float zNear, float zFar)
{
    // LH - DirectX: Z -> [0, 1]
    return float4x4(
        2.0f / (right - left), 0, 0, 0,
        0.0, 2.0f / (top - bottom), 0, 0,
        0, 0, 1.0f / (zFar - zNear), 0,
        -(right + left) / (right - left), -(top + bottom) / (top - bottom), -zNear / (zFar - zNear), 1);
}

SampleBase* CreateSample()
{
    return new Tutorial31_GpuDriver2();
}

void Tutorial31_GpuDriver2::InitConstData()
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

void Tutorial31_GpuDriver2::CreatePipelineState()
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

        // clang-format off
        ShaderResourceVariableDesc ClearVars[] = {
            {SHADER_TYPE_COMPUTE, "uncompactedDrawArgsRW", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferAlpha", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferNoAlpha", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = ClearVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(ClearVars);

        PSODesc.Name      = gStrClearVisibilityBuffers;
        PSOCreateInfo.pCS = pclearVisibilityBuffersCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrClearVisibilityBuffers]);
    }

    {
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> triangleFiltering;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            //ShaderCI.HLSLVersion     = ShaderVersion(6, 1);
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name  = "triangleFiltering CS";
            ShaderCI.FilePath   = "triangleFiltering.hlsl";
            m_pDevice->CreateShader(ShaderCI, &triangleFiltering);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        // clang-format off
        PSODesc.Name                            = gStrTriangleFiltering;
        ShaderResourceVariableDesc Vars[] = {
            {SHADER_TYPE_COMPUTE, "batchData_rootcbv", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "visibilityBufferConstants", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "meshConstantsBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "indexDataBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "vertexDataBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "uncompactedDrawArgsRW", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "filteredIndicesBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        PSOCreateInfo.pCS                                 = triangleFiltering;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrTriangleFiltering]);
    }

    {
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> batchCompaction;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            //ShaderCI.HLSLVersion     = ShaderVersion(6, 1);
            ShaderCI.EntryPoint = "main";
            ShaderCI.Desc.Name  = "batchCompaction CS";
            ShaderCI.FilePath   = "batchCompaction.hlsl";
            m_pDevice->CreateShader(ShaderCI, &batchCompaction);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        // clang-format off
        PSODesc.Name                            = gStrBatchCompaction;
        ShaderResourceVariableDesc Vars[] = {
            {SHADER_TYPE_COMPUTE, "uncompactedDrawArgs", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "materialProps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferAlpha", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferNoAlpha", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "indirectMaterialBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        PSOCreateInfo.pCS                                 = batchCompaction;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrBatchCompaction]);
        VERIFY_EXPR(m_pPSOs[gStrBatchCompaction] != nullptr);
    }
}

void Tutorial31_GpuDriver2::LoadModel()
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

    mMeshCount = _countof(gStlModelPath);
    mMaterialCount = mMeshCount;
    mScene.mStlModel.resize(mMeshCount);
    std::vector<std::thread> threads;
    for (int i = 0; i < mScene.mStlModel.size(); ++i)
    {
        threads.push_back(std::thread(&Tutorial31_GpuDriver2::ThreadLoadModel, this, i));
    }

    for (int i = 0; i < threads.size(); ++i)
    {
        threads[i].join();
    }

    mScene.mMeshes.resize(mMeshCount);
    for (Uint32 i = 0; i < mMeshCount; i++)
    {
        ClusterContainer* subMesh      = &mScene.mMeshes[i];

        IndirectDrawIndexArguments indirectdraw;
        indirectdraw.mIndexCount    = (Uint32)mScene.mStlModel[i].tris.size();
        indirectdraw.mInstanceCount = 1;
        indirectdraw.mStartIndex    = 0;
        indirectdraw.mVertexOffset  = 0;
        indirectdraw.mStartInstance = 0;

        createClusters(mScene.mStlModel[i], &indirectdraw, subMesh);
    }

    std::vector<MeshConstants> meshConstants(mMeshCount);
    for (uint32_t i = 0; i < mMeshCount; ++i)
    {
        meshConstants[i].faceCount   = (Uint32)mScene.mStlModel[i].tris.size();
        meshConstants[i].indexOffset = 0;
        meshConstants[i].materialID  = i;
        meshConstants[i].twoSided    = 0;
        meshConstants[i].worldMatrix = mModelPassConstants[i].World;
    }
}

void Tutorial31_GpuDriver2::CreateBuffer()
{
    { // mBufferMeshConstants
        std::vector<MeshConstants> meshConstants(mMeshCount);
        for (uint32_t i = 0; i < mMeshCount; ++i)
        {
            meshConstants[i].faceCount   = (Uint32)mScene.mStlModel[0].tris.size() / 3;
            meshConstants[i].indexOffset = 0;
            meshConstants[i].materialID  = i;
            meshConstants[i].twoSided    = 0;
        }

        //StructuredBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name              = "mesh constants buffer";
        BuffDesc.Usage             = USAGE_DEFAULT;
        BuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        BuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
        BuffDesc.ElementByteStride = sizeof(MeshConstants);
        BuffDesc.Size              = sizeof(MeshConstants) * meshConstants.size();

        BufferData VBData;
        VBData.pData    = meshConstants.data();
        VBData.DataSize = sizeof(MeshConstants) * static_cast<Uint32>(meshConstants.size());
        m_pDevice->CreateBuffer(BuffDesc, &VBData, &mBufferMeshConstants);
        VERIFY_EXPR(mBufferMeshConstants != nullptr);
    }
    { // mBufferMaterialProps
        //StructuredBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name              = "materialProps buffer";
        BuffDesc.Usage             = USAGE_DEFAULT;
        BuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        BuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
        BuffDesc.ElementByteStride = sizeof(uint);
        BuffDesc.Size              = sizeof(uint) * mMaterialCount;

        m_pDevice->CreateBuffer(BuffDesc, nullptr, &mBufferMaterialProps);
        VERIFY_EXPR(mBufferMaterialProps != nullptr);
    }

    {
        //RWByteAddressBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name      = "vertex buffer";
        BuffDesc.Usage     = USAGE_DEFAULT;
        BuffDesc.BindFlags = BIND_SHADER_RESOURCE;
        BuffDesc.Mode      = BUFFER_MODE_RAW;
        BuffDesc.Size      = mScene.mStlModel[0].vtx.size() * sizeof(Vertex);

        BufferData VBData;
        VBData.pData    = mScene.mStlModel[0].vtx.data();
        VBData.DataSize = BuffDesc.Size;

        m_pDevice->CreateBuffer(BuffDesc, &VBData, &mBufferVertexData);
        VERIFY_EXPR(mBufferVertexData != nullptr);
    }

    {
        //RWByteAddressBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name      = "index buffer";
        BuffDesc.Usage     = USAGE_DEFAULT;
        BuffDesc.BindFlags = BIND_SHADER_RESOURCE;
        BuffDesc.Mode      = BUFFER_MODE_RAW;
        BuffDesc.Size      = mScene.mStlModel[0].tris.size() * sizeof(Uint32);

        BufferData VBData;
        VBData.pData    = mScene.mStlModel[0].tris.data();
        VBData.DataSize = BuffDesc.Size;

        m_pDevice->CreateBuffer(BuffDesc, &VBData, &mBufferIndexData);
        VERIFY_EXPR(mBufferIndexData != nullptr);
    }

    {
        //RWByteAddressBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name      = "indirectMaterial buffer";
        BuffDesc.Usage     = USAGE_DEFAULT;
        BuffDesc.BindFlags = BIND_UNORDERED_ACCESS;
        BuffDesc.Mode      = BUFFER_MODE_RAW;
        BuffDesc.Size      = sizeof(uint) * MATERIAL_BUFFER_SIZE;

        m_pDevice->CreateBuffer(BuffDesc, nullptr, &mBufferFilterIndirectMaterial);
        VERIFY_EXPR(mBufferFilterIndirectMaterial != nullptr);
    }

    {
        for (int i = 0; i < NUM_CULLING_VIEWPORTS; ++i)
        {
            //RWByteAddressBuffer
            BufferDesc BuffDesc;
            BuffDesc.Name      = "Filtered Index buffer";
            BuffDesc.Usage     = USAGE_DEFAULT;
            BuffDesc.BindFlags = BIND_UNORDERED_ACCESS;
            BuffDesc.Mode      = BUFFER_MODE_RAW;
            BuffDesc.Size      = mScene.mStlModel[0].tris.size() * sizeof(Uint32);
            m_pDevice->CreateBuffer(BuffDesc, nullptr, &mBufferFilteredIndex[i]);

            BufferDesc StructBuffDesc;
            StructBuffDesc.Name              = "uncompactedDrawArgs buffer";
            StructBuffDesc.Usage             = USAGE_DEFAULT;
            StructBuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
            StructBuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
            StructBuffDesc.ElementByteStride = sizeof(UncompactedDrawArguments);
            StructBuffDesc.Size              = sizeof(UncompactedDrawArguments) * MAX_DRAWS_INDIRECT;
            m_pDevice->CreateBuffer(StructBuffDesc, nullptr, &mBufferUncompactedDrawArguments[i]);

            StructBuffDesc.Name              = "indirectDrawArgsBufferAlpha buffer";
            StructBuffDesc.ElementByteStride = sizeof(uint);
            StructBuffDesc.Size              = sizeof(uint) * MAX_DRAWS_INDIRECT * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS;
            m_pDevice->CreateBuffer(StructBuffDesc, nullptr, &mBufferFilteredIndirectDrawArguments[0][i]);

            StructBuffDesc.Name              = "indirectDrawArgsBufferNoAlpha buffer";
            StructBuffDesc.ElementByteStride = sizeof(uint);
            StructBuffDesc.Size              = sizeof(uint) * MAX_DRAWS_INDIRECT * INDIRECT_DRAW_ARGUMENTS_STRUCT_NUM_ELEMENTS;
            m_pDevice->CreateBuffer(StructBuffDesc, nullptr, &mBufferFilteredIndirectDrawArguments[1][i]);
        }
    }

    { // const buffer
        //visibilityBufferConstants
        BufferDesc BuffDesc;
        BuffDesc.Name           = gStrVisibilityBufferConstants;
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(VisibilityBufferConstants);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrVisibilityBufferConstants]);

        BufferDesc BuffBatchDesc;
        BuffBatchDesc.Name           = gStrBatchDataConstants;
        BuffBatchDesc.Usage          = USAGE_DYNAMIC;
        BuffBatchDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffBatchDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffBatchDesc.Size           = sizeof(FillBatchData);
        m_pDevice->CreateBuffer(BuffBatchDesc, nullptr, &m_Buffers[gStrBatchDataConstants]);
    }

    { // arg
        BufferDesc BuffDesc2;
        BuffDesc2.Name              = "arg buffer";
        BuffDesc2.Usage             = USAGE_DEFAULT;
        BuffDesc2.BindFlags         = BIND_INDIRECT_DRAW_ARGS | BIND_UNORDERED_ACCESS;
        BuffDesc2.Mode              = BUFFER_MODE_FORMATTED;
        BuffDesc2.CPUAccessFlags    = CPU_ACCESS_NONE;
        BuffDesc2.ElementByteStride = sizeof(Uint32);
        BuffDesc2.Size              = sizeof(Uint32) * MATERIAL_BUFFER_SIZE;
        m_pDevice->CreateBuffer(BuffDesc2, nullptr, &m_Buffers[gStrArg]);
        VERIFY_EXPR(m_Buffers[gStrArg] != nullptr);
    }

    {
        IBufferView* FilteredIndexRW[] = {
            mBufferFilteredIndex[0]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS),
            mBufferFilteredIndex[1]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS)};
        IBufferView* uncompactedSRV[] = {
            mBufferUncompactedDrawArguments[0]->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE),
            mBufferUncompactedDrawArguments[1]->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE)};
        IBufferView* uncompactedRW[] = {
            mBufferUncompactedDrawArguments[0]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS),
            mBufferUncompactedDrawArguments[1]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS)};
        IBufferView* Alpha[] = {
            mBufferFilteredIndirectDrawArguments[GEOMSET_OPAQUE][0]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS),
            mBufferFilteredIndirectDrawArguments[GEOMSET_OPAQUE][1]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS)};
        IBufferView* noAlpha[] = {
            mBufferFilteredIndirectDrawArguments[GEOMSET_ALPHATESTED][0]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS),
            mBufferFilteredIndirectDrawArguments[GEOMSET_ALPHATESTED][1]->GetDefaultView(BUFFER_VIEW_UNORDERED_ACCESS)};
        m_pPSOs[gStrClearVisibilityBuffers]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "uncompactedDrawArgsRW")->SetArray((IDeviceObject* const*)uncompactedRW, 0, 2);
        m_pPSOs[gStrClearVisibilityBuffers]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferAlpha")->SetArray((IDeviceObject* const*)Alpha, 0, 2);
        m_pPSOs[gStrClearVisibilityBuffers]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferNoAlpha")->SetArray((IDeviceObject* const*)noAlpha, 0, 2);
        m_pPSOs[gStrClearVisibilityBuffers]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrClearVisibilityBuffers], true);

        RefCntAutoPtr<IBufferView> pArgUAV;
        BufferViewDesc             ViewDesc;
        ViewDesc.ViewType             = BUFFER_VIEW_UNORDERED_ACCESS;
        ViewDesc.Format.ValueType     = VT_UINT32;
        ViewDesc.Format.NumComponents = 1;
        m_Buffers[gStrArg]->CreateView(ViewDesc, &pArgUAV);


        m_pPSOs[gStrTriangleFiltering]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrTriangleFiltering], true);
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "batchData_rootcbv")->Set(m_Buffers[gStrBatchDataConstants]);
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "visibilityBufferConstants")->Set(m_Buffers[gStrVisibilityBufferConstants]);
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "meshConstantsBuffer")->Set(mBufferMeshConstants->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "vertexDataBuffer")->Set(mBufferVertexData->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "indexDataBuffer")->Set(mBufferIndexData->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "uncompactedDrawArgsRW")->SetArray((IDeviceObject* const*)uncompactedRW, 0, 2);
        m_ShaderResourceBindings[gStrTriangleFiltering]->GetVariableByName(SHADER_TYPE_COMPUTE, "filteredIndicesBuffer")->SetArray((IDeviceObject* const*)FilteredIndexRW, 0, 2);

        m_pPSOs[gStrBatchCompaction]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrBatchCompaction], true);
        m_ShaderResourceBindings[gStrBatchCompaction]->GetVariableByName(SHADER_TYPE_COMPUTE, "uncompactedDrawArgs")->SetArray((IDeviceObject* const*)uncompactedSRV, 0, 2);
        m_ShaderResourceBindings[gStrBatchCompaction]->GetVariableByName(SHADER_TYPE_COMPUTE, "materialProps")->Set(mBufferMaterialProps->GetDefaultView(BUFFER_VIEW_SHADER_RESOURCE));
        m_ShaderResourceBindings[gStrBatchCompaction]->GetVariableByName(SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferAlpha")->SetArray((IDeviceObject* const*)Alpha, 0, 2);
        m_ShaderResourceBindings[gStrBatchCompaction]->GetVariableByName(SHADER_TYPE_COMPUTE, "indirectDrawArgsBufferNoAlpha")->SetArray((IDeviceObject* const*)noAlpha, 0, 2);
        m_ShaderResourceBindings[gStrBatchCompaction]->GetVariableByName(SHADER_TYPE_COMPUTE, "indirectMaterialBuffer")->Set(pArgUAV);
    }
}

void Tutorial31_GpuDriver2::ThreadLoadModel(int idx)
{
    mScene.mStlModel[idx].readSTL_Binary(std::string(gStlModelPath[idx]));
}

void Tutorial31_GpuDriver2::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    InitConstData();
    CreatePipelineState();
    LoadModel();
    CreateBuffer();
}

// Render a frame
void Tutorial31_GpuDriver2::Render()
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
}

void Tutorial31_GpuDriver2::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }

    float4x4 view = float4x4(-4.37113883e-08f, 0.0995022133f, -0.995037317f, 0.00000000f,
                             0.00000000f, 0.995037317f, 0.0995022133f, 0.00000000f,
                             1.00000000f, 4.34937997e-09f, -4.34944631e-08f, 0.00000000f,
                             -13.9999886f, -124.379257f, 258.908844f, 1.00000000f);
    float4x4 proj = float4x4(1.f, 0.f, 0.f, 0.f,
                             0.00000000f, 1.74999988f, 0.00000000f, 0.00000000f,
                             0.00000000f, 0.00000000f, -0.000100009995f, 1.00000000f,
                             0.00000000f, 0.00000000f, 0.100009993f, 0.00000000f);

    {
        /************************************************************************/
        // Update Camera
        /************************************************************************/
        const uint32_t width  = mWidth;
        const uint32_t height = mHeight;

        mCameraUniform.mView           = view;
        mCameraUniform.mProject        = proj;
        mCameraUniform.mViewProject    = view * proj;
        mCameraUniform.mInvProj        = mCameraUniform.mViewProject.Inverse();
        mCameraUniform.mInvView        = mCameraUniform.mView.Inverse();
        mCameraUniform.mInvViewProject = mCameraUniform.mProject.Inverse();
        mCameraUniform.mNear           = gCamearNear;
        mCameraUniform.mFarNearDiff    = gCamearFar - gCamearNear; // if OpenGL convention was used this would be 2x the value
        mCameraUniform.mFarNear        = gCamearNear * gCamearFar;
        mCameraUniform.mCameraPos      = float4(m_Camera.GetPos(), 1.f);

        mCameraUniform.mTwoOverRes = float2(1.5f / width, 1.5f / height);

        float4x4 primaryProjMat = mCameraUniform.mProject;
        float    depthMul       = primaryProjMat.m22;
        float    depthAdd       = primaryProjMat.m32;

        if (depthAdd == 0.f)
        {
            //avoid dividing by 0 in this case
            depthAdd = 0.00000001f;
        }

        if (primaryProjMat.m33 < 1.0f)
        {
            float subtractValue = depthMul / depthAdd;
            subtractValue -= 0.00000001f;
            mCameraUniform.mDeviceZToWorldZ = float4(0.f, 0.f, 1.f / depthAdd, subtractValue);
        }
        mCameraUniform.mWindowSize = float2((float)width, (float)height);
    }
    {
        gMeshInfoData.mTranslationMat = float4x4::Translation(gMeshInfoData.mTranslation);
        gMeshInfoData.mScaleMat       = float4x4::Scale(gMeshInfoData.mScale);

        float4x4 offsetTranslationMat = float4x4::Translation((gMeshInfoData.mOffsetTranslation));

        float4x4 world = offsetTranslationMat * gMeshInfoData.mScaleMat * gMeshInfoData.mTranslationMat;

        mVisibilityBufferCB.mWorldViewProjMat[VIEW_CAMERA]              = world * view * proj;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mWindowSizeX = (float)mWidth;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mWindowSizeY = (float)mHeight;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mSampleCount = gAppSettings.mMsaaLevel;
    }
}

void Tutorial31_GpuDriver2::WindowResize(Uint32 Width, Uint32 Height)
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
