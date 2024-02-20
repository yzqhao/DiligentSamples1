#include "MeshSimplify.h"
#include "BitArray.h"
#include "HashTable.h"
#include "Heap.h"

#include <vector>
#include <assert.h>
#include <algorithm>
#include "../MeshUtil.h"

namespace Diligent
{
double3 ToDVec3(const float3& f3)
{
    return double3(f3.x, f3.y, f3.z);
}

inline bool invertColumnMajor(const double m[16], double invOut[16])
{
    double inv[16], det;
    int i;

    inv[0]  = m[5] * m[10] * m[15] - m[5] * m[11] * m[14] - m[9] * m[6] * m[15] + m[9] * m[7] * m[14] + m[13] * m[6] * m[11] - m[13] * m[7] * m[10];
    inv[4]  = -m[4] * m[10] * m[15] + m[4] * m[11] * m[14] + m[8] * m[6] * m[15] - m[8] * m[7] * m[14] - m[12] * m[6] * m[11] + m[12] * m[7] * m[10];
    inv[8]  = m[4] * m[9] * m[15] - m[4] * m[11] * m[13] - m[8] * m[5] * m[15] + m[8] * m[7] * m[13] + m[12] * m[5] * m[11] - m[12] * m[7] * m[9];
    inv[12] = -m[4] * m[9] * m[14] + m[4] * m[10] * m[13] + m[8] * m[5] * m[14] - m[8] * m[6] * m[13] - m[12] * m[5] * m[10] + m[12] * m[6] * m[9];
    inv[1]  = -m[1] * m[10] * m[15] + m[1] * m[11] * m[14] + m[9] * m[2] * m[15] - m[9] * m[3] * m[14] - m[13] * m[2] * m[11] + m[13] * m[3] * m[10];
    inv[5]  = m[0] * m[10] * m[15] - m[0] * m[11] * m[14] - m[8] * m[2] * m[15] + m[8] * m[3] * m[14] + m[12] * m[2] * m[11] - m[12] * m[3] * m[10];
    inv[9]  = -m[0] * m[9] * m[15] + m[0] * m[11] * m[13] + m[8] * m[1] * m[15] - m[8] * m[3] * m[13] - m[12] * m[1] * m[11] + m[12] * m[3] * m[9];
    inv[13] = m[0] * m[9] * m[14] - m[0] * m[10] * m[13] - m[8] * m[1] * m[14] + m[8] * m[2] * m[13] + m[12] * m[1] * m[10] - m[12] * m[2] * m[9];
    inv[2]  = m[1] * m[6] * m[15] - m[1] * m[7] * m[14] - m[5] * m[2] * m[15] + m[5] * m[3] * m[14] + m[13] * m[2] * m[7] - m[13] * m[3] * m[6];
    inv[6]  = -m[0] * m[6] * m[15] + m[0] * m[7] * m[14] + m[4] * m[2] * m[15] - m[4] * m[3] * m[14] - m[12] * m[2] * m[7] + m[12] * m[3] * m[6];
    inv[10] = m[0] * m[5] * m[15] - m[0] * m[7] * m[13] - m[4] * m[1] * m[15] + m[4] * m[3] * m[13] + m[12] * m[1] * m[7] - m[12] * m[3] * m[5];
    inv[14] = -m[0] * m[5] * m[14] + m[0] * m[6] * m[13] + m[4] * m[1] * m[14] - m[4] * m[2] * m[13] - m[12] * m[1] * m[6] + m[12] * m[2] * m[5];
    inv[3]  = -m[1] * m[6] * m[11] + m[1] * m[7] * m[10] + m[5] * m[2] * m[11] - m[5] * m[3] * m[10] - m[9] * m[2] * m[7] + m[9] * m[3] * m[6];
    inv[7]  = m[0] * m[6] * m[11] - m[0] * m[7] * m[10] - m[4] * m[2] * m[11] + m[4] * m[3] * m[10] + m[8] * m[2] * m[7] - m[8] * m[3] * m[6];
    inv[11] = -m[0] * m[5] * m[11] + m[0] * m[7] * m[9] + m[4] * m[1] * m[11] - m[4] * m[3] * m[9] - m[8] * m[1] * m[7] + m[8] * m[3] * m[5];
    inv[15] = m[0] * m[5] * m[10] - m[0] * m[6] * m[9] - m[4] * m[1] * m[10] + m[4] * m[2] * m[9] + m[8] * m[1] * m[6] - m[8] * m[2] * m[5];

    det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

    if (det == 0) return false;

    det = 1.f / det;

    for (i = 0; i < 16; i++)
        invOut[i] = inv[i] * det;

    return true;
}

struct Quadric
{
    double a2, b2, c2, d2;
    double ab, ac, ad;
    double bc, bd, cd;
    Quadric() { memset(this, 0, sizeof(double) * 10); }
    Quadric(double3 p0, double3 p1, double3 p2)
    {
        double3 n        = normalize(cross(p1 - p0, p2 - p0));
        double a = n.x, b = n.y, c = n.z;
        double d          = -dot(n, p0);
        a2 = a * a, b2 = b * b, c2 = c * c, d2 = d * d;
        ab = a * b, ac = a * c, ad = a * d;
        bc = b * c, bd = b * d, cd = c * d;
    }
    void add(Quadric b)
    {
        double* t1 = (double*)this;
        double* t2 = (double*)&b;
        for (Uint32 i = 0; i < 10; i++) t1[i] += t2[i];
    }
    bool get(float3& p)
    {
        double4x4 m(a2, ab, ac, 0,
                    ab, b2, bc, 0,
                    ac, bc, c2, 0,
                    ad, bd, cd, 1);
        double4x4 inv;// = m.Inverse();
        invertColumnMajor((double*)&m, (double*)&inv);

        double4 v(inv._41, inv._42, inv._43, inv._43);
        p       = {(float)v.x, (float)v.y, (float)v.z};
        return true;
    }
    float evaluate(float3 p)
    {
        float res = a2 * p.x * p.x + 2 * ab * p.x * p.y + 2 * ac * p.x * p.z + 2 * ad * p.x + b2 * p.y * p.y + 2 * bc * p.y * p.z + 2 * bd * p.y + c2 * p.z * p.z + 2 * cd * p.z + d2;
        return res <= 0.f ? 0.f : res;
    }
};

class MeshSimplifierImpl 
{
public:
    Uint32 num_vert;
    Uint32 num_index;
    Uint32 num_tri;

