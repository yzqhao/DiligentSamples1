
#include "Tutorial36_VXGI.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

#include <cassert>
#include "imgui.h"

namespace Diligent
{
static Diligent::float3 gCamPos{10.0f, -1.0f, 0.9f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const Uint32 gShadowMapSize = 4096;


static const char* gStrVoxelization         = "Voxelization";
static const char* gStrVoxelizationTex3D    = "VoxelizationTex3D";
static const char* gStrVoxelizationTex3DSRV = "VoxelizationTex3DSRV";
static const char* gStrVoxelizationTex3DRTV = "VoxelizationTex3DRTV";
static const char* gStrVoxelizationTex3DUAV = "VoxelizationTex3DUAV";

static const char* gStrFillTex3D       = "FillTex3D";
static const char* gStrFillTex3DBuffer = "FillTex3DBuffer";

static const char* gStrDepthPass         = "DepthPass";
static const char* gStrDepthPassObjectCB = "DepthPassObjectCB";
static const char* gStrDepthPassCameraCB = "DepthPassCameraCB";


static const char* gStrDepthShadowMap    = "DepthShadowMap";
static const char* gStrDepthShadowMapSRV = "DepthShadowMapSRV";
static const char* gStrDepthShadowMapDSV = "DepthShadowMapDSV";

static const char* gStrCubeBack    = "CubeBack";
static const char* gStrCubeBackSRV = "CubeBackSRV";
static const char* gStrCubeBackRTV = "CubeBackRTV";

static const char* gStrCubeFront    = "CubeFront";
static const char* gStrCubeFrontSRV = "CubeFrontSRV";
static const char* gStrCubeFrontRTV = "CubeFrontRTV";

static const char* gStrCubeDepth    = "CubeDepth";
static const char* gStrCubeDepthSRV = "CubeDepthSRV";
static const char* gStrCubeDepthDSV = "CubeDepthDSV";

static const char* gStrScreenVB = "ScreenVB";
static const char* gStrSkyBoxVB = "SkyBoxVB";
static const char* gStrSkyBoxIB = "SkyBoxIB";

static const char* gStrWorldPosBack   = "WorldPosBack";
static const char* gStrWorldPosFront  = "WorldPosFront";
static const char* gStrWorldPosCamera = "WorldPosCamera";

static const char* gStrVoxelizationVisualizer        = "VoxelizationVisualizer";
static const char* gStrVoxelizationVisualizerCamera  = "VoxelizationVisualizerCamera";
static const char* gStrVoxelizationVisualizerSetting = "VoxelizationVisualizerSetting";

static const char* gStrGBuffer = "GBuffer";

static const char* gStrGBufferA     = "GBufferA";
static const char* gStrGBufferB     = "GBufferB";
static const char* gStrGBufferC     = "GBufferC";
static const char* gStrGBufferD     = "GBufferD";
static const char* gStrGBufferDepth = "GBufferDepth";

static const char* gStrGBufferASRV     = "GBufferASRV";
static const char* gStrGBufferBSRV     = "GBufferBSRV";
static const char* gStrGBufferCSRV     = "GBufferCSRV";
static const char* gStrGBufferDSRV     = "GBufferDSRV";
static const char* gStrGBufferDepthSRV = "GBufferDepthSRV";

static const char* gStrGBufferARTV     = "GBufferARTV";
static const char* gStrGBufferBRTV     = "GBufferBRTV";
static const char* gStrGBufferCRTV     = "GBufferCRTV";
static const char* gStrGBufferDRTV     = "GBufferDRTV";
static const char* gStrGBufferDepthDSV = "GBufferDepthDSV";

static const char* gStrGBufferCBV = "GBufferCBV";
static const char* gStrCameraCBV  = "CameraCBV";

static const char* gStrConeTracing          = "ConeTracing";
static const char* gStrConeTracingSettingCB = "gStrConeTracingSettingCB";

static const TEXTURE_FORMAT gShadowMapFormat = TEX_FORMAT_D32_FLOAT;
static const TEXTURE_FORMAT gCubeBackFormat  = TEX_FORMAT_RGBA16_FLOAT;
static const TEXTURE_FORMAT gCubeFrontFormat = TEX_FORMAT_RGBA16_FLOAT;
static const TEXTURE_FORMAT gCubeDepthFormat = TEX_FORMAT_D32_FLOAT;

static const TEXTURE_FORMAT gGBufferFormatA     = TEX_FORMAT_RGBA8_UNORM_SRGB;
static const TEXTURE_FORMAT gGBufferFormatB     = TEX_FORMAT_RGBA16_FLOAT;
static const TEXTURE_FORMAT gGBufferFormatC     = TEX_FORMAT_RGBA16_FLOAT;
static const TEXTURE_FORMAT gGBufferFormatD     = TEX_FORMAT_RG16_FLOAT;
static const TEXTURE_FORMAT gGBufferFormatDepth = TEX_FORMAT_D32_FLOAT;


// clang-format off
float gScreenQuadPoints[] = {
	-1.0f, 3.0f, 0.5f, 0.0f, -1.0f,
	-1.0f, -1.0f, 0.5f, 0.0f, 1.0f,
	3.0f, -1.0f, 0.5f, 2.0f, 1.0f,
};

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

SampleBase* CreateSample()
{
    return new Tutorial36_VXGI();
}

void Tutorial36_VXGI::voxel_grid_resolution_changed(int new_resolution_index)
{
    assert(new_resolution_index < TOTAL_VOXELGRID_RESOLUTIONS);

    mVoxelization.current_resolution = new_resolution_index;
    voxelize_next_frame = true;
}
int Tutorial36_VXGI::get_current_voxelgrid_resolution()
{
    return VOXELGRID_RESOLUTIONS[mVoxelization.current_resolution];
}

RefCntAutoPtr<ITexture> Tutorial36_VXGI::get_current_voxelgrid()
{
    return mVoxelization.resolutions[mVoxelization.current_resolution];
}

void Tutorial36_VXGI::CreatePipelineState()
{
    {	//gStrFillTex3D
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
                {SHADER_TYPE_COMPUTE, "RootConstant", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
                {SHADER_TYPE_COMPUTE, "data", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
                {SHADER_TYPE_COMPUTE, "dstTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        PSODesc.Name      = gStrFillTex3D;
        PSOCreateInfo.pCS = pBRDFIntegrationCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrFillTex3D]);

        m_pPSOs[gStrFillTex3D]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrFillTex3D], true);
    }

    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrDepthPass;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 0;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = gShadowMapFormat;
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
            ShaderCI.Desc.Name       = "Depth VS";
            ShaderCI.FilePath        = "Depth.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(DepthPassObject), "object CB", &m_Buffers[gStrDepthPassObjectCB]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "camera CB", &m_Buffers[gStrDepthPassCameraCB]);
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

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrDepthPass]);

        m_pPSOs[gStrDepthPass]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrDepthPassObjectCB]);
        m_pPSOs[gStrDepthPass]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrDepthPassCameraCB]);
        m_pPSOs[gStrDepthPass]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrDepthPass], true);
    }

    // cube world pos
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrWorldPosBack;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = gCubeBackFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_FRONT;
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
            ShaderCI.Desc.Name       = "WorldPos VS";
            ShaderCI.FilePath        = "WorldPos.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "WorldPos PS";
            ShaderCI.FilePath        = "WorldPos.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(WorldPosCameraCB), "camera CB", &m_Buffers[gStrWorldPosCamera]);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False}};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_VERTEX, "CameraCB", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrWorldPosBack]);

        PSOCreateInfo.PSODesc.Name                             = gStrWorldPosFront;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode = CULL_MODE_BACK;
        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrWorldPosFront]);

        m_pPSOs[gStrWorldPosBack]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "CameraCB")->Set(m_Buffers[gStrWorldPosCamera]);
        m_pPSOs[gStrWorldPosBack]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrWorldPosBack], true);

        m_pPSOs[gStrWorldPosFront]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "CameraCB")->Set(m_Buffers[gStrWorldPosCamera]);
        m_pPSOs[gStrWorldPosFront]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrWorldPosFront], true);
    }

    // voxel visualizer
	{
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrVoxelizationVisualizer;
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
            ShaderCI.Desc.Name       = "VoxelizationVisualizer VS";
            ShaderCI.FilePath        = "VoxelizationVisualizer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "VoxelizationVisualizer PS";
            ShaderCI.FilePath        = "VoxelizationVisualizer.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(Voxelization_Settings), "setting CB", &m_Buffers[gStrVoxelizationVisualizerSetting]);
            CreateUniformBuffer(m_pDevice, sizeof(VisualizerCameraCB), "camera CB", &m_Buffers[gStrVoxelizationVisualizerCamera]);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False}};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "VoxelizationSettings", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "CameraCB", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "TexVoxelgrid", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "TexCubeBack", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "TexCubeFront", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] = {
            {SHADER_TYPE_PIXEL, "TexVoxelgrid", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "TexCubeBack", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "TexCubeFront", SamLinearWrapDesc},
		};
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrVoxelizationVisualizer]);

        m_pPSOs[gStrVoxelizationVisualizer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "VoxelizationSettings")->Set(m_Buffers[gStrVoxelizationVisualizerSetting]);
        m_pPSOs[gStrVoxelizationVisualizer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "CameraCB")->Set(m_Buffers[gStrVoxelizationVisualizerCamera]);
        m_pPSOs[gStrVoxelizationVisualizer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrVoxelizationVisualizer], true);
    }

    // GBuffer
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrGBuffer;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 4;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = gGBufferFormatA;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[1]                = gGBufferFormatB;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[2]                = gGBufferFormatC;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[3]                = gGBufferFormatD;
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
            ShaderCI.Desc.Name       = "FillGbuffers VS";
            ShaderCI.FilePath        = "FillGbuffers.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "cbPerObject CB", &m_Buffers[gStrGBufferCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrCameraCBV]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "FillGbuffers PS";
            ShaderCI.FilePath        = "FillGbuffers.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
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
            {SHADER_TYPE_VERTEX, "cbPerObject", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_VERTEX, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
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

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrGBuffer]);

        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_VERTEX, "cbPerObject")->Set(m_Buffers[gStrGBufferCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrGBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPerObject")->Set(m_Buffers[gStrGBufferCBV]);
        m_pPSOs[gStrGBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrGBuffer], true);
    }

    // cone tracing
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                  = gStrConeTracing;
        PSOCreateInfo.PSODesc.PipelineType                          = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets             = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.DSVFormat                    = m_pSwapChain->GetDesc().DepthBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology            = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode      = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable = False;

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
            ShaderCI.Desc.Name       = "Voxelconetracing VS";
            ShaderCI.FilePath        = "Voxelconetracing.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "PS";
            ShaderCI.Desc.Name       = "Voxelconetracing PS";
            ShaderCI.FilePath        = "Voxelconetracing.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(Cone_Tracing_Shader_Settings), "Cone_Tracing_Shader_Settings CB", &m_Buffers[gStrConeTracingSettingCB]);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False}};

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "Settings", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "CameraCB", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "AlbedoTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "NormalTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "RoughnessTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "ShadowMaps", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "VoxelGridTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] = {
            {SHADER_TYPE_PIXEL, "AlbedoTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "NormalTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "RoughnessTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "DepthTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "ShadowMaps", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "VoxelGridTexture", SamLinearWrapDesc},
		};
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrConeTracing]);
        
        m_pPSOs[gStrConeTracing]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "Settings")->Set(m_Buffers[gStrConeTracingSettingCB]);
        m_pPSOs[gStrConeTracing]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrConeTracing]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrConeTracing], true);
    }
}

