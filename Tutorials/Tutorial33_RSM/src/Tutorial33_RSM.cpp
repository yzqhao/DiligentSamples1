
#include "Tutorial33_RSM.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

#include <random>

#include "imgui.h"

namespace Diligent
{

static Diligent::float3 gCamPos{20.0f, -2.0f, 0.9f};

static const Uint32 gRSMBufferWidth = 2048;

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const char* gStrRSM              = "RSM";
static const char* gStrgStrRSMObjectCBV = "RSMObjectCBV";
static const char* gStrgStrRSMCameraCBV = "RSMCameraCB";
static const char* gStrDirect           = "Direct";
static const char* gStrDirectObjectCBV  = "DirectObjectCBV";
static const char* gStrDirectCameraCBV  = "DirectCameraCB";

static const char* gStrRSMBufferA     = "RSMBufferA";
static const char* gStrRSMBufferB     = "RSMBufferB";
static const char* gStrRSMBufferC     = "RSMBufferC";
static const char* gStrRSMBufferDepth = "RSMBufferDepth";

static const char* gStrRSMBufferASRV     = "RSMBufferASRV";
static const char* gStrRSMBufferBSRV     = "RSMBufferBSRV";
static const char* gStrRSMBufferCSRV     = "RSMBufferCSRV";
static const char* gStrRSMBufferDepthSRV = "RSMBufferDepthSRV";

static const char* gStrRSMBufferARTV     = "RSMBufferARTV";
static const char* gStrRSMBufferBRTV     = "RSMBufferBRTV";
static const char* gStrRSMBufferCRTV     = "RSMBufferCRTV";
static const char* gStrRSMBufferDepthDSV = "RSMBufferDepthDSV";

// clang-format off
const char* pMaterialImageFileNames[] = {
	"SponzaPBR_Textures/ao",
	"SponzaPBR_Textures/ao",
	"SponzaPBR_Textures/ao",
	"SponzaPBR_Textures/ao",
	"SponzaPBR_Textures/ao",

	//common
	"SponzaPBR_Textures/ao",
	"SponzaPBR_Textures/Dielectric_metallic",
	"SponzaPBR_Textures/Metallic_metallic",
	"SponzaPBR_Textures/gi_flag",

	//Background
	"SponzaPBR_Textures/Background/Background_Albedo",
	"SponzaPBR_Textures/Background/Background_Normal",
	"SponzaPBR_Textures/Background/Background_Roughness",

	//ChainTexture
	"SponzaPBR_Textures/ChainTexture/ChainTexture_Albedo",
	"SponzaPBR_Textures/ChainTexture/ChainTexture_Metallic",
	"SponzaPBR_Textures/ChainTexture/ChainTexture_Normal",
	"SponzaPBR_Textures/ChainTexture/ChainTexture_Roughness",

	//Lion
	"SponzaPBR_Textures/Lion/Lion_Albedo",
	"SponzaPBR_Textures/Lion/Lion_Normal",
	"SponzaPBR_Textures/Lion/Lion_Roughness",

	//Sponza_Arch
	"SponzaPBR_Textures/Sponza_Arch/Sponza_Arch_diffuse",
	"SponzaPBR_Textures/Sponza_Arch/Sponza_Arch_normal",
	"SponzaPBR_Textures/Sponza_Arch/Sponza_Arch_roughness",

	//Sponza_Bricks
	"SponzaPBR_Textures/Sponza_Bricks/Sponza_Bricks_a_Albedo",
	"SponzaPBR_Textures/Sponza_Bricks/Sponza_Bricks_a_Normal",
	"SponzaPBR_Textures/Sponza_Bricks/Sponza_Bricks_a_Roughness",

	//Sponza_Ceiling
	"SponzaPBR_Textures/Sponza_Ceiling/Sponza_Ceiling_diffuse",
	"SponzaPBR_Textures/Sponza_Ceiling/Sponza_Ceiling_normal",
	"SponzaPBR_Textures/Sponza_Ceiling/Sponza_Ceiling_roughness",

	//Sponza_Column
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_a_diffuse",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_a_normal",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_a_roughness",

	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_b_diffuse",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_b_normal",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_b_roughness",

	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_c_diffuse",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_c_normal",
	"SponzaPBR_Textures/Sponza_Column/Sponza_Column_c_roughness",

	//Sponza_Curtain
	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Blue_diffuse",
	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Blue_normal",

	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Green_diffuse",
	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Green_normal",

	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Red_diffuse",
	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_Red_normal",

	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_metallic",
	"SponzaPBR_Textures/Sponza_Curtain/Sponza_Curtain_roughness",

	//Sponza_Details
	"SponzaPBR_Textures/Sponza_Details/Sponza_Details_diffuse",
	"SponzaPBR_Textures/Sponza_Details/Sponza_Details_metallic",
	"SponzaPBR_Textures/Sponza_Details/Sponza_Details_normal",
	"SponzaPBR_Textures/Sponza_Details/Sponza_Details_roughness",

	//Sponza_Fabric
	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Blue_diffuse",
	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Blue_normal",

	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Green_diffuse",
	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Green_normal",

	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_metallic",
	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_roughness",

	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Red_diffuse",
	"SponzaPBR_Textures/Sponza_Fabric/Sponza_Fabric_Red_normal",

	//Sponza_FlagPole
	"SponzaPBR_Textures/Sponza_FlagPole/Sponza_FlagPole_diffuse",
	"SponzaPBR_Textures/Sponza_FlagPole/Sponza_FlagPole_normal",
	"SponzaPBR_Textures/Sponza_FlagPole/Sponza_FlagPole_roughness",

	//Sponza_Floor
	"SponzaPBR_Textures/Sponza_Floor/Sponza_Floor_diffuse",
	"SponzaPBR_Textures/Sponza_Floor/Sponza_Floor_normal",
	"SponzaPBR_Textures/Sponza_Floor/Sponza_Floor_roughness",

	//Sponza_Roof
	"SponzaPBR_Textures/Sponza_Roof/Sponza_Roof_diffuse",
	"SponzaPBR_Textures/Sponza_Roof/Sponza_Roof_normal",
	"SponzaPBR_Textures/Sponza_Roof/Sponza_Roof_roughness",

	//Sponza_Thorn
	"SponzaPBR_Textures/Sponza_Thorn/Sponza_Thorn_diffuse",
	"SponzaPBR_Textures/Sponza_Thorn/Sponza_Thorn_normal",
	"SponzaPBR_Textures/Sponza_Thorn/Sponza_Thorn_roughness",

	//Vase
	"SponzaPBR_Textures/Vase/Vase_diffuse",
	"SponzaPBR_Textures/Vase/Vase_normal",
	"SponzaPBR_Textures/Vase/Vase_roughness",

	//VaseHanging
	"SponzaPBR_Textures/VaseHanging/VaseHanging_diffuse",
	"SponzaPBR_Textures/VaseHanging/VaseHanging_normal",
	"SponzaPBR_Textures/VaseHanging/VaseHanging_roughness",

	//VasePlant
	"SponzaPBR_Textures/VasePlant/VasePlant_diffuse",
	"SponzaPBR_Textures/VasePlant/VasePlant_normal",
	"SponzaPBR_Textures/VasePlant/VasePlant_roughness",

	//VaseRound
	"SponzaPBR_Textures/VaseRound/VaseRound_diffuse",
	"SponzaPBR_Textures/VaseRound/VaseRound_normal",
	"SponzaPBR_Textures/VaseRound/VaseRound_roughness",

	"lion/lion_albedo",
	"lion/lion_specular",
	"lion/lion_normal",

};

uint32_t gMaterialIds[] = {
	0,  3,  1,  4,  5,  6,  7,  8,  6,  9,  7,  6, 10, 5, 7,  5, 6, 7,  6, 7,  6,  7,  6,  7,  6,  7,  6,  7,  6,  7,  6,  7,  6,  7,  6,
	5,  6,  5,  11, 5,  11, 5,  11, 5,  10, 5,  9, 8,  6, 12, 2, 5, 13, 0, 14, 15, 16, 14, 15, 14, 16, 15, 13, 17, 18, 19, 18, 19, 18, 17,
	19, 18, 17, 20, 21, 20, 21, 20, 21, 20, 21, 3, 1,  3, 1,  3, 1, 3,  1, 3,  1,  3,  1,  3,  1,  22, 23, 4,  23, 4,  5,  24, 5,
};

void assignSponzaTextures(std::vector<uint32_t>& sponzaTextureIndexforMaterial)
{
	int AO = 5;
	int NoMetallic = 6;

	//00 : leaf
	sponzaTextureIndexforMaterial.push_back(66);
	sponzaTextureIndexforMaterial.push_back(67);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(68);
	sponzaTextureIndexforMaterial.push_back(AO);

	//01 : vase_round
	sponzaTextureIndexforMaterial.push_back(78);
	sponzaTextureIndexforMaterial.push_back(79);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(80);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 02 : 16___Default (gi_flag)
	sponzaTextureIndexforMaterial.push_back(8);
	sponzaTextureIndexforMaterial.push_back(8);    // !!!!!!
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(8);    // !!!!!
	sponzaTextureIndexforMaterial.push_back(AO);

	//03 : Material__57 (Plant)
	sponzaTextureIndexforMaterial.push_back(75);
	sponzaTextureIndexforMaterial.push_back(76);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(77);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 04 : Material__298
	sponzaTextureIndexforMaterial.push_back(9);
	sponzaTextureIndexforMaterial.push_back(10);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(11);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 05 : bricks
	sponzaTextureIndexforMaterial.push_back(22);
	sponzaTextureIndexforMaterial.push_back(23);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(24);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 06 :  arch
	sponzaTextureIndexforMaterial.push_back(19);
	sponzaTextureIndexforMaterial.push_back(20);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(21);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 07 : ceiling
	sponzaTextureIndexforMaterial.push_back(25);
	sponzaTextureIndexforMaterial.push_back(26);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(27);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 08 : column_a
	sponzaTextureIndexforMaterial.push_back(28);
	sponzaTextureIndexforMaterial.push_back(29);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(30);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 09 : Floor
	sponzaTextureIndexforMaterial.push_back(60);
	sponzaTextureIndexforMaterial.push_back(61);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(6);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 10 : column_c
	sponzaTextureIndexforMaterial.push_back(34);
	sponzaTextureIndexforMaterial.push_back(35);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(36);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 11 : details
	sponzaTextureIndexforMaterial.push_back(45);
	sponzaTextureIndexforMaterial.push_back(47);
	sponzaTextureIndexforMaterial.push_back(46);
	sponzaTextureIndexforMaterial.push_back(48);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 12 : column_b
	sponzaTextureIndexforMaterial.push_back(31);
	sponzaTextureIndexforMaterial.push_back(32);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(33);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 13 : flagpole
	sponzaTextureIndexforMaterial.push_back(57);
	sponzaTextureIndexforMaterial.push_back(58);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(59);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 14 : fabric_e (green)
	sponzaTextureIndexforMaterial.push_back(51);
	sponzaTextureIndexforMaterial.push_back(52);
	sponzaTextureIndexforMaterial.push_back(53);
	sponzaTextureIndexforMaterial.push_back(54);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 15 : fabric_d (blue)
	sponzaTextureIndexforMaterial.push_back(49);
	sponzaTextureIndexforMaterial.push_back(50);
	sponzaTextureIndexforMaterial.push_back(53);
	sponzaTextureIndexforMaterial.push_back(54);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 16 : fabric_a (red)
	sponzaTextureIndexforMaterial.push_back(55);
	sponzaTextureIndexforMaterial.push_back(56);
	sponzaTextureIndexforMaterial.push_back(53);
	sponzaTextureIndexforMaterial.push_back(54);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 17 : fabric_g (curtain_blue)
	sponzaTextureIndexforMaterial.push_back(37);
	sponzaTextureIndexforMaterial.push_back(38);
	sponzaTextureIndexforMaterial.push_back(43);
	sponzaTextureIndexforMaterial.push_back(44);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 18 : fabric_c (curtain_red)
	sponzaTextureIndexforMaterial.push_back(41);
	sponzaTextureIndexforMaterial.push_back(42);
	sponzaTextureIndexforMaterial.push_back(43);
	sponzaTextureIndexforMaterial.push_back(44);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 19 : fabric_f (curtain_green)
	sponzaTextureIndexforMaterial.push_back(39);
	sponzaTextureIndexforMaterial.push_back(40);
	sponzaTextureIndexforMaterial.push_back(43);
	sponzaTextureIndexforMaterial.push_back(44);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 20 : chain
	sponzaTextureIndexforMaterial.push_back(12);
	sponzaTextureIndexforMaterial.push_back(14);
	sponzaTextureIndexforMaterial.push_back(13);
	sponzaTextureIndexforMaterial.push_back(15);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 21 : vase_hanging
	sponzaTextureIndexforMaterial.push_back(72);
	sponzaTextureIndexforMaterial.push_back(73);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(74);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 22 : vase
	sponzaTextureIndexforMaterial.push_back(69);
	sponzaTextureIndexforMaterial.push_back(70);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(71);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 23 : Material__25 (lion)
	sponzaTextureIndexforMaterial.push_back(16);
	sponzaTextureIndexforMaterial.push_back(17);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(18);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 24 : roof
	sponzaTextureIndexforMaterial.push_back(63);
	sponzaTextureIndexforMaterial.push_back(64);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(65);
	sponzaTextureIndexforMaterial.push_back(AO);

	// 25 : Material__47 - it seems missing
	sponzaTextureIndexforMaterial.push_back(19);
	sponzaTextureIndexforMaterial.push_back(20);
	sponzaTextureIndexforMaterial.push_back(NoMetallic);
	sponzaTextureIndexforMaterial.push_back(21);
	sponzaTextureIndexforMaterial.push_back(AO);
}
// clang-format on

static TEXTURE_FORMAT gRSMFormatA           = TEX_FORMAT_RGBA16_FLOAT;
static TEXTURE_FORMAT gRSMFormatB           = TEX_FORMAT_RGBA16_FLOAT;
static TEXTURE_FORMAT gRSMFormatC           = TEX_FORMAT_RGBA8_UNORM;
static TEXTURE_FORMAT gRSMBufferFormatDepth = TEX_FORMAT_D32_FLOAT;

SampleBase* CreateSample()
{
    return new Tutorial33_RSM();
}

void Tutorial33_RSM::CreatePipelineState()
{
    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrRSM;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 3;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = gRSMFormatA;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[1]                = gRSMFormatB;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[2]                = gRSMFormatC;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = gRSMBufferFormatDepth;
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
            ShaderCI.Desc.Name       = "RSM VS";
            ShaderCI.FilePath        = "RSM.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "cbPerObject CB", &m_Buffers[gStrgStrRSMObjectCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrgStrRSMCameraCBV]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "RSM PS";
            ShaderCI.FilePath        = "RSM.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
            // Attribute 1 - normal
            LayoutElement{2, 0, 3, VT_FLOAT32, False}
		};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbLightPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "textureMaps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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
                {SHADER_TYPE_PIXEL, "textureMaps", SamLinearWrapDesc}};
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrRSM]);

        m_pPSOs[gStrRSM]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrgStrRSMCameraCBV]);
        m_pPSOs[gStrRSM]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrgStrRSMObjectCBV]);
        m_pPSOs[gStrRSM]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrgStrRSMCameraCBV]);
        m_pPSOs[gStrRSM]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPerObject")->Set(m_Buffers[gStrgStrRSMObjectCBV]);
        m_pPSOs[gStrRSM]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrRSM], true);
    }
	
	if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrDirect;
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
            ShaderCI.Desc.Name       = "light VS";
            ShaderCI.FilePath        = "light.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "cbPerObject CB", &m_Buffers[gStrDirectObjectCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrDirectCameraCBV]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "light PS";
            ShaderCI.FilePath        = "light.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        // clang-format off
        LayoutElement LayoutElems[] = {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
            // Attribute 1 - normal
            LayoutElement{2, 0, 3, VT_FLOAT32, False}
		};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "textureMaps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "fluxTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "positionTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "normalTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] = {
             {SHADER_TYPE_PIXEL, "textureMaps", SamLinearWrapDesc},
             {SHADER_TYPE_PIXEL, "fluxTexture", SamLinearWrapDesc},
             {SHADER_TYPE_PIXEL, "positionTexture", SamLinearWrapDesc},
             {SHADER_TYPE_PIXEL, "normalTexture", SamLinearWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrDirect]);

        m_pPSOs[gStrDirect]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrDirectCameraCBV]);
        m_pPSOs[gStrDirect]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrDirectObjectCBV]);
        m_pPSOs[gStrDirect]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrDirectCameraCBV]);
        m_pPSOs[gStrDirect]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPerObject")->Set(m_Buffers[gStrDirectObjectCBV]);
        m_pPSOs[gStrDirect]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrDirect], true);
    }
}

