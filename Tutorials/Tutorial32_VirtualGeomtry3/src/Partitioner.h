#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

#include <map>

namespace Diligent
{
struct Graph
{
    std::vector<std::map<Uint32, int>> g;

    void init(Uint32 n)
    {
        g.resize(n);
    }
    void add_node()
    {
        g.push_back({});
    }
    void add_edge(Uint32 from, Uint32 to, int cost)
    {
        g[from][to] = cost;
    }
    void increase_edge_cost(Uint32 from, Uint32 to, int i_cost)
    {
        g[from][to] += i_cost;
    }
};

struct MetisGraph;

class Partitioner
{
    Uint32  bisect_graph(MetisGraph* graph_data, MetisGraph* child_graphs[2], Uint32 start, Uint32 end);
    void recursive_bisect_graph(MetisGraph* graph_data, Uint32 start, Uint32 end);

public:
    void init(Uint32 num_node);
    void partition(const Graph& graph, Uint32 min_part_size, Uint32 max_part_size);

    std::vector<Uint32>                 node_id; //将节点按划分编号排序
    std::vector<std::pair<Uint32, Uint32>> ranges;  //分块的连续范围，范围内是相同划分
    std::vector<Uint32>                 sort_to;
    Uint32                              min_part_size;
    Uint32                              max_part_size;
};
    
}
