
#include "Tutorial27_SSR.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include "ShaderMacroHelper.hpp"
#include "GLTFLoader.hpp"

static const int      gBRDFIntegrationSize = 512;
static const uint32_t gSkyboxMips          = 11;
static const uint32_t gIrradianceSize      = 32;
static const uint32_t gSpecularSize        = 128;
static const uint32_t gSpecularMips        = 8; // (uint)log2(gSpecularSize) + 1;
static const uint32_t gSkyboxSize          = 1024;

static uint32_t gSSSR_MaxTravelsalIntersections = 128;
static uint32_t gSSSR_MinTravelsalOccupancy     = 4;
static uint32_t gSSSR_MostDetailedMip           = 1;
static float    pSSSR_TemporalStability         = 0.99f;
static float    gSSSR_DepthThickness            = 0.15f;
static int32_t  gSSSR_SamplesPerQuad            = 1;
static int32_t  gSSSR_EAWPassCount              = 1;
static bool     gSSSR_TemporalVarianceEnabled   = true;
static float    gSSSR_RougnessThreshold         = 0.1f;
static bool     gSSSR_SkipDenoiser              = false;

static bool gUseHolePatching          = true;
static bool gUseExpensiveHolePatching = true;

static bool gUseNormalMap  = false;
static bool gUseFadeEffect = true;

static uint32_t gRenderMode         = Diligent::SCENE_WITH_REFLECTIONS;
static uint32_t gReflectionType     = Diligent::PP_REFLECTION;
static uint32_t gLastReflectionType = gReflectionType;

static uint32_t gPlaneNumber   = 1;
static float    gPlaneSize     = 75.0f;
static float    gRRP_Intensity = 0.2f;

