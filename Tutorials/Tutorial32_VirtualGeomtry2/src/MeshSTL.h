#pragma once

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

#include <iostream>
#include <math.h>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>

// ���ݽṹ----���㡢����������
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
    std::vector<Vertex> vtx;     /// ����
    std::vector<Uint32> tris;    /// ������Ƭ
    std::vector<Normal> vtxNrm;  /// ���㷨��
    std::vector<Normal> faceNrm; /// �淨��

    float3 center; /// ģ������
    float  radius; /// ģ�Ͱ뾶

    std::vector<Vertex> mkh_joints;
    std::vector<Vertex> scan_joints;
    std::string         modelname;
    std::string         scan_joint_path;

public:
    MeshSTL() {}
    ~MeshSTL() {}

    // ����STLģ��
    bool readSTL_ASCII(std::string cfilename);
    bool readSTL_Binary(std::string cfilename);
};

} // namespace Diligent