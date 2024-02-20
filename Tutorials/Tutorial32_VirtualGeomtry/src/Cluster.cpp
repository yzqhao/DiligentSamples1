#include "Cluster.h"
#include "Partitioner.h"
#include "MeshUtil.h"
#include "MeshSimplify/HashTable.h"
#include "MeshSimplify/MeshSimplify.h"
#include "span.h"

#include <unordered_map>
#include <algorithm>
#include <cassert>

using namespace std;

namespace Diligent
{


inline Uint32 hash(float3 v)
{
    union
    {
        float  f;
        Uint32 u;
    } x, y, z;
    x.f = (v.x == 0.f ? 0 : v.x);
    y.f = (v.y == 0.f ? 0 : v.y);
    z.f = (v.z == 0.f ? 0 : v.z);
    return murmur_mix(murmur_add(murmur_add(x.u, y.u), z.u));
}

inline Uint32 hash(pair<float3, float3> e)
{
    Uint32 h0 = hash(e.first);
    Uint32 h1 = hash(e.second);
    return murmur_mix(murmur_add(h0, h1));
}

//将原来的数位以2个0分隔：10111->1000001001001，用于生成莫顿码
inline Uint32 expand_bits(Uint32 v)
{
    v = (v * 0x00010001u) & 0xFF0000FFu;
    v = (v * 0x00000101u) & 0x0F00F00Fu;
    v = (v * 0x00000011u) & 0xC30C30C3u;
    v = (v * 0x00000005u) & 0x49249249u;
    return v;
}
//莫顿码，要求 0<=x,y,z<=1
Uint32 morton3D(float3 p)
{
    Uint32 x = p.x * 1023, y = p.y * 1023, z = p.z * 1023;
    x = expand_bits(x);
    y = expand_bits(y);
    z = expand_bits(z);
    return (x << 2) | (y << 1) | (z << 1);
}

//边哈希，找到共享顶点且相反的边，代表两三角形相邻
void build_adjacency_edge_link(
    const vector<float3>& verts,
    const vector<Uint32>& indexes,
    Graph&                edge_link)
{
    HashTable edge_ht(indexes.size());
    edge_link.init(indexes.size());

    for (Uint32 i = 0; i < indexes.size(); i++)
    {
        float3 p0 = verts[indexes[i]];
        float3 p1 = verts[indexes[cycle3(i)]];
        edge_ht.add(hash({p0, p1}), i);

        for (Uint32 j : edge_ht[hash({p1, p0})])
        {
            if (p1 == verts[indexes[j]] && p0 == verts[indexes[cycle3(j)]])
            {
                edge_link.increase_edge_cost(i, j, 1);
                edge_link.increase_edge_cost(j, i, 1);
            }
        }
    }
}

// 根据边的邻接构建三角形的邻接图，边权为1，当需要加入local时需要adjacency边权足够大
void build_adjacency_graph(
    const Graph& edge_link,
    Graph&       graph)
{
    graph.init(edge_link.g.size() / 3);
    Uint32 u = 0;
    for (const auto& mp : edge_link.g)
    {
        for (auto& kv : mp)
        {
            auto v = kv.first;
            auto w = kv.second;
            graph.increase_edge_cost(u / 3, v / 3, 1);
        }
        u++;
    }
}

void cluster_triangles(
    const vector<float3>& verts,
    const vector<Uint32>& indexes,
    vector<Cluster>&      clusters)
{
    Graph edge_link, graph;
    build_adjacency_edge_link(verts, indexes, edge_link);
    build_adjacency_graph(edge_link, graph);

    Partitioner partitioner;
    partitioner.partition(graph, Cluster::cluster_size - 4, Cluster::cluster_size);

    // 根据划分结果构建clusters
    for (auto& kv : partitioner.ranges)
    {
        auto l = kv.first;
        auto r = kv.second;

        clusters.push_back({});
        Cluster& cluster = clusters.back();

        std::unordered_map<Uint32, Uint32> mp;
        for (Uint32 i = l; i < r; i++)
        {
            Uint32 t_idx = partitioner.node_id[i];
            for (Uint32 k = 0; k < 3; k++)
            {
                Uint32 e_idx = t_idx * 3 + k;
                Uint32 v_idx = indexes[e_idx];
                if (mp.find(v_idx) == mp.end())
                { //重映射顶点下标
                    mp[v_idx] = cluster.verts.size();
                    cluster.verts.push_back(verts[v_idx]);
                }
                bool is_external = false;
                for (auto& kv : edge_link.g[e_idx])
                {
                    auto adj_edge = kv.first;

                    Uint32 adj_tri = partitioner.sort_to[adj_edge / 3];
                    if (adj_tri < l || adj_tri >= r)
                    { //出点在不同划分说明是边界
                        is_external = true;
                        break;
                    }
                }
                if (is_external)
                {
                    cluster.external_edges.push_back(cluster.indexes.size());
                }
                cluster.indexes.push_back(mp[v_idx]);
            }
        }

        cluster.mip_level     = 0;
        cluster.lod_error     = 0;
        cluster.sphere_bounds = Sphere::from_points(cluster.verts.data(), cluster.verts.size());
        cluster.lod_bounds    = cluster.sphere_bounds;
        cluster.box_bounds    = cluster.verts[0];
        for (float3 p : cluster.verts) cluster.box_bounds = cluster.box_bounds + p;
    }
}

void build_clusters_edge_link(
    span<const Cluster>                 clusters,
    const vector<pair<Uint32, Uint32>>& ext_edges,
    Graph&                              edge_link)
{
    HashTable edge_ht(ext_edges.size());
    edge_link.init(ext_edges.size());

    Uint32 i = 0;
    for (auto& kv : ext_edges)
    {
        auto c_id = kv.first;
        auto e_id = kv.second;

        auto&  pos = clusters[c_id].verts;
        auto&  idx = clusters[c_id].indexes;
        float3 p0  = pos[idx[e_id]];
        float3 p1  = pos[idx[cycle3(e_id)]];
        edge_ht.add(hash({p0, p1}), i);
        for (Uint32 j : edge_ht[hash({p1, p0})])
        {
            auto  c_id1 = ext_edges[j].first;
            auto  e_id1 = ext_edges[j].second;
            auto& pos1  = clusters[c_id1].verts;
            auto& idx1  = clusters[c_id1].indexes;

            if (pos1[idx1[e_id1]] == p1 && pos1[idx1[cycle3(e_id1)]] == p0)
            {
                edge_link.increase_edge_cost(i, j, 1);
                edge_link.increase_edge_cost(j, i, 1);
            }
        }
        i++;
    }
}

void build_clusters_graph(
    const Graph&          edge_link,
    const vector<Uint32>& mp,
    Uint32                num_cluster,
    Graph&                graph)
{
    graph.init(num_cluster);
    Uint32 u = 0;
    for (const auto& emp : edge_link.g)
    {
        for (auto& kv : emp)
        {
            graph.increase_edge_cost(mp[u], mp[kv.first], 1);
        }
        u++;
    }
}

void group_clusters(
    vector<Cluster>&      clusters,
    Uint32                offset,
    Uint32                num_cluster,
    vector<ClusterGroup>& cluster_groups,
    Uint32                mip_level)
{
    span<const Cluster> clusters_view(&clusters[offset], num_cluster);

    //取出每个cluster的边界，并建立边id到簇id的映射
    std::vector<Uint32>               mp;  //edge_id to cluster_id
    std::vector<Uint32>               mp1; //cluster_id to first_edge_id
    std::vector<pair<Uint32, Uint32>> ext_edges;
    Uint32                            i = 0;
    for (auto& cluster : clusters_view)
    {
        assert(cluster.mip_level == mip_level);
        mp1.push_back(mp.size());
        for (Uint32 e : cluster.external_edges)
        {
            ext_edges.push_back({i, e});
            mp.push_back(i);
        }
        i++;
    }
    Graph edge_link, graph;
    build_clusters_edge_link(clusters_view, ext_edges, edge_link);
    build_clusters_graph(edge_link, mp, num_cluster, graph);

    Partitioner partitioner;
    partitioner.partition(graph, ClusterGroup::group_size - 4, ClusterGroup::group_size);

    //todo: 包围盒
    for (auto& range : partitioner.ranges)
    {
        auto l = range.first;
        auto r = range.second;

        cluster_groups.push_back({});
        auto& group     = cluster_groups.back();
        group.mip_level = mip_level;

        for (Uint32 i = l; i < r; i++)
        {
            Uint32 c_id                      = partitioner.node_id[i];
            clusters[c_id + offset].group_id = cluster_groups.size() - 1;
            group.clusters.push_back(c_id + offset);
            for (Uint32 e_idx = mp1[c_id]; e_idx < mp.size() && mp[e_idx] == c_id; e_idx++)
            {
                bool is_external = false;
                for (auto& kv : edge_link.g[e_idx])
                {
                    Uint32 adj_cl = partitioner.sort_to[mp[kv.first]];
                    if (adj_cl < l || adj_cl >= r)
                    {
                        is_external = true;
                        break;
                    }
                }
                if (is_external)
                {
                    Uint32 e = ext_edges[e_idx].second;
                    group.external_edges.push_back({c_id + offset, e});
                }
            }
        }
    }
}

void build_parent_clusters(
    ClusterGroup&         cluster_group,
    std::vector<Cluster>& clusters)
{
    vector<float3> pos;
    vector<Uint32> idx;
    vector<Sphere> lod_bounds;
    float          max_parent_lod_error = 0;
    Uint32         i_ofs                = 0;
    for (Uint32 c : cluster_group.clusters)
    {
        auto& cluster = clusters[c];
        for (float3 p : cluster.verts) pos.push_back(p);
        for (Uint32 i : cluster.indexes) idx.push_back(i + i_ofs);
        i_ofs += cluster.verts.size();
        lod_bounds.push_back(cluster.lod_bounds);
        max_parent_lod_error = std::max(max_parent_lod_error, cluster.lod_error); //强制父节点的error大于等于子节点
    }
    Sphere parent_lod_bound = Sphere::from_spheres(lod_bounds.data(), lod_bounds.size());

    MeshSimplifier simplifier(pos.data(), pos.size(), idx.data(), idx.size());
    HashTable      edge_ht(cluster_group.external_edges.size());
    Uint32         i = 0;

    for (auto& kv : cluster_group.external_edges)
    {
        auto c = kv.first;
        auto e = kv.second;

        auto&  pos = clusters[c].verts;
        auto&  idx = clusters[c].indexes;
        float3 p0 = pos[idx[e]], p1 = pos[idx[cycle3(e)]];
        edge_ht.add(hash({p0, p1}), i);
        simplifier.lock_position(p0);
        simplifier.lock_position(p1);
        i++;
    }

    simplifier.simplify((Cluster::cluster_size - 2) * (cluster_group.clusters.size() / 2));
    pos.resize(simplifier.remaining_num_vert());
    idx.resize(simplifier.remaining_num_tri() * 3);

    max_parent_lod_error = std::max(max_parent_lod_error, sqrt(simplifier.max_error()));

    Graph edge_link, graph;
    build_adjacency_edge_link(pos, idx, edge_link);
    build_adjacency_graph(edge_link, graph);

    Partitioner partitioner;
    partitioner.partition(graph, Cluster::cluster_size - 4, Cluster::cluster_size);

    for (auto& range : partitioner.ranges)
    {
        auto l = range.first;
        auto r = range.second;

        clusters.push_back({});
        Cluster& cluster = clusters.back();

        unordered_map<Uint32, Uint32> mp;
        for (Uint32 i = l; i < r; i++)
        {
            Uint32 t_idx = partitioner.node_id[i];
            for (Uint32 k = 0; k < 3; k++)
            {
                Uint32 e_idx = t_idx * 3 + k;
                Uint32 v_idx = idx[e_idx];
                if (mp.find(v_idx) == mp.end())
                { //重映射顶点下标
                    mp[v_idx] = cluster.verts.size();
                    cluster.verts.push_back(pos[v_idx]);
                }
                bool is_external = false;
                for (auto& kv : edge_link.g[e_idx])
                {
                    auto adj_edge = kv.first;

                    Uint32 adj_tri = partitioner.sort_to[adj_edge / 3];
                    if (adj_tri < l || adj_tri >= r)
                    { //出点在不同划分说明是边界
                        is_external = true;
                        break;
                    }
                }
                float3 p0 = pos[v_idx], p1 = pos[idx[cycle3(e_idx)]]; //
                if (!is_external)
                {
                    for (Uint32 j : edge_ht[hash({p0, p1})])
                    {
                        auto& external_edge = cluster_group.external_edges[j];

                        auto c = external_edge.first;
                        auto e = external_edge.second;

                        auto& pos = clusters[c].verts;
                        auto& idx = clusters[c].indexes;
                        if (p0 == pos[idx[e]] && p1 == pos[idx[cycle3(e)]])
                        {
                            is_external = true;
                            break;
                        }
                    }
                }

                if (is_external)
                {
                    cluster.external_edges.push_back(cluster.indexes.size());
                }
                cluster.indexes.push_back(mp[v_idx]);
            }
        }

        cluster.mip_level     = cluster_group.mip_level + 1;
        cluster.sphere_bounds = Sphere::from_points(cluster.verts.data(), cluster.verts.size());
        //强制父节点的lod包围盒覆盖所有子节点lod包围盒
        cluster.lod_bounds = parent_lod_bound;
        cluster.lod_error  = max_parent_lod_error;
        cluster.box_bounds = cluster.verts[0];
        for (float3 p : cluster.verts) cluster.box_bounds = cluster.box_bounds + p;
    }
    cluster_group.lod_bounds           = parent_lod_bound;
    cluster_group.max_parent_lod_error = max_parent_lod_error;
}

} // namespace Diligent