void Tutorial33_RSM::CreateBuffers()
{
    { // RSMbuffer
        TextureDesc RSMBufferATexDesc;
        RSMBufferATexDesc.Name      = gStrRSMBufferA;
        RSMBufferATexDesc.Type      = RESOURCE_DIM_TEX_2D;
        RSMBufferATexDesc.Width     = gRSMBufferWidth;
        RSMBufferATexDesc.Height    = gRSMBufferWidth;
        RSMBufferATexDesc.MipLevels = 1;
        RSMBufferATexDesc.Format    = gRSMFormatA;
        RSMBufferATexDesc.Usage     = USAGE_DEFAULT;
        RSMBufferATexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(RSMBufferATexDesc, nullptr, &m_Textures[gStrRSMBufferA]);
        m_TextureViews[gStrRSMBufferARTV] = m_Textures[gStrRSMBufferA]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrRSMBufferASRV] = m_Textures[gStrRSMBufferA]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc RSMBufferBTexDesc;
        RSMBufferBTexDesc.Name      = gStrRSMBufferB;
        RSMBufferBTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        RSMBufferBTexDesc.Width     = gRSMBufferWidth;
        RSMBufferBTexDesc.Height    = gRSMBufferWidth;
        RSMBufferBTexDesc.MipLevels = 1;
        RSMBufferBTexDesc.Format    = gRSMFormatB;
        RSMBufferBTexDesc.Usage     = USAGE_DEFAULT;
        RSMBufferBTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(RSMBufferBTexDesc, nullptr, &m_Textures[gStrRSMBufferB]);
        m_TextureViews[gStrRSMBufferBRTV] = m_Textures[gStrRSMBufferB]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrRSMBufferBSRV] = m_Textures[gStrRSMBufferB]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc RSMBufferCTexDesc;
        RSMBufferCTexDesc.Name      = gStrRSMBufferC;
        RSMBufferCTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        RSMBufferCTexDesc.Width     = gRSMBufferWidth;
        RSMBufferCTexDesc.Height    = gRSMBufferWidth;
        RSMBufferCTexDesc.MipLevels = 1;
        RSMBufferCTexDesc.Format    = gRSMFormatC;
        RSMBufferCTexDesc.Usage     = USAGE_DEFAULT;
        RSMBufferCTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(RSMBufferCTexDesc, nullptr, &m_Textures[gStrRSMBufferC]);
        m_TextureViews[gStrRSMBufferCRTV] = m_Textures[gStrRSMBufferC]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrRSMBufferCSRV] = m_Textures[gStrRSMBufferC]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc RSMBufferDepthTexDesc;
        RSMBufferDepthTexDesc.Name      = gStrRSMBufferDepth;
        RSMBufferDepthTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        RSMBufferDepthTexDesc.Width     = gRSMBufferWidth;
        RSMBufferDepthTexDesc.Height    = gRSMBufferWidth;
        RSMBufferDepthTexDesc.MipLevels = 1;
        RSMBufferDepthTexDesc.Format    = gRSMBufferFormatDepth;
        RSMBufferDepthTexDesc.Usage     = USAGE_DEFAULT;
        RSMBufferDepthTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(RSMBufferDepthTexDesc, nullptr, &m_Textures[gStrRSMBufferDepth]);
        m_TextureViews[gStrRSMBufferDepthDSV] = m_Textures[gStrRSMBufferDepth]->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        m_TextureViews[gStrRSMBufferDepthSRV] = m_Textures[gStrRSMBufferDepth]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
}

