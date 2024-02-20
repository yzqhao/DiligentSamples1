cmake_minimum_required (VERSION 3.6)

project(Tutorial33_RSM CXX)

set(SOURCE
    src/Tutorial33_RSM.cpp
)

set(INCLUDE
    src/Tutorial33_RSM.hpp
)

set(SHADERS
    assets/rsm.hlsl
    assets/light.hlsl
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial33_RSM" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial33_RSM PRIVATE Diligent-TextureLoader Diligent-AssetLoader)