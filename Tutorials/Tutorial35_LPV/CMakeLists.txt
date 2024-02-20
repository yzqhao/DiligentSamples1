cmake_minimum_required (VERSION 3.6)

project(Tutorial35_LPV CXX)

set(SOURCE
    src/Tutorial35_LPV.cpp
)

set(INCLUDE
    src/Tutorial35_LPV.hpp
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

add_sample_app("Tutorial35_LPV" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial35_LPV PRIVATE Diligent-TextureLoader Diligent-AssetLoader)