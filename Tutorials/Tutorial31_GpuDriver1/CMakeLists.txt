cmake_minimum_required (VERSION 3.6)

project(Tutorial31_GpuDriver1 CXX)

set(SOURCE
    src/Tutorial31_GpuDriver1.cpp
    src/MeshSTL.cpp
)

set(INCLUDE
    src/Tutorial31_GpuDriver1.hpp
    src/MeshSTL.h
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial31_GpuDriver1" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial31_GpuDriver1 PRIVATE Diligent-TextureLoader Diligent-AssetLoader)