static Diligent::float3 gCamPos{20.0f, -2.0f, 0.9f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

static const char* gStrBRDFIntegration    = "BRDFIntegration";
static const char* gStrBRDFIntegrationUAV = "BRDFIntegrationUAV";
static const char* gStrBRDFIntegrationSRV = "BRDFIntegrationSRV";

static const char* gStrPanoSkyBox = "PanoSkyBox";
static const char* gStrSkyBox     = "SkyBox";
static const char* gStrSkyBoxUAV  = "SkyBoxUAV";
static const char* gStrSkyBoxSRV  = "SkyBoxSRV";
static const char* gStrSkyBoxCBV  = "SkyBoxRootConstant";

static const char* gStrIrradianceMap    = "IrradianceMap";
static const char* gStrIrradianceMapSRV = "IrradianceMapSRV";

static const char* gStrSpecular    = "Specular";
static const char* gStrSpecularSRV = "SpecularSRV";
static const char* gStrSpecularCBV = "SpecularRootConstant";

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

static const char* gStrGBufferCBV   = "GBufferCBV";
static const char* gStrCameraCBV    = "CameraCBV";

static const char* gStrRenderSceneBuffer    = "RenderSceneBuffer";
static const char* gStrRenderSceneBufferSRV = "RenderSceneBufferSRV";
static const char* gStrRenderSceneBufferRTV = "RenderSceneBufferRTV";

static const char* gStrPPRProjection              = "PPR_Projection";
static const char* gStrPPRProjectionCameraCBV     = "PPRProjectionCameraCBV";
static const char* gStrPPRProjectionPlaneCBV      = "PPRProjectionPlaneCBV";
static const char* gStrPPRProjectionCameraCompCBV = "PPRProjectionCameraCompCBV";
static const char* gStrPPRProjectionPlaneCompCBV  = "PPRProjectionPlaneCompCBV";

static const char* gStrPPRReflection              = "PPR_Reflection";
static const char* gStrPPRReflectionSRV           = "PPR_ReflectionBufferSRV";
static const char* gStrPPRReflectionRTV           = "PPR_ReflectionBufferRTV";
static const char* gStrPPRProjectionPropertiesCBV = "PPRProjectionPropertiesCBV";

static const char* gStrPPRHolepatching              = "PPRHolepatching";
static const char* gStrPPRHolepatchingCameraCompCBV = "PPRHolepatchingCameraCompCBV";
static const char* gStrPPRHolepatchingPropertiesCBV = "PPRHolepatchingPropertiesCBV";

namespace
{
// clang-format off
    float gScreenQuadPoints[] = {
		-1.0f, 3.0f, 0.5f, 0.0f, -1.0f,
		-1.0f, -1.0f, 0.5f, 0.0f, 1.0f,
		3.0f, -1.0f, 0.5f, 2.0f, 1.0f,
	};

	float gSkyBoxPoints[] = {
		0.5f,  -0.5f, -0.5f, 1.0f,    // -z
		-0.5f, -0.5f, -0.5f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f,
		0.5f,  0.5f,  -0.5f, 1.0f,
		0.5f,  -0.5f, -0.5f, 1.0f,

		-0.5f, -0.5f, 0.5f,  1.0f,    //-x
		-0.5f, -0.5f, -0.5f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f,
		-0.5f, 0.5f,  0.5f,  1.0f,
		-0.5f, -0.5f, 0.5f,  1.0f,

		0.5f,  -0.5f, -0.5f, 1.0f,    //+x
		0.5f,  -0.5f, 0.5f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		0.5f,  0.5f,  -0.5f, 1.0f,
		0.5f,  -0.5f, -0.5f, 1.0f,

		-0.5f, -0.5f, 0.5f,  1.0f,    // +z
		-0.5f, 0.5f,  0.5f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		0.5f,  -0.5f, 0.5f,  1.0f,
		-0.5f, -0.5f, 0.5f,  1.0f,

		-0.5f, 0.5f,  -0.5f, 1.0f,    //+y
		0.5f,  0.5f,  -0.5f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		0.5f,  0.5f,  0.5f,  1.0f,
		-0.5f, 0.5f,  0.5f,  1.0f,
		-0.5f, 0.5f,  -0.5f, 1.0f,

		0.5f,  -0.5f, 0.5f,  1.0f,    //-y
		0.5f,  -0.5f, -0.5f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f,
		-0.5f, -0.5f, -0.5f, 1.0f,
		-0.5f, -0.5f, 0.5f,  1.0f,
		0.5f,  -0.5f, 0.5f,  1.0f,
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
} // namespace

namespace Diligent
{

static TEXTURE_FORMAT gGBufferFormatA               = TEX_FORMAT_RGBA8_UNORM_SRGB;
static TEXTURE_FORMAT gGBufferFormatB               = TEX_FORMAT_RGBA16_FLOAT;
static TEXTURE_FORMAT gGBufferFormatC               = TEX_FORMAT_RGBA16_FLOAT;
static TEXTURE_FORMAT gGBufferFormatD               = TEX_FORMAT_RG16_FLOAT;
static TEXTURE_FORMAT gGBufferFormatDepth           = TEX_FORMAT_D32_FLOAT;
static TEXTURE_FORMAT gRenderSceneBufferFormatDepth = TEX_FORMAT_RGBA16_FLOAT;
static TEXTURE_FORMAT gPPRReflectionDepth           = TEX_FORMAT_RGBA16_FLOAT;

SampleBase* CreateSample()
{
    return new Tutorial27_SSR();
}


void Tutorial27_SSR::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    m_Camera.SetPos(gCamPos);
    m_Camera.SetRotation(PI_F / 2.f, 0);
    m_Camera.SetRotationSpeed(0.005f);
    m_Camera.SetMoveSpeed(5.f);
    m_Camera.SetSpeedUpScales(5.f, 10.f);

    CreatePipelineState();
    CreateBuffers();
    LoadTexture();
    LoadModel();
    PreDraw();
}

void Tutorial27_SSR::CreatePipelineState()
{
    { // pre draw
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
            ShaderCI.Desc.Name       = "BRDFIntegration CS";
            ShaderCI.FilePath        = "BRDFIntegration.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pBRDFIntegrationCS);
        }
        RefCntAutoPtr<IShader> pPanoToCubeCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "panoToCube CS";
            ShaderCI.FilePath        = "panoToCube.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPanoToCubeCS);
        }
        RefCntAutoPtr<IShader> pIrradianceMapCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "IrradianceMap CS";
            ShaderCI.FilePath        = "IrradianceMap.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pIrradianceMapCS);
        }
        RefCntAutoPtr<IShader> pSpecularMapCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "SpecularMap CS";
            ShaderCI.FilePath        = "SpecularMap.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pSpecularMapCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        // This is a compute pipeline
        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        PSODesc.Name      = gStrBRDFIntegration;
        PSOCreateInfo.pCS = pBRDFIntegrationCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrBRDFIntegration]);

        PSODesc.Name                            = gStrSkyBox;
        ShaderResourceVariableDesc SkyBoxVars[] = {
            {SHADER_TYPE_COMPUTE, "RootConstant", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "srcTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        SamplerDesc SamLinearWarpDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
            {
                {SHADER_TYPE_COMPUTE, "srcTexture", SamLinearWarpDesc}};
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = SkyBoxVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(SkyBoxVars);
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        PSOCreateInfo.pCS                                         = pPanoToCubeCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrSkyBox]);

        PSODesc.Name                                   = gStrIrradianceMap;
        ShaderResourceVariableDesc IrradianceMapVars[] = {
            {SHADER_TYPE_COMPUTE, "srcTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = IrradianceMapVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(IrradianceMapVars);
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        PSOCreateInfo.pCS                                         = pIrradianceMapCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrIrradianceMap]);

        PSODesc.Name                                 = gStrSpecular;
        ShaderResourceVariableDesc SpecularMapVars[] = {
            {SHADER_TYPE_COMPUTE, "RootConstant", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "srcTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = SpecularMapVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(SpecularMapVars);
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        PSOCreateInfo.pCS                                         = pSpecularMapCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrSpecular]);
    }

    if (true)
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
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "FillGbuffers VS";
            ShaderCI.FilePath        = "FillGbuffersVS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);

            CreateUniformBuffer(m_pDevice, sizeof(ObjectConstants), "cbPerObject CB", &m_Buffers[gStrGBufferCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(PassConstants), "cbPass CB", &m_Buffers[gStrCameraCBV]);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "FillGbuffers PS";
            ShaderCI.FilePath        = "FillGbuffersPS.hlsl";
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

    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                      = gStrRenderSceneBuffer;
        PSOCreateInfo.PSODesc.PipelineType              = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]    = gRenderSceneBufferFormatDepth;
        //PSOCreateInfo.GraphicsPipeline.DSVFormat                         = gGBufferFormatDepth;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

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
            ShaderCI.Desc.Name       = "renderSceneBRDF VS";
            ShaderCI.FilePath        = "renderSceneBRDFVS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "renderSceneBRDF PS";
            ShaderCI.FilePath        = "renderSceneBRDFpS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False},
            };

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbPass", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "girradianceMap", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "gSpecularMap", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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
                {SHADER_TYPE_PIXEL, "girradianceMap", SamLinearWrapDesc},
                {SHADER_TYPE_PIXEL, "gSpecularMap", SamLinearWrapDesc},
                {SHADER_TYPE_PIXEL, "textureMaps", SamLinearWrapDesc},
            };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrRenderSceneBuffer]);

        m_pPSOs[gStrRenderSceneBuffer]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbPass")->Set(m_Buffers[gStrCameraCBV]);
        m_pPSOs[gStrRenderSceneBuffer]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrRenderSceneBuffer], true);
    }

    { // SSR
        ShaderCreateInfo ShaderCI;
        ShaderCI.SourceLanguage                  = SHADER_SOURCE_LANGUAGE_HLSL;
        ShaderCI.Desc.UseCombinedTextureSamplers = true;

        // Create a shader source stream factory to load shaders from files.
        RefCntAutoPtr<IShaderSourceInputStreamFactory> pShaderSourceFactory;
        m_pEngineFactory->CreateDefaultShaderSourceStreamFactory(nullptr, &pShaderSourceFactory);
        ShaderCI.pShaderSourceStreamFactory = pShaderSourceFactory;

        RefCntAutoPtr<IShader> pPPRProjectionCS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_COMPUTE;
            ShaderCI.EntryPoint      = "csmain";
            ShaderCI.Desc.Name       = "PPR_Projection CS";
            ShaderCI.FilePath        = "PPR_Projection.hlsl";
            //ShaderCI.HLSLVersion     = {6, 1};
            //ShaderCI.ShaderCompiler  = SHADER_COMPILER_DXC;
            m_pDevice->CreateShader(ShaderCI, &pPPRProjectionCS);
        }

        ComputePipelineStateCreateInfo PSOCreateInfo;
        PipelineStateDesc&             PSODesc = PSOCreateInfo.PSODesc;

        PSODesc.PipelineType                       = PIPELINE_TYPE_COMPUTE;
        PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE;

        // clang-format off
        ShaderResourceVariableDesc ProjectionVars[] = {
            {SHADER_TYPE_COMPUTE, "cbExtendCamera", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "planeInfoBuffer", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_COMPUTE, "DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_COMPUTE, "IntermediateBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        SamplerDesc SamLinearWarpDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_COMPUTE, "DepthTexture", SamLinearWarpDesc}
        };
        // clang-format on
        PSOCreateInfo.PSODesc.ResourceLayout.Variables            = ProjectionVars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables         = _countof(ProjectionVars);
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        PSODesc.Name      = gStrPPRProjection;
        PSOCreateInfo.pCS = pPPRProjectionCS;
        m_pDevice->CreateComputePipelineState(PSOCreateInfo, &m_pPSOs[gStrPPRProjection]);
    }

    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                      = gStrPPRReflection;
        PSOCreateInfo.PSODesc.PipelineType              = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]    = gPPRReflectionDepth;
        //PSOCreateInfo.GraphicsPipeline.DSVFormat                         = gGBufferFormatDepth;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

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
            ShaderCI.Desc.Name       = "PPR_Reflection VS";
            ShaderCI.FilePath        = "PPR_ReflectionVS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "PPR_Reflection PS";
            ShaderCI.FilePath        = "PPR_ReflectionPS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(UniformExtendedCamData), "cbExtendCamera CB", &m_Buffers[gStrPPRProjectionCameraCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(UniformPlaneInfoData), "planeInfoBuffer CB", &m_Buffers[gStrPPRProjectionPlaneCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(cbProperties), "cbProperties CB", &m_Buffers[gStrPPRProjectionPropertiesCBV]);
        }

        LayoutElement LayoutElems[] =
            {
                // Attribute 0 - vertex position
                LayoutElement{0, 0, 3, VT_FLOAT32, False},
                // Attribute 1 - texture coordinates
                LayoutElement{1, 0, 2, VT_FLOAT32, False},
            };

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbExtendCamera", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "planeInfoBuffer", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbProperties", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "SceneTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "IntermediateBuffer", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "DepthTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
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
                {SHADER_TYPE_PIXEL, "SceneTexture", SamLinearWrapDesc},
                {SHADER_TYPE_PIXEL, "DepthTexture", SamLinearWrapDesc},
            };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrPPRReflection]);

        m_pPSOs[gStrPPRReflection]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbExtendCamera")->Set(m_Buffers[gStrPPRProjectionCameraCBV]);
        m_pPSOs[gStrPPRReflection]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "planeInfoBuffer")->Set(m_Buffers[gStrPPRProjectionPlaneCBV]);
        m_pPSOs[gStrPPRReflection]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbProperties")->Set(m_Buffers[gStrPPRProjectionPropertiesCBV]);
        m_pPSOs[gStrPPRReflection]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrPPRReflection], true);
    }

    if (true)
    {
        GraphicsPipelineStateCreateInfo PSOCreateInfo;

        PSOCreateInfo.PSODesc.Name                                       = gStrPPRHolepatching;
        PSOCreateInfo.PSODesc.PipelineType                               = PIPELINE_TYPE_GRAPHICS;
        PSOCreateInfo.GraphicsPipeline.NumRenderTargets                  = 1;
        PSOCreateInfo.GraphicsPipeline.RTVFormats[0]                     = m_pSwapChain->GetDesc().ColorBufferFormat;
        PSOCreateInfo.GraphicsPipeline.PrimitiveTopology                 = PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        PSOCreateInfo.GraphicsPipeline.RasterizerDesc.CullMode           = CULL_MODE_NONE;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthEnable      = false;
        PSOCreateInfo.GraphicsPipeline.DepthStencilDesc.DepthWriteEnable = false;

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
            ShaderCI.Desc.Name       = "PPR_Holepatching VS";
            ShaderCI.FilePath        = "PPR_HolepatchingVS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pVS);
        }

        // Create a pixel shader
        RefCntAutoPtr<IShader> pPS;
        {
            ShaderCI.Desc.ShaderType = SHADER_TYPE_PIXEL;
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "PPR_Holepatching PS";
            ShaderCI.FilePath        = "PPR_HolepatchingPS.hlsl";
            m_pDevice->CreateShader(ShaderCI, &pPS);

            CreateUniformBuffer(m_pDevice, sizeof(UniformExtendedCamData), "cbExtendCamera CB", &m_Buffers[gStrPPRHolepatchingCameraCompCBV]);
            CreateUniformBuffer(m_pDevice, sizeof(cbProperties), "cbProperties CB", &m_Buffers[gStrPPRHolepatchingPropertiesCBV]);
        }

        // clang-format off
        LayoutElement LayoutElems[] =
        {
            // Attribute 0 - vertex position
            LayoutElement{0, 0, 3, VT_FLOAT32, False},
            // Attribute 1 - texture coordinates
            LayoutElement{1, 0, 2, VT_FLOAT32, False},
        };
        // clang-format on

        PSOCreateInfo.pVS = pVS;
        PSOCreateInfo.pPS = pPS;

        PSOCreateInfo.GraphicsPipeline.InputLayout.LayoutElements = LayoutElems;
        PSOCreateInfo.GraphicsPipeline.InputLayout.NumElements    = _countof(LayoutElems);

        // Define variable type that will be used by default
        PSOCreateInfo.PSODesc.ResourceLayout.DefaultVariableType = SHADER_RESOURCE_VARIABLE_TYPE_STATIC;

        // clang-format off
        ShaderResourceVariableDesc Vars[] = 
        {
            {SHADER_TYPE_PIXEL, "cbExtendCamera", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "cbProperties", SHADER_RESOURCE_VARIABLE_TYPE_STATIC},
            {SHADER_TYPE_PIXEL, "SceneTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "SSRTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
            {SHADER_TYPE_PIXEL, "SSRPointTexture", SHADER_RESOURCE_VARIABLE_TYPE_MUTABLE},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.Variables    = Vars;
        PSOCreateInfo.PSODesc.ResourceLayout.NumVariables = _countof(Vars);

        // Define immutable sampler for g_Texture. Immutable samplers should be used whenever possible
        SamplerDesc SamLinearWrapDesc{
            FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR, FILTER_TYPE_LINEAR,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        SamplerDesc SamPointWrapDesc{
            FILTER_TYPE_POINT, FILTER_TYPE_POINT, FILTER_TYPE_POINT,
            TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP, TEXTURE_ADDRESS_WRAP};
        ImmutableSamplerDesc ImtblSamplers[] =
        {
            {SHADER_TYPE_PIXEL, "SceneTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "SSRTexture", SamLinearWrapDesc},
            {SHADER_TYPE_PIXEL, "SSRPointTexture", SamPointWrapDesc},
        };
        PSOCreateInfo.PSODesc.ResourceLayout.ImmutableSamplers    = ImtblSamplers;
        PSOCreateInfo.PSODesc.ResourceLayout.NumImmutableSamplers = _countof(ImtblSamplers);
        // clang-format on

        m_pDevice->CreateGraphicsPipelineState(PSOCreateInfo, &m_pPSOs[gStrPPRHolepatching]);

        m_pPSOs[gStrPPRHolepatching]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbExtendCamera")->Set(m_Buffers[gStrPPRProjectionCameraCBV]);
        m_pPSOs[gStrPPRHolepatching]->GetStaticVariableByName(SHADER_TYPE_PIXEL, "cbProperties")->Set(m_Buffers[gStrPPRProjectionPropertiesCBV]);
        m_pPSOs[gStrPPRHolepatching]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrPPRHolepatching], true);
    }
}

