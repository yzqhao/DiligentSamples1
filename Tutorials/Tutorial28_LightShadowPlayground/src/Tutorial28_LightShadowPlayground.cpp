
#include "Tutorial28_LightShadowPlayground.hpp"
#include "MapHelper.hpp"
#include "GraphicsUtilities.h"
#include "TextureUtilities.h"
#include "ColorConversion.h"
#include "SDFConstant.h"

namespace Diligent
{

#define ESM_MSAA_SAMPLES  1
#define ESM_SHADOWMAP_RES 2048u
#define NUM_SDF_MESHES    3

#define Epilson (1.e-4f)

#define SAN_MIGUEL_ORIGINAL_SCALE   50.0f
#define SAN_MIGUEL_ORIGINAL_OFFSETX -20.f


#define SAN_MIGUEL_OFFSETX 150.f
#define MESH_COUNT         1
#define MESH_SCALE         10.f

#define ASM_SUN_SPEED 0.001f

SDFMesh SDFMeshes[NUM_SDF_MESHES] = {};
size_t  gSDFProgressValue         = 0;

static const float gCamearNear = 0.1f;
static const float gCamearFar  = 1000.0f;

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
    float         mSunSpeedY  = 0.025f;
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



// clang-format off
// kinda: 0.09f // thin: 0.15f // superthin: 0.5f // triplethin: 0.8f
MeshInfo opaqueMeshInfos[] = {
    {"twosided_superthin_innerfloor_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"twosided_triplethin_hugewall_02", NULL, MATERIAL_FLAG_TWO_SIDED, 0.8f},
    {"twosided_triplethin_hugewall_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.8f},
    {"balcony_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outerfloor_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_06", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_07", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_11", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_10", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_09", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"indoortable_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outdoortable_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outdoortable_07", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_15", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_14", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_13", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_12", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outdoortable_08", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_10", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_06", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_09", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"outertable_11", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"uppertable_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"uppertable_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"uppertable_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"uppertable_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"opendoor_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"opendoor_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"indoortable_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"indoortable_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"indoortable_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"indoortable_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"waterpool_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_06", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_10", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_09", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_08", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_07", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_12", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_thin_leavesbasket_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.15f},
    {"twosided_kinda_double_combinedoutdoorchairs_01", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"gargoyle_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"gargoyle_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"gargoyle_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"gargoyle_06", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_kinda_double_outdoorchairs_02", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_indoorchairs_02", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_thin_double_indoorchairs_03", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.15f},
    {"twosided_thin_double_indoorchairs_04", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.15f},
    {"twosided_kinda_double_indoorchairs_05", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_indoorchairs_06", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"double_upperchairs_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"longpillar_11", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_thin_leavesbasket_02", NULL, MATERIAL_FLAG_TWO_SIDED, 0.15f},
    {"twosided_kinda_double_outdoorchairs_01", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_04", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"double_upperchairs_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"double_upperchairs_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"double_upperchairs_04", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"doorwall_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"doorwall_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"underledge_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"underceil_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"underceil_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_kinda_double_indoorchairs_07", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"hugewallfront_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_kinda_metalLedges_02", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_07", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_05", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_03", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_04", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_06", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_double_outdoorchairs_06", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_05", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_metalLedges_08", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_metalLedges_09", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_doorwall_03", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"twosided_kinda_doorwall_04", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"myinnerfloor_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_kinda_double_outdoorchairs_07", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_08", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"mywall_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_kinda_double_outdoorchairs_09", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_10", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_11", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_12", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_13", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_14", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_double_outdoorchairs_15", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.09f},
    {"twosided_kinda_metalLedges_10", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"curvepillar_12", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"curvepillar_08", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_superthin_innerhall_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"doorwall_06", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"doorwall_05", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"double_doorwall_07", NULL, MATERIAL_FLAG_DOUBLE_VOXEL_SIZE, 0.0f},
    {"twosided_kinda_basketonly_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.09f},
    {"backmuros_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"shortceil_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"shortceil_02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"shortceil_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"cap03", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"cap01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"cap02", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"cap04", NULL, MATERIAL_FLAG_NONE, 0.0f}
};

MeshInfo flagsMeshInfos[] = {
    {"gargoyle_08", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"twosided_superthin_forgeflags_01", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"twosided_superthin_forgeflags_02", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"twosided_superthin_forgeflags_04", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"twosided_superthin_forgeflags_03", NULL, MATERIAL_FLAG_TWO_SIDED, 0.5f},
    {"gargoyle_01", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"gargoyle_07", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"gargoyle_03", NULL, MATERIAL_FLAG_NONE, 0.0f},
};

MeshInfo alphaTestedMeshInfos[] = {
    // group 0
    {"twosided_thin_alphatested_smallLeaves04_beginstack0", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"flower0339_continuestack0", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"stem01_continuestack0", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"basket01_continuestack0", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 1
    {"twosided_thin_alphatested_smallLeaves04_beginstack1", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"stem_continuestack1", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"smallLeaves019_alphatested_continuestack1", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"smallLeaves0377_alphatested_continuestack1", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack1", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"rose00_alphatested_continuestack1", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    // group 2
    {"twosided_thin_alphatested_smallLeaves00_beginstack2", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves023_continuestack2", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"stem02_continuestack2", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"alphatested_flower0304_continuestack2", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"stem03_continuestack2", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"basket02_continuestack2", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 3
    {"twosided_thin_alphatested_smallLeaves04_beginstack3", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"twosided_thin_alphatested_smallLeaves09_continuestack3", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket_continuestack3", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 4
    {"twosided_thin_alphatested_smallLeaves022_beginstack4", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket_continuestack4", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 5
    {"twosided_thin_alphatested_smallLeaves013_beginstack5", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves012_continuestack5", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves020_continuestack5", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_twosided_thin_smallLeaves07_continuestack5", NULL, MATERIAL_FLAG_ALPHA_TESTED | MATERIAL_FLAG_TWO_SIDED, 0.15f},
    {"alphatested_twosided_thin_smallLeaves05_continuestack5", NULL, MATERIAL_FLAG_ALPHA_TESTED | MATERIAL_FLAG_TWO_SIDED, 0.15f},
    {"twosided_thin_alphatested_floor1_continuestack5", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack5", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 6
    {"twosided_thin_alphatested_smallLeaves023_beginstack6", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves0300_continuestack6", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves016_continuestack6", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0341_continuestack6", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack6", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 7
    {"twosided_thin_alphatested_smallLeaves014_beginstack7", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves015_continuestack7", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack7", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 8
    {"twosided_thin_alphatested_smallLeaves027_beginstack8", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0343_continuestack8", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves018_continuestack8", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack8", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 9
    {"twosided_thin_alphatested_smallLeaves0380_beginstack9", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0338_continuestack9", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"bakset01_continuestack9", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 10
    {"twosided_thin_alphatested_smallLeaves00_beginstack10", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0304_continuestack10", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack10", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 11
    {"twosided_thin_double_alphatested__group146_beginstack11", NULL, MATERIAL_FLAG_ALL, 0.15f},
    {"alphatested_group147_continuestack11", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"branch_group145_continuestack11", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 12
    {"twosided_superthin_alphatested_double_treeLeaves04_beginstack12", NULL, MATERIAL_FLAG_ALL, 0.5f},
    {"alphatested_treeLeaves00_continuestack12", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_treeLeaves02_continuestack12", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_treeLeaves05_continuestack12", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    // group 13
    {"twosided_superthin_alphatested_double_treeLeaves08_beginstack13", NULL, MATERIAL_FLAG_ALL, 0.5f},
    {"alphatested_treeLeaves05_continuestack13", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_treeLeaves07_continuestack13", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    // group 14
    {"twosided_superthin_alphatested_double_treeLeaves03_beginstack14", NULL, MATERIAL_FLAG_ALL, 0.5f},
    {"alphatested_treeLeaves01_continuestack14", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_treeLeaves06_continuestack14", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    // group 15
    {"twosided_thin_alphatested_smallLeaves02_beginstack15", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves08_continuestack15", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack15", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 16
    {"twosided_thin_double_alphatested_smallLeaves019_beginstack16", NULL, MATERIAL_FLAG_ALL, 0.15f},
    {"alphatested_flower0343_continuestack16", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves018_continuestack16", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves0377_continuestack16", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_rose00_continuestack16", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack16", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 17
    {"twosided_thin_alphatested_smallLeaves0380_beginstack17", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0342_continuestack17", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves07_continuestack17", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves05_continuestack17", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_floor1_continuestack17", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0338_continuestack17", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack17", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 18
    {"twosided_thin_alphatested_smallLeaves06_beginstack18", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves0378_continuestack18", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0344_continuestack18", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0340_continuestack18", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack18", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 19
    {"twosided_thin_double_alphatested_smallLeaves00_beginstack19", NULL, MATERIAL_FLAG_ALL, 0.15f},
    {"alphatested_smallLeaves016_continuestack19", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0341_continuestack19", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves017_continuestack19", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves021_continuestack19", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0304_continuestack19", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack19", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 20
    {"twosided_thin_alphatested_smallLeaves024_beginstack20", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack20", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 21
    {"twosided_thin_alphatested_smallLeaves010_beginstack21", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack21", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 22
    {"twosided_thin_alphatested_smallLeaves01_beginstack22", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack22", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 23
    {"twosided_thin_alphatested_smallLeaves04_beginstack23", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack23", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 24
    {"twosided_thin_alphatested_smallLeaves024_beginstack24", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack24", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 25
    {"twosided_thin_alphatested_smallLeaves00_beginstack25", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0304_continuestack25", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack25", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 26
    {"twosided_thin_alphatested_smallLeaves00_beginstack26", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves023_continuestack26", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0304_continuestack26", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack26", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 27
    {"twosided_thin_double_alphatested_smallLeaves025_beginstack27", NULL, MATERIAL_FLAG_ALL, 0.15f},
    {"alphatested_smallLeaves011_continuestack27", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves021_continuestack27", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves017_continuestack27", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves08_continuestack27", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack27", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 28
    {"twosided_thin_alphatested_smallLeaves0377_beginstack28", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_rose00_continuestack28", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack28", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 29
    {"twosided_thin_double_alphatested_smallLeaves00_beginstack29", NULL, MATERIAL_FLAG_ALL, 0.15f},
    {"alphatested_smallLeaves026_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower01_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves09_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves013_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_smallLeaves023_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"alphatested_flower0304_continuestack29", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack29", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 30
    {"twosided_thin_alphatested_smallLeaves027_beginstack30", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"flower00_continuestack30", NULL, MATERIAL_FLAG_NONE, 0.0f},
    {"basket01_continuestack30", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 31
    {"twosided_thin_alphatested_smallLeaves013_beginstack31", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack31", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 32
    {"twosided_thin_alphatested_smallLeaves04_beginstack32", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack32", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 33
    {"twosided_thin_alphatested_smallLeaves00_beginstack33", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_flower0304_continuestack33", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack33", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 34
    {"twosided_thin_alphatested_smallLeaves0381_beginstack34", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack34", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 35
    {"twosided_thin_alphatested_smallLeaves0379_beginstack35", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"basket01_continuestack35", NULL, MATERIAL_FLAG_NONE, 0.0f},
    // group 36
    {"twosided_thin_alphatested_smallLeaves021_beginstack36", NULL, MATERIAL_FLAG_TWO_SIDED | MATERIAL_FLAG_ALPHA_TESTED, 0.15f},
    {"alphatested_smallLeaves017_continuestack36", NULL, MATERIAL_FLAG_ALPHA_TESTED, 0.0f},
    {"basket01_continuestack36", NULL, MATERIAL_FLAG_NONE, 0.0f}
};
// clang-format on

uint32_t alphaTestedGroupSizes[] = {
    4, 6, 6, 3, 2, 7, 5, 3, 4, 3, 3, 3, 4, 3, 3, 3, 6, 7,
    5, 7, 2, 2, 2, 2, 2, 3, 4, 6, 3, 8, 3, 2, 2, 3, 2, 2, 3};

uint32_t alphaTestedMeshIndices[] = {
    30, 12, 31, 32,                    // group 0
    34, 35, 36, 37, 38, 39,            // group 1
    40, 41, 42, 43, 44, 45,            // group 2
    47, 50, 51,                        // group 3
    9, 52,                             // group 4
    53, 6, 25, 55, 57, 59, 60,         // group 5
    61, 14, 63, 65, 66,                // group 6
    0, 7, 67,                          // group 7
    69, 71, 73, 75,                    // group 8
    77, 79, 80,                        // group 9
    81, 84, 86,                        // group 10
    2, 26, 28,                         // group 11
    33, 5, 16, 85,                     // group 12
    24, 20, 27,                        // group 13
    13, 17, 19,                        // group 14
    83, 82, 87,                        // group 15
    46, 70, 72, 88, 89, 90,            // group 16
    76, 1, 54, 56, 58, 78, 91,         // group 17
    11, 3, 18, 29, 92,                 // group 18
    95, 62, 64, 93, 94, 96, 97,        // group 19
    99, 100,                           // group 20
    23, 101,                           // group 21
    10, 102,                           // group 22
    104, 105,                          // group 23
    98, 106,                           // group 24
    107, 108, 109,                     // group 25
    111, 110, 112, 113,                // group 26
    15, 21, 117, 118, 119, 120,        // group 27
    114, 115, 116,                     // group 28
    125, 4, 8, 49, 122, 123, 124, 126, // group 29
    68, 74, 127,                       // group 30
    121, 48,                           // group 31
    103, 129,                          // group 32
    128, 130, 131,                     // group 33
    22, 132,                           // group 34
    133, 134,                          // group 35
    135, 136, 137                      // group 36
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
    return new Tutorial28_LightShadowPlayground();
}

void Tutorial28_LightShadowPlayground::CreatePipelineState()
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
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "triangleFiltering CS";
            ShaderCI.FilePath        = "triangleFiltering.hlsl";
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
            ShaderCI.EntryPoint      = "main";
            ShaderCI.Desc.Name       = "batchCompaction CS";
            ShaderCI.FilePath        = "batchCompaction.hlsl";
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

void Tutorial28_LightShadowPlayground::LoadScene()
{
    mScene         = loadScene("./Models/SanMiguel.gltf", SAN_MIGUEL_ORIGINAL_SCALE, SAN_MIGUEL_ORIGINAL_OFFSETX, 0.0f, 0.0f, m_pDevice, m_pImmediateContext);
    mMeshCount     = (Uint32)mScene->mDrawArgCount;
    mMaterialCount = mMeshCount;
    pMeshes        = (ClusterContainer*)malloc(mMeshCount * sizeof(ClusterContainer));

    {
        gMeshInfoData.mColor             = float4(1.f);
        gMeshInfoData.mScale             = float3(MESH_SCALE);
        gMeshInfoData.mScaleMat          = float4x4::Scale(gMeshInfoData.mScale);
        float finalXTranslation          = SAN_MIGUEL_OFFSETX;
        gMeshInfoData.mTranslation       = float3(finalXTranslation, 0.f, 0.f);
        gMeshInfoData.mOffsetTranslation = float3(0.0f, 0.f, 0.f);
        gMeshInfoData.mTranslationMat    = float4x4::Translation(gMeshInfoData.mTranslation);
    }

    {
        // Cluster creation
        /************************************************************************/
        // Calculate clusters
        for (uint32_t i = 0; i < mMeshCount; ++i)
        {
            //MeshInstance*   subMesh = &pMeshes[i];
            ClusterContainer* subMesh      = &pMeshes[i];
            MaterialFlags     materialFlag = mScene->materialFlags[i];

            IndirectDrawIndexArguments indirectdraw;
            indirectdraw.mIndexCount    = mScene->m_Model->Meshes[i].Primitives[0].IndexCount;
            indirectdraw.mInstanceCount = 1;
            indirectdraw.mStartIndex    = mScene->m_Model->Meshes[i].Primitives[0].FirstIndex;
            indirectdraw.mVertexOffset  = 0;
            indirectdraw.mStartInstance = 0;

            createClusters(materialFlag & MATERIAL_FLAG_TWO_SIDED, mScene, &indirectdraw, subMesh);
        }
    }

    { // ring buffer
        uint bufferSizeTotal = 0;
        for (uint32_t j = 0; j < gSmallBatchChunkCount; ++j)
        {
            const uint32_t bufferSize = BATCH_COUNT * sizeof(FilterBatchData);
            bufferSizeTotal += bufferSize;
            pFilterBatchChunk[j]                       = (FilterBatchChunk*)malloc(sizeof(FilterBatchChunk));
            pFilterBatchChunk[j]->currentBatchCount    = 0;
            pFilterBatchChunk[j]->currentDrawCallCount = 0;
        }
        addUniformGPURingBuffer(bufferSizeTotal, &pBufferFilterBatchData);
    }
    mBatchDatas.resize(gSmallBatchChunkCount);
}

void Tutorial28_LightShadowPlayground::LoadBuffers()
{
    { // mBufferMeshConstants
        std::vector<MeshConstants> meshConstants(mMeshCount);
        for (uint32_t i = 0; i < mMeshCount; ++i)
        {
            meshConstants[i].faceCount   = (Uint32)mScene->m_Model->Meshes.size() / 3;
            meshConstants[i].indexOffset = 0;
            meshConstants[i].materialID  = i;
            meshConstants[i].twoSided    = (mScene->materialFlags[i] & MATERIAL_FLAG_TWO_SIDED) ? 1 : 0;
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
        std::vector<Uint32> materialAlphaData(mMaterialCount);
        for (uint32_t i = 0; i < mMaterialCount; ++i)
        {
            materialAlphaData[i] = (mScene->materialFlags[i] & MATERIAL_FLAG_ALPHA_TESTED) ? 1 : 0;
        }
        //StructuredBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name              = "materialProps buffer";
        BuffDesc.Usage             = USAGE_DEFAULT;
        BuffDesc.BindFlags         = BIND_SHADER_RESOURCE | BIND_UNORDERED_ACCESS;
        BuffDesc.Mode              = BUFFER_MODE_STRUCTURED;
        BuffDesc.ElementByteStride = sizeof(uint);
        BuffDesc.Size              = sizeof(uint) * mMaterialCount;

        BufferData VBData;
        VBData.pData    = materialAlphaData.data();
        VBData.DataSize = sizeof(Uint32) * static_cast<Uint32>(materialAlphaData.size());
        m_pDevice->CreateBuffer(BuffDesc, &VBData, &mBufferMaterialProps);
        VERIFY_EXPR(mBufferMaterialProps != nullptr);
    }
    
    {
        auto vertexcnt = mScene->m_Model->m_VertexData[0].size() / 4 / 11;
        std::vector<Uint8> temp(vertexcnt * 12);
        for (auto i = 0, j = 0; i < vertexcnt; ++i)
        {
            for (int ii = 0; ii < 12; ++ii)
                temp[i++] = mScene->m_Model->m_VertexData[0][j++];
            j += 32;
        }
        //RWByteAddressBuffer
        BufferDesc BuffDesc;
        BuffDesc.Name      = "vertex buffer";
        BuffDesc.Usage     = USAGE_DEFAULT;
        BuffDesc.BindFlags = BIND_SHADER_RESOURCE;
        BuffDesc.Mode      = BUFFER_MODE_RAW;
        BuffDesc.Size      = temp.size();

        BufferData VBData;
        VBData.pData    = temp.data();
        VBData.DataSize = temp.size();

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
        BuffDesc.Size      = mScene->m_Model->m_IndexData.size();

        BufferData VBData;
        VBData.pData    = mScene->m_Model->m_IndexData.data();
        VBData.DataSize = mScene->m_Model->m_IndexData.size();

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
            BuffDesc.Size      = mScene->m_Model->m_IndexData.size();
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

    {   // const buffer
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

    {   // arg
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
        BufferViewDesc ViewDesc;
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

void Tutorial28_LightShadowPlayground::LoadTexture()
{
}


void Tutorial28_LightShadowPlayground::Initialize(const SampleInitInfo& InitInfo)
{
    SampleBase::Initialize(InitInfo);

    CreatePipelineState();
    LoadScene();
    LoadBuffers();
    LoadTexture();
}

// Render a frame
void Tutorial28_LightShadowPlayground::Render()
{
    TriangleFilteringPass();
}

void Tutorial28_LightShadowPlayground::TriangleFilteringPass()
{
    // clear pass
    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrClearVisibilityBuffers]);
    DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = (Uint32)(MAX_DRAWS_INDIRECT / CLEAR_THREAD_COUNT) + 1;
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrClearVisibilityBuffers], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribs);

    uint32_t currentSmallBatchChunk          = 0;
    uint     accumDrawCount                  = 0;
    uint     accumNumTriangles               = 0;
    uint     accumNumTrianglesAtStartOfBatch = 0;
    uint     batchStart                      = 0;

    uint64_t            size   = BATCH_COUNT * sizeof(SmallBatchData) * gSmallBatchChunkCount;
    GPURingBufferOffset offset = getGPURingBufferOffset(pBufferFilterBatchData, (uint32_t)size, (uint32_t)size);

    FilterBatchData* batches  = (FilterBatchData*)&mBatchDatas[0];
    int              batchIdx = 0;

    for (uint32_t i = 0; i < mMeshCount; ++i)
    {
        ClusterContainer* drawBatch  = &pMeshes[i];
        FilterBatchChunk* batchChunk = pFilterBatchChunk[currentSmallBatchChunk];
        for (uint32_t j = 0; j < drawBatch->clusterCount; ++j)
        {
            const ClusterCompact* clusterCompactInfo = &drawBatch->clusterCompacts[j];
            {
                // cluster culling passed or is turned off
                // We will now add the cluster to the batch to be triangle filtered
                addClusterToBatchChunk(clusterCompactInfo, batchStart, accumDrawCount, accumNumTrianglesAtStartOfBatch, i, batchChunk, batches);
                accumNumTriangles += clusterCompactInfo->triangleCount;
            }

            // check to see if we filled the batch
            if (batchChunk->currentBatchCount >= BATCH_COUNT)
            {
                //uint32_t batchCount = batchChunk->currentBatchCount;
                ++accumDrawCount;

                // run the triangle filtering and switch to the next small batch chunk
                filterTriangles(batchChunk, batchIdx);
                batches                = (FilterBatchData*)&mBatchDatas[++batchIdx];
                currentSmallBatchChunk = (currentSmallBatchChunk + 1) % gSmallBatchChunkCount;
                batchChunk             = pFilterBatchChunk[currentSmallBatchChunk];

                batchStart                      = 0;
                accumNumTrianglesAtStartOfBatch = accumNumTriangles;
            }
        }

        // end of that mash, set it up so we can add the next mesh to this culling batch
        if (batchChunk->currentBatchCount > 0)
        {
            FilterBatchChunk* batchChunk2 = pFilterBatchChunk[currentSmallBatchChunk];
            ++accumDrawCount;

            batchStart                      = batchChunk2->currentBatchCount;
            accumNumTrianglesAtStartOfBatch = accumNumTriangles;
        }
    }

    filterTriangles(pFilterBatchChunk[currentSmallBatchChunk], batchIdx);

    // batch pass
    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrBatchCompaction]);
    DispatchComputeAttribs DispatAttribsBatch;
    DispatAttribsBatch.ThreadGroupCountX = (Uint32)(MAX_DRAWS_INDIRECT / CLEAR_THREAD_COUNT) + 1;
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrBatchCompaction], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribsBatch);
}

void Tutorial28_LightShadowPlayground::filterTriangles(FilterBatchChunk* batchChunk, uint64_t batchidx)
{
	// Check if there are batches to filter
	if (batchChunk->currentBatchCount == 0)
		return;

    {
        MapHelper<FillBatchData> ConstData(m_pImmediateContext, m_Buffers[gStrBatchDataConstants], MAP_WRITE, MAP_FLAG_DISCARD);
        *ConstData = mBatchDatas[batchidx];
    }
    {
        MapHelper<VisibilityBufferConstants> ConstData(m_pImmediateContext, m_Buffers[gStrVisibilityBufferConstants], MAP_WRITE, MAP_FLAG_DISCARD);
        *ConstData = mVisibilityBufferCB;
    }

    m_pImmediateContext->SetPipelineState(m_pPSOs[gStrTriangleFiltering]);
    DispatchComputeAttribs DispatAttribs;
    DispatAttribs.ThreadGroupCountX = batchChunk->currentBatchCount;
    m_pImmediateContext->CommitShaderResources(m_ShaderResourceBindings[gStrTriangleFiltering], RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    m_pImmediateContext->DispatchCompute(DispatAttribs);

	// Reset batch chunk to start adding triangles to it
	batchChunk->currentBatchCount = 0;
	batchChunk->currentDrawCallCount = 0;
}

void Tutorial28_LightShadowPlayground::Update(double CurrTime, double ElapsedTime)
{
    SampleBase::Update(CurrTime, ElapsedTime);

    // Update something
    {
        /************************************************************************/
        // Light Matrix Update
        /************************************************************************/
        float3 lightSourcePos(10.f, 000.0f, 10.f);
        lightSourcePos.x = (lightSourcePos.x + 20.0f + SAN_MIGUEL_OFFSETX);
        // directional light rotation & translation
        float4x4 rotation    = float4x4::RotationXY(gLightCpuSettings.mSunControl.x, gLightCpuSettings.mSunControl.y); //float4x4::RotationY(gLightCpuSettings.mSunControl.y) * float4x4::RotationX(gLightCpuSettings.mSunControl.x);
        float4x4 translation = float4x4::Translation(-float3(lightSourcePos));

        float4   temp         = -float4((rotation.Inverse() * float4(0, 0, 1, 0)));
        float3   newLightDir(temp.x, temp.y, temp.z);
        float4x4 lightProjMat = orthographic(-140, 140, -210, 90, -220, 100);
        float4x4 lightView    = translation * rotation;

        mLightUniformBlock.mLightPosition = float4(0.f);
        mLightUniformBlock.mLightViewProj = lightView * lightProjMat;
        mLightUniformBlock.mLightColor    = float4(1, 1, 1, 1);
        mLightUniformBlock.mLightUpVec    = float4(lightView[1][0], lightView[1][1], lightView[1][2], lightView[1][3]); // 21 22 23 24
        mLightUniformBlock.mLightDir      = newLightDir;


        const float lightSourceAngle                       = clamp(gLightCpuSettings.mSourceAngle, 0.001f, 4.0f) * PI_F / 180.0f;
        mLightUniformBlock.mTanLightAngleAndThresholdValue = float4(tan(lightSourceAngle),
                                                                    cos(PI_F / 2 + lightSourceAngle), SDF_LIGHT_THERESHOLD_VAL, 0.f);
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
        float         depthMul       = primaryProjMat.m22;
        float         depthAdd       = primaryProjMat.m32;

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

        mVisibilityBufferCB.mWorldViewProjMat[VIEW_CAMERA]              =  world * view * proj;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mWindowSizeX = (float)mWidth;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mWindowSizeY = (float)mHeight;
        mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA].mSampleCount = gAppSettings.mMsaaLevel;

        if (mCurrentShadowType == SHADOW_TYPE_ESM)
        {
            mVisibilityBufferCB.mWorldViewProjMat[VIEW_SHADOW] = world * mLightUniformBlock.mLightViewProj; // gMeshInfoUniformData[0][gFrameIndex].mWorldMat;
            mVisibilityBufferCB.mCullingViewports[VIEW_SHADOW].mSampleCount = 1;

            mVisibilityBufferCB.mCullingViewports[VIEW_SHADOW].mWindowSizeX = ESM_SHADOWMAP_RES;
            mVisibilityBufferCB.mCullingViewports[VIEW_SHADOW].mWindowSizeY = ESM_SHADOWMAP_RES;
        }
        else if (mCurrentShadowType == SHADOW_TYPE_MESH_BAKED_SDF)
        {
            mVisibilityBufferCB.mWorldViewProjMat[VIEW_SHADOW] = mVisibilityBufferCB.mWorldViewProjMat[VIEW_CAMERA];
            mVisibilityBufferCB.mCullingViewports[VIEW_SHADOW] = mVisibilityBufferCB.mCullingViewports[VIEW_CAMERA];
        }
    }
}

void Tutorial28_LightShadowPlayground::WindowResize(Uint32 Width, Uint32 Height)
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
