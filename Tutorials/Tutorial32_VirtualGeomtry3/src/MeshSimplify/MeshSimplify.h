#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{
class MeshSimplifier
{
    class MeshSimplifierImpl* impl;

public:
    MeshSimplifier() { impl = nullptr; }
    MeshSimplifier(float3* verts, Uint32 num_vert, Uint32* indexes, Uint32 num_index);
    ~MeshSimplifier();

    void lock_position(float3 p);
    void simplify(Uint32 target_num_tri);
    Uint32  remaining_num_vert();
    Uint32  remaining_num_tri();
    float  max_error();
};

} // namespace Diligent