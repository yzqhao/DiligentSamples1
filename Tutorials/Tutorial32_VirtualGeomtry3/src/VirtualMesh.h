#pragma once

#include "Cluster.h"

namespace Diligent
{

class MeshSTL;

struct VirtualMesh
{
    std::vector<Cluster> clusters;
    std::vector<ClusterGroup> cluster_groups;
    Uint32 num_mip_level;

    void build(MeshSTL& mesh);
};

}