void Tutorial27_SSR::CreateBuffers()
{
    { // BRDF Integration
        TextureDesc BRDFIntegrationTexDesc;
        BRDFIntegrationTexDesc.Name      = "BRDF Integration Tex";
        BRDFIntegrationTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        BRDFIntegrationTexDesc.Width     = gBRDFIntegrationSize;
        BRDFIntegrationTexDesc.Height    = gBRDFIntegrationSize;
        BRDFIntegrationTexDesc.MipLevels = 1;
        BRDFIntegrationTexDesc.Format    = TEX_FORMAT_RG32_FLOAT;
        BRDFIntegrationTexDesc.Usage     = USAGE_DEFAULT;
        BRDFIntegrationTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(BRDFIntegrationTexDesc, nullptr, &m_Textures[BRDFIntegrationTexDesc.Name]);
        m_TextureViews[gStrBRDFIntegrationSRV]    = m_Textures[BRDFIntegrationTexDesc.Name]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        ITextureView* pBRDFIntegrationBuffDescUAV = m_Textures[BRDFIntegrationTexDesc.Name]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);

        m_pPSOs[gStrBRDFIntegration]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrBRDFIntegrationUAV], true);
        m_ShaderResourceBindings[gStrBRDFIntegrationUAV]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->Set(pBRDFIntegrationBuffDescUAV);
    }

    { // Sky Box Tex
        BufferDesc BuffDesc;
        BuffDesc.Name           = "panToCube Constants buffer";
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(Uint32) * 2;
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrSkyBoxCBV]);
        m_pPSOs[gStrSkyBox]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "RootConstant")->Set(m_Buffers[gStrSkyBoxCBV]);
        m_pPSOs[gStrSkyBox]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSkyBox], true);

        TextureDesc CubeTexDesc;
        CubeTexDesc.Name      = "Sky Box Tex";
        CubeTexDesc.Type      = RESOURCE_DIM_TEX_CUBE;
        CubeTexDesc.Width     = gSkyboxSize;
        CubeTexDesc.Height    = gSkyboxSize;
        CubeTexDesc.MipLevels = gSkyboxMips;
        CubeTexDesc.ArraySize = 6;
        CubeTexDesc.Format    = TEX_FORMAT_RGBA32_FLOAT;
        CubeTexDesc.Usage     = USAGE_DEFAULT;
        CubeTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(CubeTexDesc, nullptr, &m_Textures[CubeTexDesc.Name]);
        m_TextureViews[gStrSkyBoxSRV]         = m_Textures[CubeTexDesc.Name]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        ITextureView* pSkyBoxUAV[gSkyboxMips] = {};
        for (int i = 0; i < gSkyboxMips; ++i)
        {
            TextureViewDesc ViewDesc;
            ViewDesc.TextureDim      = RESOURCE_DIM_TEX_2D_ARRAY;
            ViewDesc.ViewType        = TEXTURE_VIEW_UNORDERED_ACCESS;
            ViewDesc.AccessFlags     = UAV_ACCESS_FLAG_READ | UAV_ACCESS_FLAG_WRITE;
            ViewDesc.Format          = TEX_FORMAT_RGBA32_FLOAT;
            ViewDesc.MostDetailedMip = i;
            ViewDesc.NumArraySlices  = 6;
            m_Textures[CubeTexDesc.Name]->CreateView(ViewDesc, &pSkyBoxUAV[i]);
        }

        //ITextureView* pUAV = m_Textures[CubeTexDesc.Name]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);
        //m_ShaderResourceBindings[gStrSkyBox]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->Set(pUAV);
        m_ShaderResourceBindings[gStrSkyBox]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->SetArray((IDeviceObject* const*)pSkyBoxUAV, 0, gSkyboxMips);
    }

    { // IrradianceMap
        TextureDesc IrradianceMapDesc;
        IrradianceMapDesc.Name      = gStrIrradianceMap;
        IrradianceMapDesc.Type      = RESOURCE_DIM_TEX_CUBE;
        IrradianceMapDesc.Type      = RESOURCE_DIM_TEX_CUBE;
        IrradianceMapDesc.Width     = gIrradianceSize;
        IrradianceMapDesc.Height    = gIrradianceSize;
        IrradianceMapDesc.MipLevels = 1;
        IrradianceMapDesc.ArraySize = 6;
        IrradianceMapDesc.Format    = TEX_FORMAT_RGBA32_FLOAT;
        IrradianceMapDesc.Usage     = USAGE_DEFAULT;
        IrradianceMapDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(IrradianceMapDesc, nullptr, &m_Textures[IrradianceMapDesc.Name]);
        m_TextureViews[gStrIrradianceMapSRV] = m_Textures[gStrIrradianceMap]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        ITextureView* pIrradianceMapUAV      = m_Textures[gStrIrradianceMap]->GetDefaultView(TEXTURE_VIEW_UNORDERED_ACCESS);

        m_pPSOs[gStrIrradianceMap]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrIrradianceMap], true);
        m_ShaderResourceBindings[gStrIrradianceMap]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->Set(pIrradianceMapUAV);
        m_ShaderResourceBindings[gStrIrradianceMap]->GetVariableByName(SHADER_TYPE_COMPUTE, "srcTexture")->Set(m_TextureViews[gStrSkyBoxSRV]);
    }

    { // Specular Map
        BufferDesc BuffDesc;
        BuffDesc.Name           = "Specular Constants buffer";
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(Uint32) + sizeof(float);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrSpecularCBV]);
        m_pPSOs[gStrSpecular]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "RootConstant")->Set(m_Buffers[gStrSpecularCBV]);
        m_pPSOs[gStrSpecular]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrSpecular], true);

        TextureDesc CubeTexDesc;
        CubeTexDesc.Name      = "Specular Tex";
        CubeTexDesc.Type      = RESOURCE_DIM_TEX_CUBE;
        CubeTexDesc.Width     = gSpecularSize;
        CubeTexDesc.Height    = gSpecularSize;
        CubeTexDesc.MipLevels = gSpecularMips;
        CubeTexDesc.ArraySize = 6;
        CubeTexDesc.Format    = TEX_FORMAT_RGBA32_FLOAT;
        CubeTexDesc.Usage     = USAGE_DEFAULT;
        CubeTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        m_pDevice->CreateTexture(CubeTexDesc, nullptr, &m_Textures[CubeTexDesc.Name]);
        m_TextureViews[gStrSpecularSRV]           = m_Textures[CubeTexDesc.Name]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
        ITextureView* pSpecularUAV[gSpecularMips] = {};
        for (int i = 0; i < gSpecularMips; ++i)
        {
            TextureViewDesc ViewDesc;
            ViewDesc.TextureDim      = RESOURCE_DIM_TEX_2D_ARRAY;
            ViewDesc.ViewType        = TEXTURE_VIEW_UNORDERED_ACCESS;
            ViewDesc.AccessFlags     = UAV_ACCESS_FLAG_READ | UAV_ACCESS_FLAG_WRITE;
            ViewDesc.Format          = TEX_FORMAT_RGBA32_FLOAT;
            ViewDesc.MostDetailedMip = i;
            ViewDesc.NumArraySlices  = 6;
            m_Textures[CubeTexDesc.Name]->CreateView(ViewDesc, &pSpecularUAV[i]);
        }

        m_ShaderResourceBindings[gStrSpecular]->GetVariableByName(SHADER_TYPE_COMPUTE, "dstTexture")->SetArray((IDeviceObject* const*)pSpecularUAV, 0, gSpecularMips);
        m_ShaderResourceBindings[gStrSpecular]->GetVariableByName(SHADER_TYPE_COMPUTE, "srcTexture")->Set(m_TextureViews[gStrSkyBoxSRV]);
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

        TextureDesc GRenderSceneTexDesc;
        GRenderSceneTexDesc.Name      = gStrRenderSceneBuffer;
        GRenderSceneTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GRenderSceneTexDesc.Width     = mWidth;
        GRenderSceneTexDesc.Height    = mHeight;
        GRenderSceneTexDesc.MipLevels = 1;
        GRenderSceneTexDesc.Format    = gRenderSceneBufferFormatDepth;
        GRenderSceneTexDesc.Usage     = USAGE_DEFAULT;
        GRenderSceneTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GRenderSceneTexDesc, nullptr, &m_Textures[gStrRenderSceneBuffer]);
        m_TextureViews[gStrRenderSceneBufferRTV] = m_Textures[gStrRenderSceneBuffer]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrRenderSceneBufferSRV] = m_Textures[gStrRenderSceneBuffer]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

        TextureDesc GPPRReflectionTexDesc;
        GPPRReflectionTexDesc.Name      = gStrPPRReflection;
        GPPRReflectionTexDesc.Type      = RESOURCE_DIM_TEX_2D;
        GPPRReflectionTexDesc.Width     = mWidth;
        GPPRReflectionTexDesc.Height    = mHeight;
        GPPRReflectionTexDesc.MipLevels = 1;
        GPPRReflectionTexDesc.Format    = gRenderSceneBufferFormatDepth;
        GPPRReflectionTexDesc.Usage     = USAGE_DEFAULT;
        GPPRReflectionTexDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_RENDER_TARGET;
        m_pDevice->CreateTexture(GPPRReflectionTexDesc, nullptr, &m_Textures[gStrPPRReflection]);
        m_TextureViews[gStrPPRReflectionRTV] = m_Textures[gStrPPRReflection]->GetDefaultView(TEXTURE_VIEW_RENDER_TARGET);
        m_TextureViews[gStrPPRReflectionSRV] = m_Textures[gStrPPRReflection]->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);
    }

    {
        BufferDesc VertBuffDesc;
        VertBuffDesc.Name      = "ScreenQuad vertex buffer";
        VertBuffDesc.Usage     = USAGE_IMMUTABLE;
        VertBuffDesc.BindFlags = BIND_VERTEX_BUFFER;
        VertBuffDesc.Size      = sizeof(gScreenQuadPoints);
        BufferData VBData;
        VBData.pData    = gScreenQuadPoints;
        VBData.DataSize = sizeof(gScreenQuadPoints);
        m_pDevice->CreateBuffer(VertBuffDesc, &VBData, &m_Buffers[gStrRenderSceneBuffer]);
    }

    {
        BufferDesc BuffDesc;
        BuffDesc.Name           = "PPR_Projection Camera buffer";
        BuffDesc.Usage          = USAGE_DYNAMIC;
        BuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        BuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        BuffDesc.Size           = sizeof(UniformExtendedCamData);
        m_pDevice->CreateBuffer(BuffDesc, nullptr, &m_Buffers[gStrPPRProjectionCameraCompCBV]);

        BufferDesc PlaneBuffDesc;
        PlaneBuffDesc.Name           = "PPR_Projection Plane buffer";
        PlaneBuffDesc.Usage          = USAGE_DYNAMIC;
        PlaneBuffDesc.BindFlags      = BIND_UNIFORM_BUFFER;
        PlaneBuffDesc.CPUAccessFlags = CPU_ACCESS_WRITE;
        PlaneBuffDesc.Size           = sizeof(UniformPlaneInfoData);
        m_pDevice->CreateBuffer(PlaneBuffDesc, nullptr, &m_Buffers[gStrPPRProjectionPlaneCompCBV]);

        m_pPSOs[gStrPPRProjection]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "cbExtendCamera")->Set(m_Buffers[gStrPPRProjectionCameraCompCBV]);
        m_pPSOs[gStrPPRProjection]->GetStaticVariableByName(SHADER_TYPE_COMPUTE, "planeInfoBuffer")->Set(m_Buffers[gStrPPRProjectionPlaneCompCBV]);
        m_pPSOs[gStrPPRProjection]->CreateShaderResourceBinding(&m_ShaderResourceBindings[gStrPPRProjection], true);
    }

    {
        std::vector<uint32_t> gInitializeVal;
        gInitializeVal.reserve(mWidth * mHeight);
        for (uint32_t i = 0; i < mWidth * mHeight; i++)
        {
            gInitializeVal.push_back(UINT32_MAX);
        }

        BufferDesc IntermediateBuffDesc;
        IntermediateBuffDesc.Name              = "IntermediateBuffer buffer";
        IntermediateBuffDesc.Usage             = USAGE_DEFAULT;
        IntermediateBuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        IntermediateBuffDesc.Mode              = BUFFER_MODE_FORMATTED;
        IntermediateBuffDesc.ElementByteStride = sizeof(int);
        IntermediateBuffDesc.Size              = sizeof(int) * mWidth * mHeight;
        BufferData BData;
        BData.pData    = gInitializeVal.data();
        BData.DataSize = IntermediateBuffDesc.Size;
        m_pDevice->CreateBuffer(IntermediateBuffDesc, &BData, &m_Buffers[gStrPPRProjection]);

        RefCntAutoPtr<IBufferView> pUAV;
        RefCntAutoPtr<IBufferView> pSRV;
        {
            BufferViewDesc ViewDesc;
            ViewDesc.ViewType             = BUFFER_VIEW_UNORDERED_ACCESS;
            ViewDesc.Format.ValueType     = VT_INT32;
            ViewDesc.Format.NumComponents = 1;
            m_Buffers[gStrPPRProjection]->CreateView(ViewDesc, &pUAV);

            ViewDesc.ViewType = BUFFER_VIEW_SHADER_RESOURCE;
            m_Buffers[gStrPPRProjection]->CreateView(ViewDesc, &pSRV);
        }
        m_ShaderResourceBindings[gStrPPRProjection]->GetVariableByName(SHADER_TYPE_COMPUTE, "IntermediateBuffer")->Set(pUAV);
        m_ShaderResourceBindings[gStrPPRProjection]->GetVariableByName(SHADER_TYPE_COMPUTE, "DepthTexture")->Set(m_TextureViews[gStrGBufferDepthSRV]);

        m_ShaderResourceBindings[gStrPPRReflection]->GetVariableByName(SHADER_TYPE_PIXEL, "DepthTexture")->Set(m_TextureViews[gStrGBufferDepthSRV]);
        m_ShaderResourceBindings[gStrPPRReflection]->GetVariableByName(SHADER_TYPE_PIXEL, "SceneTexture")->Set(m_TextureViews[gStrRenderSceneBufferSRV]);
        m_ShaderResourceBindings[gStrPPRReflection]->GetVariableByName(SHADER_TYPE_PIXEL, "IntermediateBuffer")->Set(pUAV);

        m_ShaderResourceBindings[gStrPPRHolepatching]->GetVariableByName(SHADER_TYPE_PIXEL, "SceneTexture")->Set(m_TextureViews[gStrRenderSceneBufferSRV]);
        m_ShaderResourceBindings[gStrPPRHolepatching]->GetVariableByName(SHADER_TYPE_PIXEL, "SSRTexture")->Set(m_TextureViews[gStrPPRReflectionSRV]);
        m_ShaderResourceBindings[gStrPPRHolepatching]->GetVariableByName(SHADER_TYPE_PIXEL, "SSRPointTexture")->Set(m_TextureViews[gStrPPRReflectionSRV]);
    }
}

