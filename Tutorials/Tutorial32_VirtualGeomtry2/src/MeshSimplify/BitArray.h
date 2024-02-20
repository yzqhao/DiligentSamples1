#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{
class BitArray
{
    Uint32* bits;

public:
    BitArray() { bits = nullptr; }
    BitArray(Uint32 size);
    ~BitArray() { free(); }
    void resize(Uint32 size);
    void free()
    {
        if (bits) delete[] bits;
    }
    void set_false(Uint32 idx)
    {
        Uint32 x = idx >> 5;
        Uint32 y = idx & 31;
        bits[x] &= ~(1 << y);
    }
    void set_true(Uint32 idx)
    {
        Uint32 x = idx >> 5;
        Uint32 y = idx & 31;
        bits[x] |= (1 << y);
    }
    bool operator[](Uint32 idx)
    {
        Uint32 x = idx >> 5;
        Uint32 y = idx & 31;
        return (bool)(bits[x] >> y & 1);
    }
};
} // namespace Diligent