void Tutorial36_VXGI::CreateTexture3D()
{
    auto fill_pixel = [](std::vector<float>& data, int x, int y, int z, int w, int h, int d) -> void {
        int floats = 4; // r+g+b+a
        int i      = floats * (x + w * (y + h * z));

        assert(i < (w * h * d * floats));

        data[i + 0] = 1.0f;
        data[i + 1] = 1.0f;
        data[i + 2] = 1.0f;
        data[i + 3] = 1.0f;
    };

	auto fill_corners = [&](std::vector<float>& data, int w, int h, int d) -> void {
        assert(w == h && h == d);

        for (int x = 1; x < w; x++)
        {
            fill_pixel(data, x, 1, 1, w, h, d);
            fill_pixel(data, x, h - 1, 1, w, h, d);
            fill_pixel(data, x, 1, d - 1, w, h, d);
            fill_pixel(data, x, h - 1, d - 1, w, h, d);

            fill_pixel(data, 1, x, 1, w, h, d);
            fill_pixel(data, w - 1, x, 1, w, h, d);
            fill_pixel(data, w - 1, x, d - 1, w, h, d);
            fill_pixel(data, 1, x, d - 1, w, h, d);

            fill_pixel(data, 1, 1, x, w, h, d);
            fill_pixel(data, w - 1, 1, x, w, h, d);
            fill_pixel(data, w - 1, h - 1, x, w, h, d);
            fill_pixel(data, 1, h - 1, x, w, h, d);
        }
    };

    for (Uint32 i = 0; i < TOTAL_VOXELGRID_RESOLUTIONS; i++)
    {
        Uint32 dimension = VOXELGRID_RESOLUTIONS[i];
        Uint32 miplevel  = (Uint32)log2(dimension);

		size_t datasize = dimension * dimension * dimension * 4;
        mInitVoxelData[i].resize(datasize);
        fill_corners(mInitVoxelData[i], dimension, dimension, dimension);

		TextureDesc Tex3DDesc;
        Tex3DDesc.Name      = gStrVoxelizationTex3D;
        Tex3DDesc.Type      = RESOURCE_DIM_TEX_3D;
        Tex3DDesc.Width     = dimension;
        Tex3DDesc.Height    = dimension;
        Tex3DDesc.Depth     = dimension;
        Tex3DDesc.MipLevels = miplevel;
        Tex3DDesc.Format    = TEX_FORMAT_RGBA8_UNORM;
        Tex3DDesc.Usage     = USAGE_DEFAULT;
        Tex3DDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

        m_pDevice->CreateTexture(Tex3DDesc, nullptr, &m_Textures[Tex3DDesc.Name + std::to_string(i)]);
    }
}