void Tutorial27_SSR::LoadTexture()
{
    TextureLoadInfo loadInfo;
    loadInfo.IsSRGB = true;
    RefCntAutoPtr<ITexture> Tex;
    CreateTextureFromFile("LA_Helipad.dds", loadInfo, m_pDevice, &Tex);
    // Get shader resource view from the texture
    m_TextureViews[gStrPanoSkyBox] = Tex->GetDefaultView(TEXTURE_VIEW_SHADER_RESOURCE);

    // Set texture SRV in the SRB
    m_ShaderResourceBindings[gStrSkyBox]->GetVariableByName(SHADER_TYPE_COMPUTE, "srcTexture")->Set(m_TextureViews[gStrPanoSkyBox]);

    ITextureView* pMeshTex[100] = {};
    int           texcnt        = 84;
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
    pMeshTex[texcnt++] = m_TextureViews[gStrGBufferASRV];
    pMeshTex[texcnt++] = m_TextureViews[gStrGBufferBSRV];
    pMeshTex[texcnt++] = m_TextureViews[gStrGBufferCSRV];
    pMeshTex[texcnt++] = m_TextureViews[gStrGBufferDSRV];
    pMeshTex[texcnt++] = m_TextureViews[gStrGBufferDepthSRV];
    pMeshTex[texcnt++] = m_TextureViews[gStrBRDFIntegrationSRV];

    m_ShaderResourceBindings[gStrGBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "textureMaps")->SetArray((IDeviceObject* const*)pMeshTex, 0, 84);
    m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "textureMaps")->SetArray((IDeviceObject* const*)pMeshTex, 0, texcnt);
    m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "girradianceMap")->Set(m_TextureViews[gStrIrradianceMapSRV]);
    m_ShaderResourceBindings[gStrRenderSceneBuffer]->GetVariableByName(SHADER_TYPE_PIXEL, "gSpecularMap")->Set(m_TextureViews[gStrSpecularSRV]);
}

