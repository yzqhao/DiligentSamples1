#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

#include "BasicMath.hpp"
#include "AdvancedMath.hpp"

namespace Diligent
{
struct TVertex
{
    TVertex() {}
    TVertex(
        const float3& p,
        const float3& n,
        const float3& t,
        const float2& uv) :
        Position(p),
        Normal(n),
        TangentU(t),
        TexC(uv) {}
    TVertex(
        float px,
        float py,
        float pz,
        float nx,
        float ny,
        float nz,
        float tx,
        float ty,
        float tz,
        float u,
        float v) :
        Position(px, py, pz),
        Normal(nx, ny, nz),
        TangentU(tx, ty, tz),
        TexC(u, v) {}

    float3 Position;
    float3 Normal;
    float3 TangentU;
    float2 TexC;
};

struct TMeshSDFDescriptor
{
    float3 Center;
    float    Extent;

    int Resolution;
    int pad1;
    int pad2;
    int pad3;
};

class TMesh
{
public:
    using uint16 = std::uint16_t;
    using uint32 = std::uint32_t;

    TMesh();

    TMesh(TMesh&&) = default;

    TMesh(const TMesh&) = delete;

    TMesh& operator=(const TMesh&) = delete;

public:
    /// Creates a box centered at the origin with the given dimensions, where each
    /// face has m rows and n columns of vertices.
    void CreateBox(float width, float height, float depth, uint32 numSubdivisions);

    /// Creates a sphere centered at the origin with the given radius.  The
    /// slices and stacks parameters control the degree of tessellation.
    void CreateSphere(float radius, uint32 sliceCount, uint32 stackCount);


    /// Creates a cylinder parallel to the y-axis, and centered about the origin.
    /// The bottom and top radius can vary to form various cone shapes rather than true
    // cylinders.  The slices and stacks parameters control the degree of tessellation.
    void CreateCylinder(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);


    /// Creates an mxn grid in the xz-plane with m rows and n columns, centered
    /// at the origin with the specified width and depth.
    void CreateGrid(float width, float depth, uint32 m, uint32 n);

    /// Creates a quad aligned with the screen.  This is useful for postprocessing and screen effects.
    void CreateQuad(float x, float y, float w, float h, float depth);

    const std::vector<uint16>& GetIndices16() const;

    std::string GetInputLayoutName() const;

    void GenerateIndices16();

public:
    void GenerateBoundingBox();

    BoundBox GetBoundingBox() { return BoundingBox; }

private:
    void Subdivide();

    TVertex MidPoint(const TVertex& v0, const TVertex& v1);

    void BuildCylinderTopCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

    void BuildCylinderBottomCap(float bottomRadius, float topRadius, float height, uint32 sliceCount, uint32 stackCount);

public:
    std::string MeshName;

    std::vector<TVertex> Vertices;

    std::vector<uint32> Indices32;

    std::vector<uint16> Indices16;

    std::string InputLayoutName;

    BoundBox BoundingBox;

    TMeshSDFDescriptor SDFDescriptor;
};

} // namespace Diligent