void Tutorial36_VXGI::CreateBuffer()
{
    {
        TextureDesc GBufferDepthTexDesc;
        GBufferDepthTexDesc.Name      = gStrDepthShadowMap;
        GBufferDepthTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GBufferDepthTexDesc.Width     = gShadowMapSize;
        GBufferDepthTexDesc.Height    = gShadowMapSize;
        GBufferDepthTexDesc.MipLevels = 1;
        GBufferDepthTexDesc.Format    = gShadowMapFormat;
        GBufferDepthTexDesc.Usage     = USAGE_DEFAULT;
        GBufferDepthTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(GBufferDepthTexDesc, nullptr, &m_Textures[gStrDepthShadowMap]);
        m_TextureViews[gStrDepthShadowMapDSV]  = m_Textures[gStrDepthShadowMap]->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        m_TextureViews[gStrDepthShadowMapSRV]  = m_Textures[gStrDepthShadowMap]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    {
        TextureDesc TexDesc;
        TexDesc.Name      = gStrCubeBack;
        TexDesc.Type      = RESOURCE_DIM_TEX_2D;
        TexDesc.Width     = mWidth;
        TexDesc.Height    = mHeight;
        TexDesc.MipLevels = 1;
        TexDesc.Format    = gCubeBackFormat;
        TexDesc.Usage     = USAGE_DEFAULT;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(TexDesc, nullptr, &m_Textures[gStrCubeBack]);
        m_TextureViews[gStrCubeBackRTV] = m_Textures[gStrCubeBack]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrCubeBackSRV] = m_Textures[gStrCubeBack]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureDesc TexDesc;
        TexDesc.Name      = gStrCubeFront;
        TexDesc.Type      = RESOURCE_DIM_TEX_2D;
        TexDesc.Width     = mWidth;
        TexDesc.Height    = mHeight;
        TexDesc.MipLevels = 1;
        TexDesc.Format    = gCubeFrontFormat;
        TexDesc.Usage     = USAGE_DEFAULT;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(TexDesc, nullptr, &m_Textures[gStrCubeFront]);
        m_TextureViews[gStrCubeFrontRTV] = m_Textures[gStrCubeFront]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrCubeFrontSRV] = m_Textures[gStrCubeFront]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }
    {
        TextureDesc TexDesc;
        TexDesc.Name      = gStrCubeDepth;
        TexDesc.Type      = RESOURCE_DIM_TEX_2D;
        TexDesc.Width     = mWidth;
        TexDesc.Height    = mHeight;
        TexDesc.MipLevels = 1;
        TexDesc.Format    = gCubeDepthFormat;
        TexDesc.Usage     = USAGE_DEFAULT;
        TexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_DEPTH_STENCIL;
        m_pDevice->CreateTexture(TexDesc, nullptr, &m_Textures[gStrCubeDepth]);
        m_TextureViews[gStrCubeDepthDSV] = m_Textures[gStrCubeDepth]->GetDefaultView(TEXTURE_VIEW_DEPTH_STENCIL);
        m_TextureViews[gStrCubeDepthSRV] = m_Textures[gStrCubeDepth]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    // screen quad vb
    {
        BufferDesc VertBuffDesc;
        VertBuffDesc.Name      = "ScreenQuad vertex buffer";
        VertBuffDesc.Usage     = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size      = sizeof(gScreenQuadPoints);
        BufferData VBData;
        VBData.pData    = gScreenQuadPoints;
        VBData.DataSize = sizeof(gScreenQuadPoints);
        m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrScreenVB]);
    }

    // skybox vb
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

    // skybox Ib
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
    }
}

