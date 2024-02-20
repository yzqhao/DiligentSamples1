
#pragma once

#include "SampleBase.hpp"
#include "BasicMath.hpp"
#include "GLTFLoader.hpp"
#include "FirstPersonCamera.hpp"

#include <map>

namespace Diligent
{
struct Directional_Light
{
    float strength;
    float3 direction;
    float3 color;
    float3 attenuation;
};

struct Scene
{
    const char*               name  = "";
    float                     scale = 1.0f;
    float3                    voxel_scale;
    BoundBox                  bounding_box;
    Directional_Light         directional_lights;
    std::vector<GLTF::Model*> models; // note: model memory owned by Assets.
};

}