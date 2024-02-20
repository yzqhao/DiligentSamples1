cmake_minimum_required (VERSION 3.6)

project(Tutorial36_VXGI CXX)

set(SOURCE
    src/Scene.cpp
    src/Tutorial36_VXGI.cpp
)

set(INCLUDE
    src/Scene.h
    src/Tutorial36_VXGI.hpp
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

add_sample_app("Tutorial36_VXGI" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial36_VXGI PRIVATE Diligent-TextureLoader Diligent-AssetLoader)