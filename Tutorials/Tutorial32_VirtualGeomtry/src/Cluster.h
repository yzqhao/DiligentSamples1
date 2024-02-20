#pragma once

#include "Bounds.h"

namespace Diligent
{

struct Cluster
{
    static const Uint32 cluster_size = 128;

    std::vector<float3> verts;
    std::vector<Uint32> indexes;
    std::vector<Uint32> external_edges;

    BoundsAABB box_bounds;
    Sphere     sphere_bounds;
    Sphere     lod_bounds;
    float      lod_error;
    Uint32     mip_level;
    Uint32     group_id;
};

struct ClusterGroup
{
    // static const Uint32 min_group_size=8;
    static const Uint32 group_size = 32;

    Sphere                                 bounds;
    Sphere                                 lod_bounds;
    float                                  min_lod_error;
    float                                  max_parent_lod_error;
    Uint32                                 mip_level;
    std::vector<Uint32>                    clusters;       //对cluster数组的下标
    std::vector<std::pair<Uint32, Uint32>> external_edges; //first: cluster id, second: edge id
};

void cluster_triangles(
    const std::vector<float3>& verts,
    const std::vector<Uint32>& indexes,
    std::vector<Cluster>&      clusters);

void group_clusters(
    std::vector<Cluster>&      clusters,
    Uint32                     offset,
    Uint32                     num_cluster,
    std::vector<ClusterGroup>& cluster_groups,
    Uint32                     mip_level);

void build_parent_clusters(
    ClusterGroup&         cluster_group,
    std::vector<Cluster>& clusters);

} // namespace Diligent
