cmake_minimum_required (VERSION 3.6)

project(Tutorial37_SDF CXX)

set(SOURCE
    src/Tutorial37_SDF.cpp
    src/BVHTree.cpp
)

set(INCLUDE
    src/Tutorial37_SDF.hpp
    src/BVHTree.h
)

set(SHADERS
    #SDFRenderer.hlsl
    #assets/cube.psh
)

add_sample_app("Tutorial37_SDF" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial37_SDF PRIVATE Diligent-TextureLoader Diligent-AssetLoader)