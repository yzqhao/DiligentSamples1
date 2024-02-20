cmake_minimum_required (VERSION 3.6)

project(Tutorial30_VirtualTexture CXX)

set(SOURCE
    src/Mesh.cpp
    src/Tutorial30_VirtualTexture.cpp
)

set(INCLUDE
    src/Mesh.h
    src/Tutorial30_VirtualTexture.hpp
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial30_VirtualTexture" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial30_VirtualTexture PRIVATE Diligent-TextureLoader)