void Tutorial27_SSR::LoadModel()
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
    ModelCI.FileName             = "Models/Sponza.gltf";
    ModelCI.pResourceManager     = nullptr;
    ModelCI.ComputeBoundingBoxes = false;
    ModelCI.VertexAttributes     = DefaultVertexAttributes.data();
    ModelCI.NumVertexAttributes  = (Uint32)DefaultVertexAttributes.size();
    m_Model1                     = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);

    ModelCI.FileName = "Models/lion.gltf";
    m_Model2         = new GLTF::Model(m_pDevice, m_pImmediateContext, ModelCI);

    size_t subMeshCnt1 = m_Model1->Scenes[m_Model1->DefaultSceneId].LinearNodes[0]->pMesh->Primitives.size();
    size_t subMeshCnt2 = m_Model2->Scenes[m_Model2->DefaultSceneId].LinearNodes[0]->pMesh->Primitives.size();
    mObjectCbData.resize(subMeshCnt1 + subMeshCnt2);

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

        if (i >= subMeshCnt1)
        {
            DiffuseMapIndex = ((81 & 0xFF) << 0) | ((83 & 0xFF) << 8) | ((6 & 0xFF) << 16) | ((6 & 0xFF) << 24);

            obcs.World = float4x4::Scale(float3(0.2f, 0.2f, -0.2f)) * float4x4::RotationY(-1.5708f) * float4x4::Translation(0.0f, -6.0f, 1.0f);
        }
        else
        {
            uint materialID = gMaterialIds[i];
            materialID *= 5; //because it uses 5 basic textures for redering BRDF
            DiffuseMapIndex = ((gSponzaTextureIndexforMaterial[materialID + 0] & 0xFF) << 0) |
                ((gSponzaTextureIndexforMaterial[materialID + 1] & 0xFF) << 8) |
                ((gSponzaTextureIndexforMaterial[materialID + 2] & 0xFF) << 16) |
                ((gSponzaTextureIndexforMaterial[materialID + 3] & 0xFF) << 24);

            obcs.World = float4x4::Scale(0.02f) * float4x4::Translation(float3(0.0f, -6.0f, 0.0f));
        }

        obcs.MaterialIndex = DiffuseMapIndex;
    }
}

