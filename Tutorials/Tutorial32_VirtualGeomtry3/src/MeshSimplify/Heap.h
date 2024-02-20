#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{
class Heap
{
    Uint32  heap_size;
    Uint32  num_index;
    Uint32* heap;
    float* keys;
    Uint32* heap_indexes;

    void push_up(Uint32 i);
    void push_down(Uint32 i);

public:
    Heap();
    Heap(Uint32 _num_index);
    ~Heap() { free(); }

    void free()
    {
        heap_size = 0, num_index = 0;
        delete[] heap;
        delete[] keys;
        delete[] heap_indexes;
        heap = nullptr, keys = nullptr, heap_indexes = nullptr;
    }
    void resize(Uint32 _num_index);

    float  get_key(Uint32 idx);
    void clear();
    bool empty() { return heap_size == 0; }
    bool is_present(Uint32 idx) { return heap_indexes[idx] != ~0u; }
    Uint32  top();
    void pop();
    void add(float key, Uint32 idx);
    void update(float key, Uint32 idx);
    void remove(Uint32 idx);
};
} // namespace Diligent