void Tutorial36_VXGI::LoadModel()
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

        obcs.World = float4x4::Scale(0.01f) * float4x4::Translation(float3(0.0f, -6.0f, 0.0f));

        obcs.MaterialIndex = DiffuseMapIndex;
    }
}

void Tutorial36_VXGI::LoadTexture()
{
    ITextureView* pMeshTex[81] = {};
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

#if 1
    TextureLoadInfo loadInfoVol3d;
    std::string Filename = "vox3d.dds";
    RefCntAutoPtr<ITexture> Vox3dTex;
    CreateTextureFromFile(Filename.c_str(), loadInfoVol3d, m_pDevice, &Vox3dTex);
    m_TextureViews[gStrVoxelizationTex3DSRV] = Vox3dTex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
#endif
    m_ShaderResourceBindings[gStrVoxelizationVisualizer]->GetVariableByName(SHADER_TYPE_PIXEL, "TexCubeBack")->Set(m_TextureViews[gStrCubeBackSRV]);
    m_ShaderResourceBindings[gStrVoxelizationVisualizer]->GetVariableByName(SHADER_TYPE_PIXEL, "TexCubeFront")->Set(m_TextureViews[gStrCubeFrontSRV]);

    m_ShaderResourceBindings[gStrGBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "textureMaps")->SetArray((IDeviceObject* const*)pMeshTex, 0, 81);
}

