
#include "Partitioner.h"
#include "metis.h"
#include <cassert>
#include <algorithm>

namespace Diligent
{

struct MetisGraph
{
    idx_t         nvtxs;
    std::vector<idx_t> xadj;
    std::vector<idx_t> adjncy; //压缩图表示
    std::vector<idx_t> adjwgt; //边权重
};

void Partitioner::init(Uint32 num_node)
{
    node_id.resize(num_node);
    sort_to.resize(num_node);
    Uint32 i = 0;
    for (Uint32& x : node_id)
    {
        x = i, i++;
    }
    i = 0;
    for (Uint32& x : sort_to)
    {
        x = i, i++;
    }
}

MetisGraph* to_metis_data(const Graph& graph)
{
    MetisGraph* g = new MetisGraph;
    g->nvtxs      = graph.g.size();
    for (auto& mp : graph.g)
    {
        g->xadj.push_back(g->adjncy.size());
        for (auto& kv : mp)
        {
            g->adjncy.push_back(kv.first);
            g->adjwgt.push_back(kv.second);
        }
    }
    g->xadj.push_back(g->adjncy.size());
    return g;
}

Uint32 Partitioner::bisect_graph(MetisGraph* graph_data, MetisGraph* child_graphs[2], Uint32 start, Uint32 end)
{
    assert(end - start == graph_data->nvtxs);

    if (graph_data->nvtxs <= max_part_size)
    {
        ranges.push_back({start, end});
        return end;
    }
    const Uint32 exp_part_size = (min_part_size + max_part_size) / 2;
    const Uint32 exp_num_parts = std::max(2u, (graph_data->nvtxs + exp_part_size - 1) / exp_part_size);

    std::vector<idx_t> swap_to(graph_data->nvtxs);
    std::vector<idx_t> part(graph_data->nvtxs);

    idx_t  nw = 1, npart = 2, ncut = 0;
    real_t part_weight[] = {
        float(exp_num_parts >> 1) / exp_num_parts,
        1.0f - float(exp_num_parts >> 1) / exp_num_parts};
    int res = METIS_PartGraphRecursive(
        &graph_data->nvtxs,
        &nw,
        graph_data->xadj.data(),
        graph_data->adjncy.data(),
        nullptr, //vert weights
        nullptr, //vert size
        graph_data->adjwgt.data(),
        &npart,
        part_weight, //partition weight
        nullptr,
        nullptr, //options
        &ncut,
        part.data());
    assert(res == METIS_OK);

    int l = 0, r = graph_data->nvtxs - 1;
    while (l <= r)
    {
        while (l <= r && part[l] == 0) swap_to[l] = l, l++;
        while (l <= r && part[r] == 1) swap_to[r] = r, r--;
        if (l < r)
        {
            std::swap(node_id[start + l], node_id[start + r]);
            swap_to[l] = r, swap_to[r] = l;
            l++, r--;
        }
    }
    int split = l;

    int size[2] = {split, graph_data->nvtxs - split};
    assert(size[0] >= 1 && size[1] >= 1);

    if (size[0] <= max_part_size && size[1] <= max_part_size)
    {
        ranges.push_back({start, start + split});
        ranges.push_back({start + split, end});
    }
    else
    {
        for (Uint32 i = 0; i < 2; i++)
        {
            child_graphs[i] = new MetisGraph;
            child_graphs[i]->adjncy.reserve(graph_data->adjncy.size() >> 1);
            child_graphs[i]->adjwgt.reserve(graph_data->adjwgt.size() >> 1);
            child_graphs[i]->xadj.reserve(size[i] + 1);
            child_graphs[i]->nvtxs = size[i];
        }
        for (Uint32 i = 0; i < graph_data->nvtxs; i++)
        {
            Uint32         is_rs = (i >= child_graphs[0]->nvtxs);
            Uint32         u     = swap_to[i];
            MetisGraph* ch    = child_graphs[is_rs];
            ch->xadj.push_back(ch->adjncy.size());
            for (Uint32 j = graph_data->xadj[u]; j < graph_data->xadj[u + 1]; j++)
            {
                idx_t v = graph_data->adjncy[j];
                idx_t w = graph_data->adjwgt[j];
                v       = swap_to[v] - (is_rs ? size[0] : 0);
                if (0 <= v && v < size[is_rs])
                {
                    ch->adjncy.push_back(v);
                    ch->adjwgt.push_back(w);
                }
            }
        }
        child_graphs[0]->xadj.push_back(child_graphs[0]->adjncy.size());
        child_graphs[1]->xadj.push_back(child_graphs[1]->adjncy.size());
    }
    return start + split;
}

void Partitioner::recursive_bisect_graph(MetisGraph* graph_data, Uint32 start, Uint32 end)
{
    MetisGraph* child_graphs[2] = {0};
    Uint32         split           = bisect_graph(graph_data, child_graphs, start, end);
    delete graph_data;

    if (child_graphs[0] && child_graphs[1])
    {
        recursive_bisect_graph(child_graphs[0], start, split);
        recursive_bisect_graph(child_graphs[1], split, end);
    }
    else
    {
        assert(!child_graphs[0] && !child_graphs[1]);
    }
}

void Partitioner::partition(const Graph& graph, Uint32 min_part_size, Uint32 max_part_size)
{
    init(graph.g.size());
    this->min_part_size    = min_part_size;
    this->max_part_size    = max_part_size;
    MetisGraph* graph_data = to_metis_data(graph);
    recursive_bisect_graph(graph_data, 0, graph_data->nvtxs);
    std::sort(ranges.begin(), ranges.end());
    for (Uint32 i = 0; i < node_id.size(); i++)
    {
        sort_to[node_id[i]] = i;
    }
}
} // namespace Diligent