    float3* verts;
    Uint32*  indexes;

    HashTable   vert_ht;
    HashTable   corner_ht;
    std::vector<Uint32> vert_refs;
    std::vector<Uint8>  flags;
    BitArray    tri_removed;

    enum flag
    {
        AdjMask  = 1,
        LockMask = 2
    };

    std::vector<std::pair<float3, float3>> edges;
    HashTable                edge0_ht;
    HashTable                edge1_ht;
    Heap                     heap;

    std::vector<Uint32> move_vert;
    std::vector<Uint32> move_corner;
    std::vector<Uint32> move_edge;
    std::vector<Uint32> reevaluate_edge;

    std::vector<Quadric> tri_quadrics;

    float max_error;
    Uint32 remaining_num_vert;
    Uint32 remaining_num_tri;

    MeshSimplifierImpl(float3* verts, Uint32 num_vert, Uint32* indexes, Uint32 num_index);
    ~MeshSimplifierImpl() {}

    Uint32 hash(const float3& v)
    {
        union
        {
            float f;
            Uint32 u;
        } x, y, z;
        x.f = (v.x == 0.f ? 0 : v.x);
        y.f = (v.y == 0.f ? 0 : v.y);
        z.f = (v.z == 0.f ? 0 : v.z);
        return murmur_mix(murmur_add(murmur_add(x.u, y.u), z.u));
    }
    void set_vert_idx(Uint32 corner, Uint32 idx);
    void remove_if_vert_duplicate(Uint32 corner);
    bool is_tri_duplicate(Uint32 tri_idx);
    void fixup_tri(Uint32 tri_idx);
    bool add_edge_ht(float3& p0, float3& p1, Uint32 idx);

    void calc_tri_quadric(Uint32 tri_idx);
    void  gather_adj_tris(float3 p, std::vector<Uint32>& tris, bool& lock);
    float  evaluate(float3 p0, float3 p1, bool merge);
    void lock_position(float3 p);
    // bool is_position_locked(float3 p);
    void simplify(Uint32 target_num_tri);
    void compact();

