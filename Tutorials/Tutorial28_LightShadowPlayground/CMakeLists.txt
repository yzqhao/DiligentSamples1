cmake_minimum_required (VERSION 3.6)

project(Tutorial28_LightShadowPlayground CXX)

set(SOURCE
    src/Geometry.cpp
    src/Tutorial28_LightShadowPlayground.cpp
)

set(INCLUDE
    src/Geometry.h
    src/RingBuffer.h
    src/SDFConstant.h
    src/ShaderDef.h
    src/Tutorial28_LightShadowPlayground.hpp
)

set(SHADERS
    assets/batchCompaction.hlsl
    assets/clearVisibilityBuffers.hlsl
    assets/cull_resources.h.hlsl
    assets/Packing.h.hlsl
    assets/Shader_Defs.h.hlsl
    assets/triangleFiltering.hlsl
    assets/vb_resources.h.hlsl
    assets/visibilityBufferPass_vert.hlsl
    assets/visibilityBufferPass_frag.hlsl
)

set(ASSETS
    #assets/DGLogo.png
)

add_sample_app("Tutorial28_LightShadowPlayground" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial28_LightShadowPlayground PRIVATE Diligent-TextureLoader Diligent-AssetLoader)

if(MSVC)
    target_compile_definitions(Tutorial28_LightShadowPlayground PRIVATE _CRT_SECURE_NO_WARNINGS)
endif()