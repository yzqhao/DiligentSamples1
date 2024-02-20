#include "MeshSTL.h"

namespace Diligent
{


using namespace std;

bool MeshSTL::readSTL_Binary(std::string fileName)
{
    ifstream ifs(fileName.c_str(), ios::binary);
    if (!ifs)
    {
        ifs.close();
        cout << "read stl error" << endl;
        return false;
    }

    vtx.clear();
    tris.clear();

    int intSize   = sizeof(int);
    int floatSize = sizeof(float);
    ifs.ignore(80);

    // 面的个数
    int num_tris;
    ifs.read((char*)(&num_tris), intSize);
    cout << "面片数量：" << num_tris << endl;

    float tn0, tn1, tn2;
    float v0, v1, v2;
    float cx = 0.0, cy = 0.0, cz = 0.0;

    vtx.reserve(num_tris * 3);
    tris.reserve(num_tris * 3);

    for (int i = 0; i < num_tris; i++)
    {
        ifs.read((char*)(&tn0), floatSize);
        ifs.read((char*)(&tn1), floatSize);
        ifs.read((char*)(&tn2), floatSize);

        //如果模型进行坐标变换，需要重新计算法向量
        // faceNrm.push_back(Normal(tn0, tn1, -tn2));

        // 01-STL model
        ifs.read((char*)(&v0), floatSize);
        ifs.read((char*)(&v1), floatSize);
        ifs.read((char*)(&v2), floatSize);

        vtx.push_back(Vertex(v0, v1, v2));
        cx += v0;
        cy += v1;
        cz += v2;

        ifs.read((char*)(&v0), floatSize);
        ifs.read((char*)(&v1), floatSize);
        ifs.read((char*)(&v2), floatSize);

        vtx.push_back(Vertex(v0, v1, v2));
        cx += v0;
        cy += v1;
        cz += v2;

        ifs.read((char*)(&v0), floatSize);
        ifs.read((char*)(&v1), floatSize);
        ifs.read((char*)(&v2), floatSize);

        vtx.push_back(Vertex(v0, v1, v2));
        cx += v0;
        cy += v1;
        cz += v2;

        // 建立面片索引，确定顶点顺序
        tris.push_back(i * 3 + 0);
        tris.push_back(i * 3 + 1);
        tris.push_back(i * 3 + 2);

        ifs.ignore(2);
    }
    ifs.close();

    return true;
}

bool MeshSTL::readSTL_ASCII(std::string fileName)
{
    ifstream ifs(fileName.c_str(), ios::binary);
    if (!ifs)
    {
        ifs.close();
        cout << "read stl error" << endl;
        return false;
    }

    vtx.clear();
    tris.clear();

    float tn0, tn1, tn2;
    float v0, v1, v2;
    float cx = 0.0, cy = 0.0, cz = 0.0;

    string name1, name2;
    ifs >> name1 >> name2;
    //cout << "name: " << name1 << " " << name2 <<  endl;
    int t = 0;

    while (!ifs.eof()) // ifs.good()
    {
        string temp2;
        ifs >> temp2;
        //cout << "temp2: " << temp2 << endl;
        if (temp2 == "facet")
        {
            string temp_normal;
            ifs >> temp_normal;
            ifs >> tn0 >> tn1 >> tn2;
            //cout << "normal: " << tn0 << " " << tn1 << " " << tn2 << endl;

            Normal faceN;
            faceN.x = tn0;
            faceN.y = tn1;
            faceN.z = tn2;
            faceNrm.push_back(faceN);

            ifs.ignore(11);
            string temp;
            ifs >> temp;
            //cout << "SSS:" << temp << endl;
            while (temp == "vertex")
            {
                //cout << "=========================" << endl;
                ifs >> v0 >> v1 >> v2;
                //cout << "vertex: " << v0 << " " << v1 << " " << v2 << endl;
                vtx.push_back(Vertex(v0, v1, v2));
                cx += v0;
                cy += v1;
                cz += v2;

                ifs >> temp;
                //cout << "temp: " << temp << endl;
            }
            //cout << "end1: " << temp << endl;
            ifs >> temp;
            //cout << "end2: " << temp << endl;

            {
                tris.push_back(t * 3 + 0);
                tris.push_back(t * 3 + 1);
                tris.push_back(t * 3 + 2);
                t = t + 1;
            }
        }

        cout << "end string " << temp2 << endl;
    }
    ifs.close();

    // 计算模型的中心位置
    center.x = cx / (tris.size() * 3);
    center.y = cy / (tris.size() * 3);
    center.z = cz / (tris.size() * 3);

    //计算模型的半径
    radius = 0;
    for (int i = 0; i < vtx.size(); i++)
    {
        vtx[i] = vtx[i] - center;
        float lens;
        lens = sqrt(dot(vtx[i], vtx[i]));
        if (lens > radius)
        {
            radius = lens;
        }
    }

    center.x = 0;
    center.y = 0;
    center.z = 0;

    return true;
}


}