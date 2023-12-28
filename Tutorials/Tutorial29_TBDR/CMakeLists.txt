cmake_minimum_required (VERSION 3.6)

project(Tutorial29_TBDR CXX)

set(SOURCE
    src/Mesh.cpp
    src/Tutorial29_TBDR.cpp
)

set(INCLUDE
    src/Mesh.h
    src/Tutorial29_TBDR.hpp
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial29_TBDR" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial29_TBDR PRIVATE Diligent-TextureLoader Diligent-AssetLoader)