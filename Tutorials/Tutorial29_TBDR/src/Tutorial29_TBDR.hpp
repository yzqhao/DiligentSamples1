
#pragma once

#include "SampleBase.hpp"
#include "Mesh.h"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{

struct ObjectConstants
{
    float4x4 World{};
    float4x4 PreWorld{};
    float4x4 TexTransform{};
};

struct PassConstants
{
    float4x4 View{};
    float4x4 InvView{};
    float4x4 Proj{};
    float4x4 InvProj{};
    float4x4 ViewProj{};
    float4x4 InvViewProj{};
    float4x4 PreViewProj{};
    float4   EyePosW;          // xyz
    float4   RenderTargetSize; // xy : size, zw : InvRenderTargetSize
    float    NearZ     = 0.0f;
    float    FarZ      = 0.0f;
    float    TotalTime = 0.0f;
    float    DeltaTime = 0.0f;
    float4   ViewPortSize;

    float4 gFogColor;
    float  gFogStart;
    float  gFogRange;
    float2 cbPassPad2;
};

struct MaterialData
{
    float4   DiffuseAlbedo;
    float3   FresnelR0;
    float    Roughness;
    float4x4 MatTransform;

    float3 EmissiveColor;
    uint   ShadingModel;
};

struct SSAOPass
{
    // Coordinates given in view space.
    float gOcclusionRadius;
    float gOcclusionFadeStart;
    float gOcclusionFadeEnd;
    float gSurfaceEpsilon;
};

struct CbBlurSettings
{
    int gBlurRadius;

    // Support up to 11 blur weights.
    float w0;
    float w1;
    float w2;

    float w3;
    float w4;
    float w5;
    float w6;

    float w7;
    float w8;
    float w9;
    float w10;
};

struct LightParameters
{
    float3 Color;      // All light
    float  Intensity;  // All light
    float3 Position;   // Point/Spot light only
    float  Range;      // Point/Spot light only
    float3 Direction;  // Directional/Spot light only
    float  SpotRadius; // Spot light only
    float2 SpotAngles; // Spot light only
    uint   LightType;
    int    ShadowMapIdx;

    float4x4 LightProj;
    float4x4 ShadowTransform;

    // AreaLight
    float3 AreaLightPoint0InWorld;
    float  LightPad2;
    float3 AreaLightPoint1InWorld;
    float  LightPad3;
    float3 AreaLightPoint2InWorld;
    float  LightPad4;
    float3 AreaLightPoint3InWorld;
    float  LightPad5;
};

struct LightCommonData
{
    uint LightCount;
};

#define TILE_BLOCK_SIZE         16
#define MAX_LIGHT_COUNT_IN_TILE 500
struct TileLightInfo
{
    uint LightIndices[MAX_LIGHT_COUNT_IN_TILE];
    uint LightCount;
};

struct DeferredLighting
{
    uint EnableSSAO;
};

enum ELightType
{
    None,
    AmbientLight,
    DirectionalLight,
    PointLight,
    SpotLight,
    AreaLight
};

class Tutorial29_TBDR final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;
    virtual void WindowResize(Uint32 Width, Uint32 Height) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial29 : TBDR"; }

private:
    void InitLightData();
    void CreatePipelineState();
    void CreateBuffer();
    void CreateMesh();
    void LoadTexture();

    TMesh m_SphereMesh;
    TMesh m_GridMesh;

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<IBufferView>>            m_BufferViews;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    FirstPersonCamera m_Camera;
    MouseState        m_LastMouseState;

    uint32_t   mWidth = 1280;
    uint32_t   mHeight = 720;

    GLTF::Model* m_Model;

    std::vector<ObjectConstants> mObjectCbData;
    PassConstants                mPassCbData;
    MaterialData                 mMaterialData;
    SSAOPass                     mSSAOPass;
    CbBlurSettings               mCbBlurSettings;
    std::vector<LightParameters> mLightParameters;
    LightCommonData              mLightCommonData;
    std::vector<TileLightInfo>   mTileLightInfo;
    DeferredLighting             mDeferredLighting;
};

} // namespace Diligent
