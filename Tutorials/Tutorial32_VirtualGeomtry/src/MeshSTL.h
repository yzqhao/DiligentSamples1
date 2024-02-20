#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// 数据结构----顶点、法向、三角形
namespace Diligent
{

typedef float3 Vertex;
typedef float3 Normal;

struct VtxIdxSortItem
{
    int    i;
    Vertex value;
};

class MeshSTL
{
public:
    std::vector<Vertex> vtx;     /// 顶点
    std::vector<Uint32> tris;    /// 三角面片
    std::vector<Normal> vtxNrm;  /// 顶点法向
    std::vector<Normal> faceNrm; /// 面法向

    float3 center; /// 模型中心
    float  radius; /// 模型半径

    std::vector<Vertex> mkh_joints;
    std::vector<Vertex> scan_joints;
    std::string         modelname;
    std::string         scan_joint_path;

public:
    MeshSTL() {}
    ~MeshSTL() {}

    // 解析STL模型
    bool readSTL_ASCII(std::string cfilename);
    bool readSTL_Binary(std::string cfilename);
};

} // namespace Diligent