void Tutorial33_RSM::CreateIndexBuffer()
{
    
}

void Tutorial33_RSM::LoadTexture()
{
    ITextureView* pMeshTex[100] = {};
    int           texcnt        = 81;
    for (int i = 0; i < texcnt; ++i)
    {
        TextureLoadInfo loadInfoMesh;

        std::string Filename = std::string("") + pMaterialImageFileNames[i] + ".dds";
        bool        srgb     = false;
        if (strstr(pMaterialImageFileNames[i], "Albedo") || strstr(pMaterialImageFileNames[i], "diffuse"))
        {
            srgb = true;
        }
        loadInfoMesh.IsSRGB = srgb;

        RefCntAutoPtr<ITexture> MeshTex;
        CreateTextureFromFile(Filename.c_str(), loadInfoMesh, m_pDevice, &MeshTex);
        m_Textures[std::to_string(i)] = MeshTex;
        pMeshTex[i]                   = MeshTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    m_ShaderResourceBindings[gStrRSM]->GetVariableByName(SHADER_TYPE_PIXEL, "textureMaps")->SetArray((IDeviceObject* const*)pMeshTex, 0, 81);
    m_ShaderResourceBindings[gStrDirect]->GetVariableByName(SHADER_TYPE_PIXEL, "textureMaps")->SetArray((IDeviceObject* const*)pMeshTex, 0, 81);
}

void Tutorial33_RSM::LoadModel()
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
    ModelCI.FileName             = "sponza.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model                      = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);

    size_t subMeshCnt1 = m_Model->Scenes[m_Model->DefaultSceneId].LinearNodes[0]->pMesh->Primitives.size();
    mObjectCbData.resize(subMeshCnt1);

    std::vector<uint32_t> gSponzaTextureIndexforMaterial;
    assignSponzaTextures(gSponzaTextureIndexforMaterial);
    for (int i = 0; i < mObjectCbData.size(); ++i)
    {
        uint             DiffuseMapIndex = 0;
        ObjectConstants& obcs            = mObjectCbData[i];
        obcs.metalness                   = 0;
        obcs.roughness                   = 0.5f;
        obcs.pbrMaterials                = 1;
        obcs.PreWorld                    = obcs.World;

        uint materialID = gMaterialIds[i];
        materialID *= 5; //because it uses 5 basic textures for redering BRDF
        DiffuseMapIndex = ((gSponzaTextureIndexforMaterial[materialID + 0] & 0xFF) << 0) |
            ((gSponzaTextureIndexforMaterial[materialID + 1] & 0xFF) << 8) |
            ((gSponzaTextureIndexforMaterial[materialID + 2] & 0xFF) << 16) |
            ((gSponzaTextureIndexforMaterial[materialID + 3] & 0xFF) << 24);

        obcs.World = float4x4::Scale(0.02f) * float4x4::Translation(float3(0.0f, -6.0f, 0.0f));

        obcs.MaterialIndex = DiffuseMapIndex;
    }
}

