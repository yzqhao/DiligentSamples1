cmake_minimum_required (VERSION 3.6)

project(Tutorial27_SSR CXX)

set(SOURCE
    src/Tutorial27_SSR.cpp
)

set(INCLUDE
    src/Tutorial27_SSR.hpp
)

set(SHADERS
    assets/BRDFIntegration.hlsl
    assets/panoToCube.hlsl
)

set(ASSETS
    #assets/LA_Helipad.png
)

add_sample_app("Tutorial27_SSR" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial27_SSR PRIVATE Diligent-TextureLoader Diligent-AssetLoader)