void Tutorial36_VXGI::PrepareVoxelTex()
{
    for (Uint32 i = 0; i < TOTAL_VOXELGRID_RESOLUTIONS; i++)
    {
        Uint32 dimension = VOXELGRID_RESOLUTIONS[i];
        size_t datasize = dimension * dimension * dimension * 2;

        {
            RefCntAutoPtr<IBuffer> buff1, buff2;

            BufferDesc BuffDesc;
            BuffDesc.Name              = "tex3D buffer";
            BuffDesc.Usage             = USAGE_IMMUTABLE;
            BuffDesc.BindFlags         = BIND_SHADER_RESOURCE;
            BuffDesc.Mode              = BUFFER_MODE_FORMATTED;
            BuffDesc.CPUAccessFlags    = CPU_ACCESS_NONE;
            BuffDesc.ElementByteStride = sizeof(float);
            BuffDesc.Size              = sizeof(float) * datasize;
            BufferData VBData;
            VBData.pData    = mInitVoxelData[i].data();
            VBData.DataSize = sizeof(float) * datasize;
            m_pDevice->CreateBuffer(BuffDesc, &VBData, &buff1);

            VBData.pData = mInitVoxelData[i].data() + datasize;
            m_pDevice->CreateBuffer(BuffDesc, &VBData, &buff2);

			RefCntAutoPtr<IBufferView> pDataSRV1, pDataSRV2;
            BufferViewDesc             ViewDesc;
            ViewDesc.ViewType             = BUFFER_VIEW_SHADER_RESOURCE;
            ViewDesc.Format.ValueType     = VT_FLOAT32;
            ViewDesc.Format.NumComponents = 1;
            buff1->CreateView(ViewDesc, &pDataSRV1);
            buff2->CreateView(ViewDesc, &pDataSRV2);

			std::string strDimension = "Dimension" + std::to_string(i);
			BufferDesc CBuffDesc;
            CBuffDesc.Name           = "Constants buffer";
            CBuffDesc.Usage          = USAGE_DYNAMIC;
            CBuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
            CBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
            CBuffDesc.Size           = sizeof(Uint32);
            m_pDevice->CreateBuffer(CBuffDesc, nullptr, &m_Buffers[strDimension]);
            m_ShaderResourceBindings[gStrFillTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "RootConstant")->Set(m_Buffers[strDimension], SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);

            m_ShaderResourceBindings[gStrFillTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "data1")->Set(pDataSRV1, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_ShaderResourceBindings[gStrFillTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "data2")->Set(pDataSRV2, SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);
            m_ShaderResourceBindings[gStrFillTex3D]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->Set(m_Textures[gStrVoxelizationTex3D + std::to_string(i)]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS), SET_SHADER_RESOURCE_FLAG_ALLOW_OVERWRITE);

			struct Constants
            {
                Uint32 Dimension;
            };
            {
                MapHelper<Constants> ConstData(m_pImmediateContext, m_Buffers[strDimension], MAP_WRITE, MAP_FLAG_DISCARD);
                ConstData->Dimension = static_cast<Uint32>(dimension);
            }
        }

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrFillTex3D]);
        DispatchComputeAttribs DispatAttribs;
        DispatAttribs.ThreadGroupCountX = (Uint32)ceilf(dimension / 16.0f);
        DispatAttribs.ThreadGroupCountY = (Uint32)ceilf(dimension / 16.0f);
        DispatAttribs.ThreadGroupCountZ = dimension;
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrFillTex3D], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribs);
    }
}