void Tutorial27_SSR::PreDraw()
{
    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrBRDFIntegration]);
    DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = (Uint32)ceilf(gBRDFIntegrationSize / 16.0f);
    DispatAttribs.ThreadGroupCountY = (Uint32)ceilf(gBRDFIntegrationSize / 16.0f);
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrBRDFIntegrationUAV], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribs);

    struct Constants
    {
        Uint32 mip;
        Uint32 maxSize;
    };
    static const Uint32 gThreadGroupSizeX = 16;
    static const Uint32 gThreadGroupSizeY = 16;
    // Map the buffer and write current world-view-projection matrix
    for (int i = 0; i < gSkyboxMips; ++i)
    {
        {
            MapHelper<Constants> ConstData(m_pImmediateContext, m_Buffers[gStrSkyBoxCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            ConstData->mip     = static_cast<Uint32>(i);
            ConstData->maxSize = gSkyboxSize;
        }

        DispatchComputeAttribs DispatAttribsToCube;
        DispatAttribsToCube.ThreadGroupCountX = std::max(1u, (Uint32)((gSkyboxSize >> i) / gThreadGroupSizeX));
        DispatAttribsToCube.ThreadGroupCountY = std::max(1u, (Uint32)((gSkyboxSize >> i) / gThreadGroupSizeY));
        DispatAttribsToCube.ThreadGroupCountZ = (Uint32)6;

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSkyBox]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSkyBox], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribsToCube);
    }

    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrIrradianceMap]);
    DispatchComputeAttribs DispatAttribsIrradianceMap;
    DispatAttribsIrradianceMap.ThreadGroupCountX = (Uint32)ceilf(gIrradianceSize / 16.0f);
    DispatAttribsIrradianceMap.ThreadGroupCountY = (Uint32)ceilf(gIrradianceSize / 16.0f);
    DispatAttribsIrradianceMap.ThreadGroupCountZ = (Uint32)6;
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrIrradianceMap], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribsIrradianceMap);

    struct SpecularConstants
    {
        Uint32 mip;
        float  roughness;
    };
    // Map the buffer and write current world-view-projection matrix
    for (int i = 0; i < gSpecularMips; ++i)
    {
        {
            MapHelper<SpecularConstants> ConstData(m_pImmediateContext, m_Buffers[gStrSpecularCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            ConstData->mip       = static_cast<Uint32>(i);
            ConstData->roughness = (float)i / (float)(gSpecularMips - 1);
        }

        DispatchComputeAttribs DispatAttribsSpecular;
        DispatAttribsSpecular.ThreadGroupCountX = std::max(1u, (Uint32)((gSpecularSize >> i) / gThreadGroupSizeX));
        DispatAttribsSpecular.ThreadGroupCountY = std::max(1u, (Uint32)((gSpecularSize >> i) / gThreadGroupSizeY));
        DispatAttribsSpecular.ThreadGroupCountZ = (Uint32)6;

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrSpecular]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrSpecular], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribsSpecular);
    }
}

