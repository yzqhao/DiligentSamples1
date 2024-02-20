#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{

inline Uint32 cycle3(Uint32 i)
{
    Uint32 imod3 = i % 3;
    return i - imod3 + ((1 << imod3) & 3);
}
inline Uint32 cycle3(Uint32 i, Uint32 ofs)
{
    return i - i % 3 + (i + ofs) % 3;
}

} // namespace Diligent