void Tutorial33_RSM::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    {
        //Ëæ»ú²ÉÑùmap
        std::vector<float4> sampleCoordsandWeights;
        std::default_random_engine e;
        std::uniform_real_distribution<float> u(0, 1);
        for (int i = 0; i < SAMPLE_NUMBER; i++)
        {
            float xi1 = u(e);
            float xi2 = u(e);
            mPassCbData.randomArray[i] = {xi1 * sin(2 * PI_F * xi2),
                                          xi1 * cos(2 * PI_F * xi2),
                                          xi1 * xi1,
                                          11.0f};
        }

        mPassCbData.filterRange = 25;
    }

    CreatePipelineState();
    CreateBuffers();
    LoadTexture();
    LoadModel();
}

// Render a frame
void Tutorial33_RSM::Render()
{
    if (true)
    {
        ITextureView*           pRTVs[3] = {m_TextureViews[gStrRSMBufferARTV], m_TextureViews[gStrRSMBufferBRTV], m_TextureViews[gStrRSMBufferCRTV]};
        SetRenderTargetsAttribs RTAttrs;
        RTAttrs.NumRenderTargets    = 3;
        RTAttrs.ppRenderTargets     = pRTVs;
        RTAttrs.pDepthStencil       = m_TextureViews[gStrRSMBufferDepthDSV];
        RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

        Viewport vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width    = static_cast<float>(gRSMBufferWidth);
        vp.Height   = static_cast<float>(gRSMBufferWidth);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_pImmediateContext->SetViewports(1, &vp, gRSMBufferWidth, gRSMBufferWidth);

        constexpr float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pRTVs[0], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[1], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[2], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearDepthStencil(m_TextureViews[gStrRSMBufferDepthDSV], CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrRSM]);

        int                         idx    = 0;
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
                for (const auto& primitive : pNode->pMesh->Primitives)
                {
                    {
                        MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrgStrRSMObjectCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mObjectCbData[idx++];
                    }
                    {
                        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrgStrRSMCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mPassCbData;
                    }

                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrRSM], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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
    if (true)
    {
        ITextureView* pRTVs[1] = {m_pSwapChain->GetCurrentBackBufferRTV()};
        auto* pDSV = m_pSwapChain->GetDepthBufferDSV();

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

        Viewport vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width    = static_cast<float>(mWidth);
        vp.Height   = static_cast<float>(mHeight);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_pImmediateContext->SetViewports(1, &vp, mWidth, mHeight);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrDirect]);

        m_ShaderResourceBindings[gStrDirect]->GetVariableByName(SHADER_TYPE_PIXEL, "normalTexture")->Set(m_TextureViews[gStrRSMBufferASRV]);
        m_ShaderResourceBindings[gStrDirect]->GetVariableByName(SHADER_TYPE_PIXEL, "positionTexture")->Set(m_TextureViews[gStrRSMBufferBSRV]);
        m_ShaderResourceBindings[gStrDirect]->GetVariableByName(SHADER_TYPE_PIXEL, "fluxTexture")->Set(m_TextureViews[gStrRSMBufferCSRV]);

        int                         idx    = 0;
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
                for (const auto& primitive : pNode->pMesh->Primitives)
                {
                    {
                        MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrDirectObjectCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mObjectCbData[idx++];
                    }
                    {
                        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrDirectCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mPassCbData;
                    }

                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrDirect], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