void Tutorial27_SSR::Render()
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
    std::array<GLTF::Model*, 2> models = {m_Model1, m_Model2};
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

    { // render brdf
        ITextureView*           pQuadRTVs[1] = {m_TextureViews[gStrRenderSceneBufferRTV]};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrRenderSceneBuffer]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrRenderSceneBuffer], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }

    UniformExtendedCamData gUniformExtendedCamData;
    gUniformExtendedCamData.mCameraWorldPos = float4(gCamPos, 1.0);
    gUniformExtendedCamData.mViewMat        = mPassCbData.View;
    gUniformExtendedCamData.mProjMat        = mPassCbData.Proj;
    gUniformExtendedCamData.mViewProjMat    = mPassCbData.ViewProj;
    gUniformExtendedCamData.mInvViewProjMat = mPassCbData.InvViewProj;
    gUniformExtendedCamData.mViewPortSize   = float4(static_cast<float>(mWidth), static_cast<float>(mHeight), 0.0, 0.0);

    UniformPlaneInfoData gUniformPlaneInfoData;
    gUniformPlaneInfoData.numPlanes                = gPlaneNumber;
    gUniformPlaneInfoData.planeInfo[0].centerPoint = float4(0.0, -6.0f, 0.9f, 0.0);
    gUniformPlaneInfoData.planeInfo[0].size        = float4(gPlaneSize);
    gUniformPlaneInfoData.planeInfo[1].centerPoint = float4(10.0, -5.0f, -1.25f, 0.0);
    gUniformPlaneInfoData.planeInfo[1].size        = float4(9.0f, 2.0f, 0.0f, 0.0f);
    gUniformPlaneInfoData.planeInfo[2].centerPoint = float4(10.0, -5.0f, 3.0f, 0.0);
    gUniformPlaneInfoData.planeInfo[2].size        = float4(9.0f, 2.0f, 0.0f, 0.0f);
    gUniformPlaneInfoData.planeInfo[3].centerPoint = float4(10.0, 1.0f, 0.9f, 0.0);
    gUniformPlaneInfoData.planeInfo[3].size        = float4(10.0f);
    float4x4 basicMat                              = float4x4(1.0, 0.0, 0.0, 0.0,  //tan
                                                              0.0, 0.0, -1.0, 0.0, //bitan
                                                              0.0, 1.0, 0.0, 0.0,  //normal
                                                              0.0, 0.0, 0.0, 1.0);
    gUniformPlaneInfoData.planeInfo[0].rotMat      = basicMat;
    gUniformPlaneInfoData.planeInfo[1].rotMat      = float4x4::RotationX(0.01745329251994329576923690768489f * -80.0f);
    gUniformPlaneInfoData.planeInfo[2].rotMat      = float4x4::RotationX(0.01745329251994329576923690768489f * -100.0f);
    gUniformPlaneInfoData.planeInfo[3].rotMat      = float4x4::RotationX(0.01745329251994329576923690768489f * 90.0f);

    cbProperties gUniformPPRProData;
    gUniformPPRProData.renderMode = gRenderMode;
    gUniformPPRProData.useHolePatching =
        ((gReflectionType == PP_REFLECTION) && gUseHolePatching == true) ? 1.0f : 0.0f;
    gUniformPPRProData.useExpensiveHolePatching = gUseExpensiveHolePatching == true ? 1.0f : 0.0f;
    gUniformPPRProData.useNormalMap             = gUseNormalMap == true ? 1.0f : 0.0f;
    gUniformPPRProData.useFadeEffect            = gUseFadeEffect == true ? 1.0f : 0.0f;
    gUniformPPRProData.intensity                = (gReflectionType == PP_REFLECTION) ? gRRP_Intensity : 1.0f;

    { // ssr projection
        {
            MapHelper<UniformExtendedCamData> ConstData(m_pImmediateContext, m_Buffers[gStrPPRProjectionCameraCompCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformExtendedCamData;
        }
        {
            MapHelper<UniformPlaneInfoData> ConstData(m_pImmediateContext, m_Buffers[gStrPPRProjectionPlaneCompCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformPlaneInfoData;
        }

        DispatchComputeAttribs DispatAttribsProjection;
        DispatAttribsProjection.ThreadGroupCountX = (Uint32)ceilf(mWidth * mHeight / 128.0f);
        DispatAttribsProjection.ThreadGroupCountY = (Uint32)1;
        DispatAttribsProjection.ThreadGroupCountZ = (Uint32)1;

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrPPRProjection]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrPPRProjection], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
        m_pImmediateContext->DispatchCompute(DispatAttribsProjection);
    }

    { // PPR_Reflection
        {
            MapHelper<UniformExtendedCamData> ConstData(m_pImmediateContext, m_Buffers[gStrPPRProjectionCameraCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformExtendedCamData;
        }
        {
            MapHelper<UniformPlaneInfoData> ConstData(m_pImmediateContext, m_Buffers[gStrPPRProjectionPlaneCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformPlaneInfoData;
        }
        {
            MapHelper<cbProperties> ConstData(m_pImmediateContext, m_Buffers[gStrPPRProjectionPropertiesCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformPPRProData;
        }

        ITextureView*           pQuadRTVs[1] = {m_TextureViews[gStrPPRReflectionRTV]};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrPPRReflection]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrPPRReflection], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }

    { // PPR_Holepatching
        {
            MapHelper<UniformExtendedCamData> ConstData(m_pImmediateContext, m_Buffers[gStrPPRHolepatchingCameraCompCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformExtendedCamData;
        }
        {
            MapHelper<cbProperties> ConstData(m_pImmediateContext, m_Buffers[gStrPPRHolepatchingPropertiesCBV], MAP_WRITE, MAP_FLAG_DISCARD);
            *ConstData = gUniformPPRProData;
        }

        ITextureView*           pQuadRTVs[1] = {m_pSwapChain->GetCurrentBackBufferRTV()};
        SetRenderTargetsAttribs QuadRTAttrs;
        QuadRTAttrs.NumRenderTargets    = 1;
        QuadRTAttrs.ppRenderTargets     = pQuadRTVs;
        QuadRTAttrs.pDepthStencil       = nullptr;
        QuadRTAttrs.StateTransitionMode = RESOURCE_STATE_TRANSITION_MODE_TRANSITION;

        m_pImmediateContext->SetRenderTargetsExt(QuadRTAttrs);

        constexpr float QuadClearColor[] = {0.0f, 0.0f, 0.0f, 0.f};
        m_pImmediateContext->ClearRenderTarget(pQuadRTVs[0], QuadClearColor, RESOURCE_STATE_TRANSITION_MODE_VERIFY);

        const Uint64 offset   = 0;
        IBuffer*     pBuffs[] = {m_Buffers[gStrRenderSceneBuffer]};
        m_pImmediateContext->SetVertexBuffers(0, 1, pBuffs, &offset, RESOURCE_STATE_TRANSITION_MODE_TRANSITION, SET_VERTEX_BUFFERS_FLAG_RESET);

        m_pImmediateContext->SetPipelineState(m_pPSOs[gStrPPRHolepatching]);
        m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrPPRHolepatching], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);

        DrawAttribs DrawAttrsQuad;
        DrawAttrsQuad.NumVertices = 3;
        DrawAttrsQuad.Flags       = DRAW_FLAG_VERIFY_ALL;
        m_pImmediateContext->Draw(DrawAttrsQuad);
    }
}

void Tutorial27_SSR::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    m_Camera.Update(m_InputController, static_cast<float>(ElapsedTime));
    {
        const auto& mouseState = m_InputController.GetMouseState();
        m_LastMouseState = mouseState;
    }

#if 0
    float4x4 view = float4x4(-4.37113883e-08f, -0.00000000f, -1.00000000f, 0.00000000f,
                             0.00000000f, 1.00000000f, -0.00000000f, 0.00000000f,
                             1.00000000f, -0.00000000f, -4.37113883e-08f, 0.00000000f,
                             -0.899999082f, 2.00000000f, 20.0000000f, 1.00000000f);
    float4x4 proj = float4x4(1.00000000f, 0.00000000f, 0.00000000f, 0.00000000f,
                             0.00000000f, 1.77777779f, 0.00000000f, 0.00000000f,
                             0.00000000f, 0.00000000f, 1.00009990f, 1.00000000f,
                             0.00000000f, 0.00000000f, -0.100009993f, 0.00000000f);
#else
    float4x4 view = m_Camera.GetViewMatrix();
    float4x4 proj = m_Camera.GetProjMatrix();
#endif

    mPassCbData.PreViewProj = mPassCbData.ViewProj;
    mPassCbData.View        = view;
    mPassCbData.Proj        = proj;
    mPassCbData.ViewProj    = view * proj;
    mPassCbData.InvView     = mPassCbData.View.Inverse();
    mPassCbData.InvProj     = mPassCbData.Proj.Inverse();
    mPassCbData.InvViewProj = mPassCbData.ViewProj.Inverse();
    ;

    mPassCbData.EyePosW          = gCamPos;
    mPassCbData.RenderTargetSize = float4((float)mWidth, (float)mHeight, 1.0f / mWidth, 1.0f / mHeight);
    mPassCbData.NearZ            = gCamearNear;
    mPassCbData.FarZ             = gCamearFar;
    mPassCbData.TotalTime        = 0;
    mPassCbData.DeltaTime        = 0;
    mPassCbData.ViewPortSize     = {(float)mWidth, (float)mWidth, 0.0f, 0.0f};

    mPassCbData.AmbientLight = {0.25f, 0.25f, 0.35f, 1.0f};

    { // Point Light
        PointLight light = {};
        light.mCol       = float4(1.0f, 0.5f, 0.1f, 0.0f);
        light.mPos       = float4(-12.5f, -3.5f, 4.7f, 0.0f);
        light.mRadius    = 10.0f;
        light.mIntensity = 400.0f;

        mPassCbData.PLights[0] = light;

        light.mCol       = float4(1.0f, 0.5f, 0.1f, 0.0f);
        light.mPos       = float4(-12.5f, -3.5f, -3.7f, 0.0f);
        light.mRadius    = 10.0f;
        light.mIntensity = 400.0f;

        mPassCbData.PLights[1] = light;

        // Add light to scene
        light.mCol       = float4(1.0f, 0.5f, 0.1f, 0.0f);
        light.mPos       = float4(9.5f, -3.5f, 4.7f, 0.0f);
        light.mRadius    = 10.0f;
        light.mIntensity = 400.0f;

        mPassCbData.PLights[2] = light;

        light.mCol       = float4(1.0f, 0.5f, 0.1f, 0.0f);
        light.mPos       = float4(9.5f, -3.5f, -3.7f, 0.0f);
        light.mRadius    = 10.0f;
        light.mIntensity = 400.0f;

        mPassCbData.PLights[3] = light;

        mPassCbData.amountOfPLights = 4;
    }
    {
        DirectionalLight light = {};
        light.mCol             = float4(1.0f, 1.0f, 1.0f, 5.0f);
        light.mPos             = float4(0.0f, 0.0f, 0.0f, 0.0f);
        light.mDir             = float4(-1.0f, -1.5f, 1.0f, 0.0f);

        mPassCbData.DLights[0] = light;

        mPassCbData.amountOfDLights = 1;
    }

    // cs uniform
    mCSUniformCbData.g_view           = view;
    mCSUniformCbData.g_proj           = proj;
    mCSUniformCbData.g_inv_view       = mCSUniformCbData.g_view.Inverse();
    mCSUniformCbData.g_inv_proj       = mCSUniformCbData.g_proj.Inverse();
    mCSUniformCbData.g_prev_view_proj = mPassCbData.PreViewProj;
    mCSUniformCbData.g_inv_view_proj  = mPassCbData.InvViewProj;

    mCSUniformCbData.g_frame_index++;
    mCSUniformCbData.g_max_traversal_intersections              = gSSSR_MaxTravelsalIntersections;
    mCSUniformCbData.g_min_traversal_occupancy                  = gSSSR_MinTravelsalOccupancy;
    mCSUniformCbData.g_most_detailed_mip                        = gSSSR_MostDetailedMip;
    mCSUniformCbData.g_temporal_stability_factor                = pSSSR_TemporalStability;
    mCSUniformCbData.g_depth_buffer_thickness                   = gSSSR_DepthThickness;
    mCSUniformCbData.g_samples_per_quad                         = gSSSR_SamplesPerQuad;
    mCSUniformCbData.g_temporal_variance_guided_tracing_enabled = gSSSR_TemporalVarianceEnabled;
    mCSUniformCbData.g_roughness_threshold                      = gSSSR_RougnessThreshold;
    mCSUniformCbData.g_skip_denoiser                            = gSSSR_SkipDenoiser;

    // Holepatching uniform
    mHolepatchingUniformData.renderMode               = SCENE_ONLY; //SCENE_WITH_REFLECTIONS
    mHolepatchingUniformData.useHolePatching          = 0.0f;
    mHolepatchingUniformData.useExpensiveHolePatching = 0.0f;
    mHolepatchingUniformData.useNormalMap             = 0.0f;
    mHolepatchingUniformData.intensity                = 1.0f;
    mHolepatchingUniformData.useFadeEffect            = 0.0f;
}

void Tutorial27_SSR::WindowResize(Uint32 Width, Uint32 Height)
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