    void begin_merge(float3 p);
    void end_merge();
};

MeshSimplifierImpl::MeshSimplifierImpl(float3* _verts, Uint32 _num_vert, Uint32* _indexes, Uint32 _num_index) :
    num_vert(_num_vert), num_index(_num_index), num_tri(num_index / 3), verts(_verts), indexes(_indexes), vert_ht(num_vert), vert_refs(num_vert), corner_ht(num_index), tri_removed(num_tri), flags(num_index)
{
    remaining_num_vert = num_vert, remaining_num_tri = num_tri;
    for (Uint32 i = 0; i < num_vert; i++)
    {
        vert_ht.add(hash(verts[i]), i);
    }

    Uint32 exp_num_edge = std::min(std::min(num_index, 3 * num_vert - 6), num_tri + num_vert);
    edges.reserve(exp_num_edge);
    edge0_ht.resize(exp_num_edge);
    edge1_ht.resize(exp_num_edge);

    for (Uint32 corner = 0; corner < num_index; corner++)
    {
        Uint32 v_idx = indexes[corner];
        vert_refs[v_idx]++;
        const float3& p = verts[v_idx];
        corner_ht.add(hash(p), corner);

        float3 p0 = p;
        float3 p1 = verts[indexes[cycle3(corner)]];
        if (add_edge_ht(p0, p1, edges.size()))
        {
            edges.push_back({p0, p1});
        }
    }
}

void MeshSimplifierImpl::lock_position(float3 p)
{
    for (Uint32 i : corner_ht[hash(p)])
    {
        if (verts[indexes[i]] == p)
        {
            flags[i] |= LockMask;
        }
    }
}

bool MeshSimplifierImpl::add_edge_ht(float3& p0, float3& p1, Uint32 idx)
{
    Uint32 h0 = hash(p0), h1 = hash(p1);
    if (h0 > h1) std::swap(h0, h1), std::swap(p0, p1);
    for (Uint32 i : edge0_ht[h0])
    {
        auto& e = edges[i];
        if (e.first == p0 && e.second == p1) return false;
    }
    edge0_ht.add(h0, idx);
    edge1_ht.add(h1, idx);
    return true;
}

void MeshSimplifierImpl::set_vert_idx(Uint32 corner, Uint32 idx)
{
    Uint32& v_idx = indexes[corner];
    assert(v_idx != ~0u);
    assert(vert_refs[v_idx] > 0);

    if (v_idx == idx) return;
    if (--vert_refs[v_idx] == 0)
    {
        vert_ht.remove(hash(verts[v_idx]), v_idx);
        remaining_num_vert--;
    }
    v_idx = idx;
    if (v_idx != ~0u) vert_refs[v_idx]++;
}

//将corner赋值为第一个遇到的相同点
void MeshSimplifierImpl::remove_if_vert_duplicate(Uint32 corner)
{
    Uint32   v_idx = indexes[corner];
    float3& v     = verts[v_idx];
    for (Uint32 i : vert_ht[hash(v)])
    {
        if (i == v_idx) break;
        if (v == verts[i])
        {
            set_vert_idx(corner, i);
            break;
        }
    }
}

bool MeshSimplifierImpl::is_tri_duplicate(Uint32 tri_idx)
{
    Uint32 i0 = indexes[tri_idx * 3 + 0], i1 = indexes[tri_idx * 3 + 1], i2 = indexes[tri_idx * 3 + 2];
    for (Uint32 i : corner_ht[hash(verts[i0])])
    {
        if (i != tri_idx * 3)
        {
            if (i0 == indexes[i] && i1 == indexes[cycle3(i)] && i2 == indexes[(cycle3(i, 2))])
                return true;
        }
    }
    return false;
}

void MeshSimplifierImpl::fixup_tri(Uint32 tri_idx)
{
    assert(!tri_removed[tri_idx]);

    const float3& p0 = verts[indexes[tri_idx * 3 + 0]];
    const float3& p1 = verts[indexes[tri_idx * 3 + 1]];
    const float3& p2 = verts[indexes[tri_idx * 3 + 2]];

    bool is_removed = false;
    if (!is_removed)
    {
        is_removed = (p0 == p1) || (p1 == p2) || (p2 == p0);
    }
    if (!is_removed)
    {
        for (Uint32 k = 0; k < 3; k++) remove_if_vert_duplicate(tri_idx * 3 + k);
        is_removed = is_tri_duplicate(tri_idx);
    }
    if (is_removed)
    {
        tri_removed.set_true(tri_idx);
        remaining_num_tri--;
        for (Uint32 k = 0; k < 3; k++)
        {
            Uint32 corner = tri_idx * 3 + k;
            Uint32 v_idx  = indexes[corner];
            corner_ht.remove(hash(verts[v_idx]), corner);
            set_vert_idx(corner, ~0u);
        }
    }
    else
        tri_quadrics[tri_idx] = Quadric(ToDVec3(p0), ToDVec3(p1), ToDVec3(p2));
}

void MeshSimplifierImpl::gather_adj_tris(float3 p, std::vector<Uint32>& tris, bool& lock)
{
    for (Uint32 i : corner_ht[hash(p)])
    {
        if (verts[indexes[i]] == p)
        {
            Uint32 tri_idx = i / 3;
            if ((flags[tri_idx * 3] & AdjMask) == 0)
            {
                flags[tri_idx * 3] |= AdjMask;
                tris.push_back(tri_idx);
            }
            if (flags[i] & LockMask)
            {
                lock = true;
            }
        }
    }
}

float MeshSimplifierImpl::evaluate(float3 p0, float3 p1, bool merge)
{
    if (p0 == p1) return 0.f;

    float error = 0;

    std::vector<Uint32> adj_tris;
    bool        lock0 = false, lock1 = false;
    gather_adj_tris(p0, adj_tris, lock0);
    gather_adj_tris(p1, adj_tris, lock1);
    if (adj_tris.size() == 0) return 0.f;
    if (adj_tris.size() > 24)
    {
        error += 0.5 * (adj_tris.size() - 24);
    }

    Quadric q;
    for (Uint32 i : adj_tris)
    {
        q.add(tri_quadrics[i]);
    }
    float3 p = (p0 + p1) * 0.5f;

    auto is_valid_pos = [&](float3 p) -> bool {
        if (length(p - p0) + length(p - p1) > 2 * length(p0 - p1))
            return false;
        return true;
    };

    if (lock0 && lock1) error += 1e8;
    if (lock0 && !lock1) p = p0;
    else if (!lock0 && lock1)
        p = p1;
    else if (!q.get(p))
        p = (p0 + p1) * 0.5f;
    if (!is_valid_pos(p))
    {
        p = (p0 + p1) * 0.5f;
    }
    error += q.evaluate(p);

    if (merge)
    {
        begin_merge(p0), begin_merge(p1);
        for (Uint32 i : adj_tris)
        {
            for (Uint32 k = 0; k < 3; k++)
            {
                Uint32   corner = i * 3 + k;
                float3& pos    = verts[indexes[corner]];
                if (pos == p0 || pos == p1)
                {
                    pos = p;
                    if (lock0 || lock1) flags[corner] |= LockMask;
                }
            }
        }
        for (Uint32 i : move_edge)
        {
            auto& e = edges[i];
            if (e.first == p0 || e.first == p1) e.first = p;
            if (e.second == p0 || e.second == p1) e.second = p;
        }
        end_merge();

        std::vector<Uint32> adj_verts;
        for (Uint32 i : adj_tris)
        {
            for (Uint32 k = 0; k < 3; k++)
            {
                adj_verts.push_back(indexes[i * 3 + k]);
            }
        }
        sort(adj_verts.begin(), adj_verts.end());
        adj_verts.erase(unique(adj_verts.begin(), adj_verts.end()), adj_verts.end());

        for (Uint32 v_idx : adj_verts)
        {
            Uint32 h = hash(verts[v_idx]);
            for (Uint32 i : edge0_ht[h])
            {
                if (edges[i].first == verts[v_idx])
                {
                    if (heap.is_present(i))
                    {
                        heap.remove(i);
                        reevaluate_edge.push_back(i);
                    }
                }
            }
            for (Uint32 i : edge1_ht[h])
            {
                if (edges[i].second == verts[v_idx])
                {
                    if (heap.is_present(i))
                    {
                        heap.remove(i);
                        reevaluate_edge.push_back(i);
                    }
                }
            }
        }
        for (Uint32 i : adj_tris)
        {
            fixup_tri(i);
        }
    }
    for (Uint32 i : adj_tris)
    {
        flags[i * 3] &= (~AdjMask);
    }
    return error;
}

void MeshSimplifierImpl::begin_merge(float3 p)
{
    Uint32 h = hash(p);
    for (Uint32 i : vert_ht[h])
    {
        if (verts[i] == p)
        {
            vert_ht.remove(h, i);
            move_vert.push_back(i);
        }
    }
    for (Uint32 i : corner_ht[h])
    {
        if (verts[indexes[i]] == p)
        {
            corner_ht.remove(h, i);
            move_corner.push_back(i);
        }
    }
    for (Uint32 i : edge0_ht[h])
    {
        if (edges[i].first == p)
        {
            edge0_ht.remove(hash(edges[i].first), i);
            edge1_ht.remove(hash(edges[i].second), i);
            move_edge.push_back(i);
        }
    }
    for (Uint32 i : edge1_ht[h])
    {
        if (edges[i].second == p)
        {
            edge0_ht.remove(hash(edges[i].first), i);
            edge1_ht.remove(hash(edges[i].second), i);
            move_edge.push_back(i);
        }
    }
}

void MeshSimplifierImpl::end_merge()
{
    for (Uint32 i : move_vert)
    {
        vert_ht.add(hash(verts[i]), i);
    }
    for (Uint32 i : move_corner)
    {
        corner_ht.add(hash(verts[indexes[i]]), i);
    }
    for (Uint32 i : move_edge)
    {
        auto& e = edges[i];
        if (e.first == e.second || !add_edge_ht(e.first, e.second, i))
        {
            heap.remove(i);
        }
    }
    move_vert.clear();
    move_corner.clear();
    move_edge.clear();
}

void MeshSimplifierImpl::simplify(Uint32 target_num_tri)
{
    tri_quadrics.resize(num_tri);
    for (Uint32 i = 0; i < num_tri; i++) fixup_tri(i);
    if (remaining_num_tri <= target_num_tri)
    {
        compact();
        return;
    }
    heap.resize(edges.size());
    Uint32 i = 0;
    for (auto& e : edges)
    {
        float error = evaluate(e.first, e.second, false);
        heap.add(error, i);
        i++;
    }

    max_error = 0;
    while (!heap.empty())
    {
        Uint32 e_idx = heap.top();
        if (heap.get_key(e_idx) >= 1e6) break;

        heap.pop();

        auto& e = edges[e_idx];
        edge0_ht.remove(hash(e.first), e_idx);
        edge1_ht.remove(hash(e.second), e_idx);

        float error = evaluate(e.first, e.second, true);
        if (error > max_error) max_error = error;

        if (remaining_num_tri <= target_num_tri) break;

        for (Uint32 i : reevaluate_edge)
        {
            auto& e     = edges[i];
            float   error = evaluate(e.first, e.second, false);
            heap.add(error, i);
        }
        reevaluate_edge.clear();
    }
    compact();
}

void MeshSimplifierImpl::compact()
{
    Uint32 v_cnt = 0;
    for (Uint32 i = 0; i < num_vert; i++)
    {
        if (vert_refs[i] > 0)
        {
            if (i != v_cnt) verts[v_cnt] = verts[i];
            //重用作下标
            vert_refs[i] = v_cnt++;
        }
    }
    assert(v_cnt == remaining_num_vert);

    Uint32 t_cnt = 0;
    for (Uint32 i = 0; i < num_tri; i++)
    {
        if (!tri_removed[i])
        {
            for (Uint32 k = 0; k < 3; k++)
            {
                indexes[t_cnt * 3 + k] = vert_refs[indexes[i * 3 + k]];
            }
            t_cnt++;
        }
    }
    assert(t_cnt == remaining_num_tri);
}

MeshSimplifier::MeshSimplifier(float3* verts, Uint32 num_vert, Uint32* indexes, Uint32 num_index)
{
    impl = new MeshSimplifierImpl(verts, num_vert, indexes, num_index);
}

MeshSimplifier::~MeshSimplifier()
{
    if (impl) delete (MeshSimplifierImpl*)impl;
}

void MeshSimplifier::lock_position(float3 p)
{
    ((MeshSimplifierImpl*)impl)->lock_position(p);
}

void MeshSimplifier::simplify(Uint32 target_num_tri)
{
    ((MeshSimplifierImpl*)impl)->simplify(target_num_tri);
}

Uint32 MeshSimplifier::remaining_num_vert()
{
    return ((MeshSimplifierImpl*)impl)->remaining_num_vert;
}

Uint32 MeshSimplifier::remaining_num_tri()
{
    return ((MeshSimplifierImpl*)impl)->remaining_num_tri;
}

float MeshSimplifier::max_error()
{
    return ((MeshSimplifierImpl*)impl)->max_error;
}

} // namespace Diligent