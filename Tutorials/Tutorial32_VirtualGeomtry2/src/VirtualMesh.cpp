
#include "VirtualMesh.h"
#include "MeshSimplify/MeshSimplify.h"
#include "MeshSTL.h"

#include <cassert>

namespace Diligent
{

void log_cluster_size(Cluster* clusters, Uint32 begin, Uint32 end)
{
    float maxsz = 0, minsz = 100000, avgsz = 0;
    for (Uint32 i = begin; i < end; i++)
    {
        auto& cluster = clusters[i];
        assert(cluster.verts.size() < 256);
        float sz = cluster.indexes.size() / 3.0;
        if (sz > maxsz) maxsz = sz;
        if (sz < minsz) minsz = sz;
        avgsz += sz;
    }
    avgsz /= end - begin;
}

void log_group_size(ClusterGroup* groups, Uint32 begin, Uint32 end)
{
    float maxsz = 0, minsz = 100000, avgsz = 0;
    for (Uint32 i = begin; i < end; i++)
    {
        float sz = groups[i].clusters.size();
        if (sz > maxsz) maxsz = sz;
        if (sz < minsz) minsz = sz;
        avgsz += sz;
    }
    avgsz /= end - begin;
}

void VirtualMesh::build(MeshSTL& mesh)
{
    std::vector<Vertex>& pos = mesh.vtx;
    std::vector<Uint32>& idx = mesh.tris;

    //使用简化器并设置大于三角形数目的目标，去除重复点与三角形
    MeshSimplifier simplifier(pos.data(), pos.size(), idx.data(), idx.size());
    simplifier.simplify(idx.size());
    pos.resize(simplifier.remaining_num_vert());
    idx.resize(simplifier.remaining_num_tri() * 3);

    //将三角形分组为三角形簇
    cluster_triangles(pos, idx, clusters);

    Uint32 level_offset = 0, mip_level = 0;

    while (1)
    {
        log_cluster_size(clusters.data(), level_offset, clusters.size());

        Uint32 num_level_clusters = clusters.size() - level_offset;
        if (num_level_clusters <= 1) break;

        Uint32 prev_cluster_num = clusters.size();
        Uint32 prev_group_num   = cluster_groups.size();

        //将簇分组
        group_clusters(
            clusters,
            level_offset,
            num_level_clusters,
            cluster_groups,
            mip_level);
        log_group_size(cluster_groups.data(), prev_group_num, cluster_groups.size());

        //将组内簇合并并简化，生成上一级簇
        //print("building parent clusters: ");
        for (Uint32 i = prev_group_num; i < cluster_groups.size(); i++)
        {
            build_parent_clusters(cluster_groups[i], clusters);
        }

        level_offset = prev_cluster_num;
        mip_level++;
    }
    num_mip_level = mip_level + 1;
}

} // namespace Diligent