void Tutorial33_RSM::Update(double CurrTime, double ElapsedTime)
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

    mPassCbData.AmbientLight = {0.25f, 0.25f, 0.35f, 1.0f};

    {
        DirectionalLight light = {};
        light.mCol             = float4(1.0f, 1.0f, 1.0f, 5.0f);
        light.mPos             = float4(0.0f, 0.0f, 0.0f, 0.0f);
        light.mDir             = float4(-1.0f, -1.5f, 1.0f, 0.0f);
        light.mDir             = normalize(light.mDir);

        mPassCbData.DLights[0] = light;

        mPassCbData.amountOfDLights = 1;

        float4x4 lightview             = float4x4::Translation(-light.mPos); //  - light.mDir * 20
        float4x4 LightProjectionMatrix = float4x4::OrthoOffCenter(-15.0f, 15.0f, -15.0f, 15.0f, 0.1f, 20.0f, false);
        mPassCbData.LightView          = lightview;
        mPassCbData.LightProj          = LightProjectionMatrix;
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::SliderFloat4("Grid Size", &mPassCbData.DLights[0].mDir.x, -1, 1))
        {
        }
        if (ImGui::Checkbox("Open RSM GI", &m_bRSM))
        {
        }
        if (ImGui::SliderInt("filter ranger", &mPassCbData.filterRange, 1, 50))
        {
        }
    }
    ImGui::End();

    mPassCbData.bRSM = m_bRSM ? 1 : 0;
}

void Tutorial33_RSM::WindowResize(Uint32 Width, Uint32 Height)
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
