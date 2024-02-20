cmake_minimum_required (VERSION 3.6)

project(Tutorial32_VirtualGeomtry CXX)

set(SOURCE
    src/Tutorial32_VirtualGeomtry.cpp
    src/MeshSTL.cpp
    src/Bounds.cpp
    src/Partitioner.cpp
    src/Cluster.cpp
    src/VirtualMesh.cpp

    src/MeshSimplify/BitArray.cpp
    src/MeshSimplify/BitArray.h
    src/MeshSimplify/HashTable.cpp
    src/MeshSimplify/HashTable.h
    src/MeshSimplify/Heap.cpp
    src/MeshSimplify/Heap.h
    src/MeshSimplify/MeshSimplify.cpp
    src/MeshSimplify/MeshSimplify.h
)

set(INCLUDE
    src/Tutorial32_VirtualGeomtry.hpp
    src/MeshUtil.h
    src/metis.h
    src/span.h
    src/MeshSTL.h
    src/Bounds.h
    src/Partitioner.h
    src/Cluster.h
    src/VirtualMesh.h
)

set(SHADERS
    #assets/cube.vsh
    #assets/cube.psh
)

if (MSVC)
    add_compile_options(
        /W4
        /permissive-
        /WX-
    )
endif()

add_sample_app("Tutorial32_VirtualGeomtry" "DiligentSamples/Tutorials" "${SOURCE}" "${INCLUDE}" "${SHADERS}" "${ASSETS}")
target_link_libraries(Tutorial32_VirtualGeomtry PRIVATE Diligent-TextureLoader)

if(PLATFORM_WIN32)
    target_link_directories(Tutorial32_VirtualGeomtry INTERFACE ${CMAKE_SOURCE_DIR})
    target_link_libraries(Tutorial32_VirtualGeomtry PRIVATE metis.lib)
endif(PLATFORM_WIN32)

