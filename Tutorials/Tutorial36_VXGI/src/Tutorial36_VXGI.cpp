
#include "Tutorial36_VXGI.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"

#include <cassert>

namespace Diligent
{
static Diligent::float3 gCamPos{20.0f, -2.0f, 0.9f};

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;


static const char* gStrVoxelization      = "Voxelization";
static const char* gStrVoxelizationTex3D = "VoxelizationTex3D";

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
    
}

void Tutorial36_VXGI::CreateTexture3D()
{
    auto fill_pixel = [](float* data, int x, int y, int z, int w, int h, int d) -> void {
        int floats = 4; // r+g+b+a
        int i      = floats * (x + w * (y + h * z));

        assert(i < (w * h * d * floats));

        data[i + 0] = 1.0;
        data[i + 1] = 1.0;
        data[i + 2] = 1.0;
        data[i + 3] = 1.0;
    };

	auto fill_corners = [&](float* data, int w, int h, int d) -> void {
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

    for (int i = 0; i < TOTAL_VOXELGRID_RESOLUTIONS; i++)
    {
        int dimension = VOXELGRID_RESOLUTIONS[i];
        Uint32 miplevel  = (Uint32)log2(dimension);

        std::vector<TextureSubResData>  m_SubResources(miplevel);
        std::vector<std::vector<Uint8>> m_Mips(miplevel);

		size_t datasize = dimension * dimension * dimension * 4;
        float* data     = new float[datasize]; // 4 = r,g,b,a (@Cleanup)
        fill_corners(data, dimension, dimension, dimension);
        m_Mips[0].resize(datasize);
        memcpy(m_Mips[0].data(), data, datasize);

        m_SubResources[0].pData  = data;
        m_SubResources[0].Stride = datasize;

		TextureDesc Tex3DDesc;
        Tex3DDesc.Name      = gStrVoxelizationTex3D;
        Tex3DDesc.Type      = RESOURCE_DIM_TEX_3D;
        Tex3DDesc.Width     = dimension;
        Tex3DDesc.Height    = dimension;
        Tex3DDesc.Depth     = dimension;
        Tex3DDesc.MipLevels = miplevel;
        Tex3DDesc.Format    = TEX_FORMAT_RGBA32_FLOAT;
        Tex3DDesc.Usage     = USAGE_DEFAULT;
        Tex3DDesc.BindFlags = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;

		for (Uint32 m = 1; m < miplevel; ++m)
        {
            const MipLevelProperties MipLevelProps = GetMipLevelProperties(Tex3DDesc, m);

            Uint64 MipSize = MipLevelProps.MipSize;
            Uint64 RowSize = MipLevelProps.RowSize;
            if ((RowSize % 4) != 0)
            {
                RowSize = AlignUp(RowSize, Uint64{4});
                MipSize = RowSize * MipLevelProps.LogicalHeight;
            }
            m_Mips[m].resize(StaticCast<size_t>(MipSize));
            m_SubResources[m].pData  = m_Mips[m].data();
            m_SubResources[m].Stride = RowSize;

            auto FinerMipProps = GetMipLevelProperties(Tex3DDesc, m - 1);
            if (true)
            {
                ComputeMipLevelAttribs Attribs;
                Attribs.Format          = Tex3DDesc.Format;
                Attribs.FineMipWidth    = FinerMipProps.LogicalWidth;
                Attribs.FineMipHeight   = FinerMipProps.LogicalHeight;
                Attribs.pFineMipData    = m_SubResources[m - 1].pData;
                Attribs.FineMipStride   = StaticCast<size_t>(m_SubResources[m - 1].Stride);
                Attribs.pCoarseMipData  = m_Mips[m].data();
                Attribs.CoarseMipStride = StaticCast<size_t>(m_SubResources[m].Stride);
                Attribs.AlphaCutoff     = false;
                static_assert(MIP_FILTER_TYPE_DEFAULT == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_DEFAULT), "Inconsistent enum values");
                static_assert(MIP_FILTER_TYPE_BOX_AVERAGE == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_BOX_AVERAGE), "Inconsistent enum values");
                static_assert(MIP_FILTER_TYPE_MOST_FREQUENT == static_cast<MIP_FILTER_TYPE>(TEXTURE_LOAD_MIP_FILTER_MOST_FREQUENT), "Inconsistent enum values");
                Attribs.FilterType = static_cast<MIP_FILTER_TYPE>(MIP_FILTER_TYPE_DEFAULT);
                ComputeMipLevel(Attribs);
            }
        }
        
        TextureData InitData{m_SubResources.data(), (Uint32)miplevel};

        m_pDevice->CreateTexture(Tex3DDesc, &InitData, &m_Textures[Tex3DDesc.Name]);

        delete[] data; // uploaded to gpu, no need to keep in app memory (@Cleanup)
    }
}

void Tutorial36_VXGI::CreateIndexBuffer()
{
    
}

void Tutorial36_VXGI::LoadTexture()
{
    
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
    CreateIndexBuffer();
    LoadTexture();
}

// Render a frame
void Tutorial36_VXGI::Render()
{
    
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
