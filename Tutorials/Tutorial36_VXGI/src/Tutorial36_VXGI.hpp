
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{
const Uint32 TOTAL_VOXELGRID_RESOLUTIONS                        = 4;
const Uint32 VOXELGRID_RESOLUTIONS[TOTAL_VOXELGRID_RESOLUTIONS] = {64, 128, 128, 128};
const Uint32 DEFAULT_VOXELGRID_RESOLUTION_INDEX                 = 2;

struct Voxelization
{
    //Camera camera; // orthographic projection

    RefCntAutoPtr<ITexture> resolutions[TOTAL_VOXELGRID_RESOLUTIONS];                // 64, 128, 256, 512
    int                     current_resolution = DEFAULT_VOXELGRID_RESOLUTION_INDEX; // index to array above ^
};

struct Voxelization_Settings // for shaders
{
    int  visualize_mipmap_level = 0;
};

struct Cone_Settings
{
    float aperture;        // aperture = tan(degrees * 0.5)
    float sampling_factor; // 0.0...1.0
    float distance_offset; // voxel_size * distance_in_voxels
    float max_distance;    // in NDC space -1.0...1.0
    float result_intensity;
    int  is_enabled;
    float _pad0;
    float _pad1;
};

struct WorldPosCameraCB
{
    float4x4 viewPorj;
    float4   cameraPos;
};

struct VisualizerCameraCB
{
    float3 u_camera_world_position;
};

struct ObjectConstants
{
    float4x4 World{};
    float4x4 PreWorld{};
    Uint32   MaterialIndex;
    float    roughness;
    float    metalness;
    int      pbrMaterials;
};

struct DepthPassObject
{
    float4x4 world;
};

struct DirectionalLight
{
    float4 mPos;
    float4 mCol; //alpha is the intesity
    float4 mDir;
};

struct PointLight
{
    float4 mPos;
    float4 mCol;
    float  mRadius;
    float  mIntensity;
    char   _pad[8];
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
    float4x4 LightView{};
    float4x4 LightProj{};
    float4x4 LightViewProj{};
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


struct Cone_Tracing_Shader_Settings
{
    Cone_Settings diffuse_settings      = {0.577f, 0.119f, 0.081f, 2.0f, 1.0f, true};
    Cone_Settings specular_settings     = {0.027f, 0.146f, 0.190f, 2.0f, 1.0f, true};
    Cone_Settings soft_shadows_settings = {0.017f, 0.200f, 0.120f, 2.0f, 1.0f, true};
    Cone_Settings ao_settings           = {0.577f, 1.000f, 0.500f, 1.0f, 1.0f, true};

    float3 scene_voxel_scale;
    int    trace_ao_separately   = false; // otherwise use diffuse cone opacity
    float3 u_ambient_light       = float3(0.2f, 0.2f, 0.2f);
    float gamma                  = 2.2f;
    float hard_shadow_bias       = 0.005f;
    float  direct_light_intensity = 1.0f;
    int    voxel_grid_resolution;
    int    max_mipmap_level;
    int  enable_direct_light    = 1;
    int  enable_hard_shadows    = 0; // shadow mapped
};


class Tutorial36_VXGI final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial03: Texturing"; }

private:

    void                    voxel_grid_resolution_changed(int new_resolution_index);
    int                     get_current_voxelgrid_resolution();
    RefCntAutoPtr<ITexture> get_current_voxelgrid();

    void CreatePipelineState();
    void CreateTexture3D();
    void CreateBuffer();
    void LoadModel();
    void LoadTexture();
    void PrepareVoxelTex();

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t mWidth  = 1280;
    uint32_t mHeight = 720;

    GLTF::Model* m_Model;

    Voxelization mVoxelization;

	bool visualize_gbuffers  = false;
    bool is_first_frame      = true;
    bool voxelize_next_frame = true;
    bool render_light_bulbs  = false;

    Voxelization_Settings        mVoxelSetting;
    VisualizerCameraCB           mVisualizerCameraCB;
    DepthPassObject              mDepthPassObject;
    PassConstants                mPassCbData;
    WorldPosCameraCB             mWorldPosCameraCB;
    Cone_Tracing_Shader_Settings mConeTracingSettings;

    std::vector<ObjectConstants> mObjectCbData;

    std::vector<float> mInitVoxelData[TOTAL_VOXELGRID_RESOLUTIONS];
};

} // namespace Diligent
