
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "Mesh.h"

#include <map>

namespace Diligent
{

#define MAX_PLANETS 20 // Does not affect test, just for allocating space in uniform block. Must match with shader.

struct PlanetInfoStruct
{
	float4x4  mTranslationMat;
    float4x4 mScaleMat;
    float4x4  mSharedMat; // Matrix to pass down to children
    float4  mColor;
	uint  mParentIndex;
	float mYOrbitSpeed;    // Rotation speed around parent
	float mZOrbitSpeed;
	float mRotationSpeed;    // Rotation speed around self
};

struct UniformBlock
{
    float4x4 mProjectView;
    float4x4 mToWorldMat[MAX_PLANETS];
    float4 mColor[MAX_PLANETS];

	// Point Light Information
    float3 mLightPosition;
    float3 mLightColor;
};

struct UniformVirtualTextureInfo
{
	uint Width;
	uint Height;
	uint pageWidth;
	uint pageHeight;

	uint DebugMode;
	uint ID;
	uint pad1;
	uint pad2;
	
	float4 CameraPos;
};

struct UniformVirtualTextureBufferInfo
{
	uint TotalPageCount;
	uint CurrentFrameOffset;
};

class Tutorial03_Texturing final : public SampleBase
{
public:
    virtual void Initialize(const SampleInitInfo& InitInfo) override final;

    virtual void Render() override final;
    virtual void Update(double CurrTime, double ElapsedTime) override final;

    virtual const Char* GetSampleName() const override final { return "Tutorial03: Texturing"; }

private:
    void CreatePipelineState();
    void CreateMesh();
    void InitPlanet();
    void  LoadTexture();

    TMesh m_SphereMesh;

    std::map<std::string, RefCntAutoPtr<IPipelineState>>         m_pPSOs;
    std::map<std::string, RefCntAutoPtr<IBuffer>>                m_Buffers;
    std::map<std::string, RefCntAutoPtr<IBufferView>>            m_BufferViews;
    std::map<std::string, RefCntAutoPtr<ITexture>>               m_Textures;
    std::map<std::string, RefCntAutoPtr<ITextureView>>           m_TextureViews;
    std::map<std::string, RefCntAutoPtr<IShaderResourceBinding>> m_ShaderResourceBindings;

    UniformVirtualTextureInfo     mUniformVirtualTextureInfo;
    UniformBlock                  mUniformBlock;
    std::vector<PlanetInfoStruct> gPlanetInfoData;
};

} // namespace Diligent
