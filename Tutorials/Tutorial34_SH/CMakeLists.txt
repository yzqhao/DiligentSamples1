cmake_minimum_required (VERSION 3.6)

project(Tutorial34_SH CXX)

set(SOURCE
    src/Tutorial34_SH.cpp
)

set(INCLUDE
    src/Tutorial34_SH.hpp
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

add_sample_app("Tutorial34_SH" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial34_SH PRIVATE Diligent-TextureLoader Diligent-AssetLoader)