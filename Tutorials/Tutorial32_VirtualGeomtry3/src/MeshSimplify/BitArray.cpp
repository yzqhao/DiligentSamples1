#include "BitArray.h"

namespace Diligent
{
BitArray::BitArray(Uint32 size)
{
    bits = new Uint32[(size + 31) / 32];
    memset(bits, 0, (size + 31) / 32 * sizeof(Uint32));
}

void BitArray::resize(Uint32 size)
{
    free();
    bits = new Uint32[(size + 31) / 32];
    memset(bits, 0, (size + 31) / 32 * sizeof(Uint32));
}
} // namespace Diligent