void Tutorial36_VXGI::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    CreatePipelineState();
    CreateTexture3D();
    CreateBuffer();
    LoadModel();
    LoadTexture();
    PrepareVoxelTex();
}

// Render a frame
void Tutorial36_VXGI::Render()
{
    // render shadow map
    {
        SetRenderTargetsAttribs RTAttrs;
        RTAttrs.NumRenderTargets    = 0;
        RTAttrs.pDepthStencil       = m_TextureViews[gStrDepthShadowMapDSV];
        RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

        Viewport vp;
        vp.TopLeftX = 0;
        vp.TopLeftY = 0;
        vp.Width    = static_cast<float>(gShadowMapSize);
        vp.Height   = static_cast<float>(gShadowMapSize);
        vp.MinDepth = 0.0f;
        vp.MaxDepth = 1.0f;
        m_pImmediateContext->SetViewports(1, &vp, gShadowMapSize, gShadowMapSize);

        m_pImmediateContext->ClearDepthStencil(m_TextureViews[gStrDepthShadowMapDSV], CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrDepthPass]);

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
                        MapHelper<DepthPassObject> CBConstants(m_pImmediateContext, m_Buffers[gStrDepthPassObjectCB], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mDepthPassObject;
                    }
                    {
                        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrDepthPassCameraCB], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mPassCbData;
                    }

                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrDepthPass], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

    // voxel visualizer
    if (false)
    {
        // clang-format off
        const char* texviews[2] = {gStrCubeBackRTV, gStrCubeFrontRTV};
        const char* psostrs[2] = {gStrWorldPosBack, gStrWorldPosFront};
        // clang-format on
        for (int di = 0; di < 2; ++di)
        {
            ITextureView* pRTVs[1] = {m_TextureViews[texviews[di]]};
            auto          pDSV     = m_TextureViews[gStrCubeDepthDSV];

            SetRenderTargetsAttribs RTAttrs;
            RTAttrs.NumRenderTargets    = 1;
            RTAttrs.ppRenderTargets     = pRTVs;
            RTAttrs.pDepthStencil       = pDSV;
            RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

            // Clear the back buffer
            float4 ClearColor = {0.0f, 0.0f, 0.0f, 1.0f};
            m_pImmediateContext->ClearRenderTarget(pRTVs[0], ClearColor.Data(), RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
            m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            {
                MapHelper<WorldPosCameraCB> CBConstants(m_pImmediateContext, m_Buffers[gStrWorldPosCamera], MAP_WRITE, MAP_FLAG_DISCARD);
                *CBConstants = mWorldPosCameraCB;
            }

            const Uint64 offset   = 0;
            IBuffer*     pBuffs[] = {m_Buffers[gStrSkyBoxVB]};
            m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);
            m_pImmediateContext->SetIndexBuffer(m_Buffers[gStrSkyBoxIB], 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            m_pImmediateContext->SetPipelineState(m_pPSOs[psostrs[di]]);
            m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[psostrs[di]], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            DrawIndexedAttribs DrawAttrs;     // This is an indexed draw call
            DrawAttrs.IndexType  = VT_UINT32; // Index type
            DrawAttrs.NumIndices = 36;
            DrawAttrs.Flags      = DRAW_FLAG_VERIFY_ALL;
            m_pImmediateContext->DrawIndexed(DrawAttrs);
        }

        { 
            ITextureView* pRTVs[1] = {m_pSwapChain->GetCurrentBackBufferRTV()};
            auto*         pDSV     = m_pSwapChain->GetDepthBufferDSV();

            SetRenderTargetsAttribs RTAttrs;
            RTAttrs.NumRenderTargets    = 1;
            RTAttrs.ppRenderTargets     = pRTVs;
            RTAttrs.pDepthStencil       = pDSV;
            RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;
            m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

            m_pImmediateContext->ClearDepthStencil(pDSV, CLEAR_DEPTH_FLAG, 1.f, 0, RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            {
                MapHelper<VisualizerCameraCB> CBConstants(m_pImmediateContext, m_Buffers[gStrVoxelizationVisualizerCamera], MAP_WRITE, MAP_FLAG_DISCARD);
                *CBConstants = mVisualizerCameraCB;
            }
            {
                MapHelper<Voxelization_Settings> CBConstants(m_pImmediateContext, m_Buffers[gStrVoxelizationVisualizerSetting], MAP_WRITE, MAP_FLAG_DISCARD);
                *CBConstants = mVoxelSetting;
            }

            constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
            m_pImmediateContext->ClearRenderTarget(pRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

            const Uint64 offset   = 0;
            IBuffer*     pBuffs[] = {m_Buffers[gStrScreenVB]};
            m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

            m_pImmediateContext->SetPipelineState(m_pPSOs[gStrVoxelizationVisualizer]);

            m_ShaderResourceBindings[gStrVoxelizationVisualizer]->GetVariableByName(SHADER_TYPE_PIXEL, "TexVoxelgrid")->Set(m_TextureViews[gStrVoxelizationTex3DSRV]);
            m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrVoxelizationVisualizer], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

            DrawAttribs DrawAttrsQuad;
            DrawAttrsQuad.NumVertices = 3;
            DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
            m_pImmediateContext->Draw(DrawAttrsQuad); 
        }
        return;
    }
    
    // GBuffer
    if (true)
    {
        ITextureView*           pRTVs[4] = {m_TextureViews[gStrGBufferARTV], m_TextureViews[gStrGBufferBRTV], m_TextureViews[gStrGBufferCRTV], m_TextureViews[gStrGBufferDRTV]};
        SetRenderTargetsAttribs RTAttrs;
        RTAttrs.NumRenderTargets    = 4;
        RTAttrs.ppRenderTargets     = pRTVs;
        RTAttrs.pDepthStencil       = m_TextureViews[gStrGBufferDepthDSV];
        RTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(RTAttrs);

        constexpr float ClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pRTVs[0], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[1], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[2], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearRenderTarget(pRTVs[3], ClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);
        m_pImmediateContext->ClearDepthStencil(m_TextureViews[gStrGBufferDepthDSV], CLEAR_DEPTH_FLAG, 1.0f, 0, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrGBuffer]);

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
                        MapHelper<ObjectConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrGBufferCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mObjectCbData[idx++];
                    }
                    {
                        MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
                        *CBConstants = mPassCbData;
                    }

                    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrGBuffer], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

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

    // ConeTracing
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

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrScreenVB]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        {
            MapHelper<Cone_Tracing_Shader_Settings> CBConstants(m_pImmediateContext, m_Buffers[gStrConeTracingSettingCB], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mConeTracingSettings;
        }
        {
            MapHelper<PassConstants> CBConstants(m_pImmediateContext, m_Buffers[gStrCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *CBConstants = mPassCbData;
        }

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrConeTracing]);

        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "AlbedoTexture")->Set(m_TextureViews[gStrGBufferASRV]);
        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "NormalTexture")->Set(m_TextureViews[gStrGBufferBSRV]);
        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "RoughnessTexture")->Set(m_TextureViews[gStrGBufferCSRV]);
        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "DepthTexture")->Set(m_TextureViews[gStrGBufferDepthSRV]);
        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "ShadowMaps")->Set(m_TextureViews[gStrDepthShadowMapSRV]);
        m_ShaderResourceBindings[gStrConeTracing]->GetVariableByName(SHADER_TYPE_PIXEL, "VoxelGridTexture")->Set(m_TextureViews[gStrVoxelizationTex3DSRV]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrConeTracing], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }
}

