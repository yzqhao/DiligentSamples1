cmake_minimum_required (VERSION 3.6)

project(Tutorial31_GpuDriver2 CXX)

set(SOURCE
    src/Tutorial31_GpuDriver2.cpp
    src/MeshSTL.cpp
    src/Geometry.cpp
)

set(INCLUDE
    src/Tutorial31_GpuDriver2.hpp
    src/ShaderDef.h
    src/MeshSTL.h
    src/Geometry.h
)

set(SHADERS
    assets/model.vsh
    assets/model.psh
    assets/modelSimple.vsh
    assets/modelSimple.psh
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial31_GpuDriver2" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial31_GpuDriver2 PRIVATE Diligent-TextureLoader Diligent-AssetLoader)