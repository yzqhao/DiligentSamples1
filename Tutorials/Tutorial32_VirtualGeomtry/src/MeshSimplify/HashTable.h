#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{

inline Uint32 murmur_add(Uint32 hash, Uint32 elememt)
{
    elememt *= 0xcc9e2d51;
    elememt = (elememt << 15) | (elememt >> (32 - 15));
    elememt *= 0x1b873593;

    hash ^= elememt;
    hash = (hash << 13) | (hash >> (32 - 13));
    hash = hash * 5 + 0xe6546b64;
    return hash;
}

inline Uint32 murmur_mix(Uint32 hash)
{
    hash ^= hash >> 16;
    hash *= 0x85ebca6b;
    hash ^= hash >> 13;
    hash *= 0xc2b2ae35;
    hash ^= hash >> 16;
    return hash;
}

inline Uint32 lower_nearest_2_power(Uint32 x)
{
    while (x & (x - 1)) x ^= (x & -x);
    return x;
}

inline Uint32 upper_nearest_2_power(Uint32 x)
{
    if (x & (x - 1))
    {
        while (x & (x - 1)) x ^= (x & -x);
        return x == 0 ? 1 : (x << 1);
    }
    else
    {
        return x == 0 ? 1 : (x << 1);
    }
}

class HashTable
{
private:
    Uint32  hash_size;
    Uint32  hash_mask;
    Uint32  index_size;
    Uint32* hash;
    Uint32* next_index;
    void    resize_index(Uint32 _index_size);

public:
    HashTable(Uint32 _index_size = 0);
    HashTable(Uint32 _hash_size, Uint32 _index_size);
    ~HashTable();

    void resize(Uint32 _index_size);
    void resize(Uint32 _hash_size, Uint32 _index_size);

    void free()
    {
        hash_size  = 0;
        hash_mask  = 0;
        index_size = 0;
        delete[] hash;
        hash = nullptr;
        delete[] next_index;
        next_index = nullptr;
    }
    void clear();

    void add(Uint32 key, Uint32 idx)
    {
        if (idx >= index_size)
        {
            resize_index(upper_nearest_2_power(idx + 1));
        }
        key &= hash_mask;
        next_index[idx] = hash[key];
        hash[key]       = idx;
    }
    void remove(Uint32 key, Uint32 idx)
    {
        if (idx >= index_size) return;
        key &= hash_mask;
        if (hash[key] == idx) hash[key] = next_index[idx];
        else
        {
            for (Uint32 i = hash[key]; i != ~0u; i = next_index[i])
            {
                if (next_index[i] == idx)
                {
                    next_index[i] = next_index[idx];
                    break;
                }
            }
        }
    }

    struct Container
    {
        Uint32  idx;
        Uint32* next;
        struct iter
        {
            Uint32  idx;
            Uint32* next;
            void    operator++() { idx = next[idx]; }
            bool    operator!=(const iter& b) const { return idx != b.idx; }
            Uint32  operator*() { return idx; }
        };
        iter begin() { return iter{idx, next}; }
        iter end() { return iter{~0u, nullptr}; }
    };

    Container operator[](Uint32 key)
    {
        if (hash_size == 0 || index_size == 0) return Container{~0u, nullptr};
        key &= hash_mask;
        return Container{hash[key], next_index};
    }
};

} // namespace Diligent