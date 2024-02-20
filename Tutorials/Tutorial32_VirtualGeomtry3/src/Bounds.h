#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{

struct BoundsAABB
{
    float3 pmin, pmax;
    BoundsAABB() { pmin = {1e9, 1e9, 1e9}, pmax = {-1e9, -1e9, -1e9}; }
    BoundsAABB(float3 p) { pmin = p, pmax = p; }
    BoundsAABB operator+(BoundsAABB b);
    BoundsAABB operator+(float3 b);
};

struct Sphere
{
    float3 center;
    float  radius;

    Sphere        operator+(Sphere b);
    static Sphere from_points(float3* pos, Uint32 size);
    static Sphere from_spheres(Sphere* spheres, Uint32 size);
};

} // namespace Diligent