float get_aperture(float degrees)
{
    static const float DEGREES_TO_RADIANS = PI_F / 180.0f;
    return tanf(DEGREES_TO_RADIANS * degrees * 0.5f);
}

void Tutorial36_VXGI::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState       = mouseState;
    }
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();


    mWorldPosCameraCB.viewPorj = view * proj;
    mWorldPosCameraCB.cameraPos = float4(m_Camera.GetPos(), 1.0f);

    mPassCbData.PreViewProj = mPassCbData.ViewProj;
    mPassCbData.View        = view;
    mPassCbData.Proj        = proj;
    mPassCbData.ViewProj    = view * proj;
    mPassCbData.InvView     = mPassCbData.View.Inverse();
    mPassCbData.InvProj     = mPassCbData.Proj.Inverse();
    mPassCbData.InvViewProj = mPassCbData.ViewProj.Inverse();

    mPassCbData.EyePosW          = m_Camera.GetPos();
    mPassCbData.RenderTargetSize = float4((float)mWidth, (float)mHeight, 1.0f / mWidth, 1.0f / mHeight);
    mPassCbData.NearZ            = gCamearNear;
    mPassCbData.FarZ             = gCamearFar;
    mPassCbData.TotalTime        = 0;
    mPassCbData.DeltaTime        = 0;
    mPassCbData.ViewPortSize     = {(float)mWidth, (float)mWidth, 0.0f, 0.0f};

    mPassCbData.AmbientLight = {0.25f, 0.25f, 0.35f, 1.0f};

    {
        static bool bInitLightSetting = false;
        if (!bInitLightSetting)
        {
            bInitLightSetting      = true;

            DirectionalLight light = {};
            light.mCol             = float4(1.0f, 1.0f, 1.0f, 5.0f);
            light.mPos             = float4(0.0f, 0.0f, 0.0f, 0.0f);
            light.mDir             = float4(-1.0f, -1.5f, 1.0f, 0.0f);
            light.mDir             = normalize(light.mDir);

            mPassCbData.DLights[0] = light;

            mPassCbData.amountOfDLights = 1;
        }

        float4x4 lightview = float4x4(0.0f, -0.969669282f, 0.244420782f, 0.0f,
                                      0.0f, 0.244420782f, 0.969669282f, 0.0f,
                                      -1.0f, 0.0f, 0.0f, 0.0f,
                                      0.0f, 0.0f, -0.249569640f, 1.0f);

        float4x4 LightProjectionMatrix = float4x4::OrthoOffCenter(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f, false);
        mPassCbData.LightView          = lightview;
        mPassCbData.LightProj          = LightProjectionMatrix;

        
        mPassCbData.LightViewProj = float4x4(0.0f, 0.0f, -0.125f, 0.0f,
                                         -0.12121f, 0.03055f, 0.0f, 0.0f,
                                         -0.01086f, -0.0431f, 0.00f, -0.10002f,
                                         0.0f, 0.0f, 0.0f, 1.0f);
        mPassCbData.LightViewProj = mPassCbData.LightViewProj.Transpose();
    }

    mDepthPassObject.world = float4x4::Scale(0.01f) * float4x4::Translation(float3(0.60519f, -6.51495f, 0.38691f));

    mVoxelSetting.visualize_mipmap_level = 0;
    mVisualizerCameraCB.u_camera_world_position = m_Camera.GetPos();

    // cone tracing setting
    static bool bInitTracingSetting = false;
    if (!bInitTracingSetting)
    {
        bInitTracingSetting = true;
        // clang-format off
        mConeTracingSettings.diffuse_settings = {get_aperture(60.00f), 0.100f, 3.9f, 2.0f, 1.0f, true};
        mConeTracingSettings.specular_settings = {get_aperture(0.700f), 1.000f, 1.0f, 2.0f, 1.0f, true};
        mConeTracingSettings.soft_shadows_settings = {get_aperture(1.293f), 0.359f, 10.7f, 2.0f, 1.0f, false};
        mConeTracingSettings.ao_settings           = { get_aperture(60.00f), 0.594f, 10.0f, 0.5f, 1.0f, true };
        // clang-format on
        mConeTracingSettings.scene_voxel_scale = float3(0.05106f, 0.12212f, 0.08303f);
        mConeTracingSettings.voxel_grid_resolution = 128;
        mConeTracingSettings.max_mipmap_level      = (int)log2(mConeTracingSettings.voxel_grid_resolution);
    }

    ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
    {
        if (ImGui::SliderFloat4("light dir", &mPassCbData.DLights[0].mDir.x, -1, 1))
        {
        }
        if (ImGui::SliderInt("direct light", &mConeTracingSettings.enable_direct_light, 0, 1))
        {
        }
        if (ImGui::SliderInt("diffuse enable", &mConeTracingSettings.diffuse_settings.is_enabled, 0, 1))
        {
        }
        if (ImGui::SliderInt("specular enable", &mConeTracingSettings.specular_settings.is_enabled, 0, 1))
        {
        }
        if (ImGui::SliderInt("soft shadow enable", &mConeTracingSettings.soft_shadows_settings.is_enabled, 0 ,1))
        {
        }
        if (ImGui::SliderInt("ao enable", &mConeTracingSettings.ao_settings.is_enabled, 0, 1))
        {
        }
        if (ImGui::SliderInt("hard shadowMap enable", &mConeTracingSettings.enable_hard_shadows, 0, 1))
        {
        }
    }
    ImGui::End();
}

void Tutorial36_VXGI::WindowResize(Uint32 Width, Uint32 Height)
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
