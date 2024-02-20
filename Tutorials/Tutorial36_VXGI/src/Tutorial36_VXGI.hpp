
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{
const int TOTAL_VOXELGRID_RESOLUTIONS                        = 4;
const int VOXELGRID_RESOLUTIONS[TOTAL_VOXELGRID_RESOLUTIONS] = {64, 128, 256, 512};
const int DEFAULT_VOXELGRID_RESOLUTION_INDEX                 = 2;

struct Voxelization
{
    //Camera camera; // orthographic projection

    RefCntAutoPtr<ITexture> resolutions[TOTAL_VOXELGRID_RESOLUTIONS];                // 64, 128, 256, 512
    int                     current_resolution = DEFAULT_VOXELGRID_RESOLUTION_INDEX; // index to array above ^
};

struct Voxelization_Settings // for shaders
{
    bool use_ambient_light      = true;
    int  visualize_mipmap_level = 0;
};

struct Cone_Settings
{
    float aperture;        // aperture = tan(degrees * 0.5)
    float sampling_factor; // 0.0...1.0
    float distance_offset; // voxel_size * distance_in_voxels
    float max_distance;    // in NDC space -1.0...1.0
    float result_intensity;
    bool  is_enabled;
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
    void CreateIndexBuffer();
    void LoadTexture();

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
};

} // namespace Diligent
