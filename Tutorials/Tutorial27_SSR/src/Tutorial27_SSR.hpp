
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{

struct ObjectConstants
{
    float4x4 World{};
    float4x4 PreWorld{};
    Uint32   MaterialIndex;
    float    roughness;
    float    metalness;
    int      pbrMaterials;
};

struct PointLight
{
    float4 mPos;
    float4 mCol;
    float  mRadius;
    float  mIntensity;
    char   _pad[8];
};

struct DirectionalLight
{
    float4 mPos;
    float4 mCol; //alpha is the intesity
    float4 mDir;
};

const static int MaxLights = 16;

struct PassConstants
{
    float4x4 View{};
    float4x4 InvView{};
    float4x4 Proj{};
    float4x4 InvProj{};
    float4x4 ViewProj{};
    float4x4 PreViewProj{};
    float4x4 InvViewProj{};
    float4   EyePosW;          // xyz
    float4   RenderTargetSize; // xy : size, zw : InvRenderTargetSize
    float    NearZ     = 0.0f;
    float    FarZ      = 0.0f;
    float    TotalTime = 0.0f;
    float    DeltaTime = 0.0f;
    float4   ViewPortSize;
    float4   AmbientLight = {0.0f, 0.0f, 0.0f, 1.0f};

    DirectionalLight DLights[MaxLights];
    PointLight       PLights[MaxLights];
    int              amountOfDLights;
    int              amountOfPLights;
};

struct UniformExtendedCamData
{
    float4x4 mViewMat;
    float4x4 mProjMat;
    float4x4 mViewProjMat;
    float4x4 mInvViewProjMat;

    float4 mCameraWorldPos;
    float4 mViewPortSize;
};

struct PlaneInfo
{
    float4x4 rotMat;
    float4   centerPoint;
    float4   size;
};

#define MAX_PLANES 4
struct UniformPlaneInfoData
{
    PlaneInfo planeInfo[MAX_PLANES];
    uint32_t  numPlanes;
    uint32_t  pad00;
    uint32_t  pad01;
    uint32_t  pad02;
};

struct cbProperties
{
    uint  renderMode;
    float useHolePatching;
    float useExpensiveHolePatching;
    float useNormalMap;

    float intensity;
    float useFadeEffect;
    float padding01;
    float padding02;
};

struct HolepatchingConstants
{
    uint  renderMode;
    float useHolePatching;
    float useExpensiveHolePatching;
    float useNormalMap; // no using

    float intensity;
    float useFadeEffect; // no using
    float padding01;
    float padding02;
};

struct UniformSSSRConstantsData
{
    float4x4 g_inv_view_proj;
    float4x4 g_proj;
    float4x4 g_inv_proj;
    float4x4 g_view;
    float4x4 g_inv_view;
    float4x4 g_prev_view_proj;

    uint  g_frame_index                              = 0;
    uint  g_max_traversal_intersections              = 0;
    uint  g_min_traversal_occupancy                  = 0;
    uint  g_most_detailed_mip                        = 0;
    float g_temporal_stability_factor                = 0;
    float g_depth_buffer_thickness                   = 0;
    uint  g_samples_per_quad                         = 0;
    uint  g_temporal_variance_guided_tracing_enabled = 0;
    float g_roughness_threshold                      = 0;
    uint  g_skip_denoiser                            = 0;
};

struct MaterialData
{
    Uint32 DiffuseMapIndex = 0;
};

enum
{
    SCENE_ONLY              = 0,
    REFLECTIONS_ONLY        = 1,
    SCENE_WITH_REFLECTIONS  = 2,
    SCENE_EXCLU_REFLECTIONS = 3,
};

enum
{
    PP_REFLECTION  = 0,
    SSS_REFLECTION = 1,
};

class Tutorial27_SSR final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial27 : Forward"; }

private:
    void CreatePipelineState();
    void CreateBuffers();
    void LoadTexture();
    void LoadModel();
    void PreDraw();

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;
    
    uint32_t mWidth = 1890;
    uint32_t mHeight = 1080;

    GLTF::Model* m_Model1;
    GLTF::Model* m_Model2;

    std::vector<ObjectConstants> mObjectCbData;
    PassConstants                mPassCbData;
    UniformSSSRConstantsData     mCSUniformCbData;
    HolepatchingConstants        mHolepatchingUniformData;
};

} // namespace Diligent
