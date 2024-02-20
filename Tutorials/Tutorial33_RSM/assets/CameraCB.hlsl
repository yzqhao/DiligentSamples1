
#define MaxLights 16

#define SAMPLE_NUMBER 120

struct DirectionalLight
{
	float4 mPos;
	float4 mCol; //alpha is the intesity
	float4 mDir;
};

struct PointLight
{
	float4 pos;
	float4 col;
	float radius;
	float intensity;
	float _pad0;
	float _pad1;
};

// Constant data that varies per material.
cbuffer cbPass : register(b1)
{
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float4x4 gViewProj;
    float4x4 gPreViewProj;
    float4x4 gInvViewProj;
    float4x4 gLightView;
    float4x4 gLightProj;
    float3 gEyePosW;
    float cbPerObjectPad1;
    float2 gRenderTargetSize;
    float2 gInvRenderTargetSize;
    float gNearZ;
    float gFarZ;
    float gTotalTime;
    float gDeltaTime;
    float4 gViewPortSize;
    float4 gAmbientLight;

    DirectionalLight gDLights[MaxLights];
    PointLight gPLights[MaxLights];
    int gAmountOfDLights;
    int gAmountOfPLights;
	int gRSM;
	int gFilterRange;
	
	float4 randomArray[SAMPLE_NUMBER];
};