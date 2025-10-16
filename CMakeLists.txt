cmake_minimum_required(VERSION 3.16)

set(ProjectName "RayTracer")
project(${ProjectName} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# If you have Khronos C++ bindings (cl.hpp / cl2.hpp) vendored:
set(OPENCL_CLHPP_DIR "${CMAKE_SOURCE_DIR}/dependencies")

# Put sources in src/
add_executable(${ProjectName}
    app/main.cpp
    src/compute/OpenCL/CLBackend.cpp
    src/compute/Backend.cpp
    

    # add other .cpp files here, e.g. src/raytracer.cpp src/kernel_runner.cpp
)

target_include_directories(${ProjectName} PRIVATE
    "${OPENCL_CLHPP_DIR}"
    "${CMAKE_SOURCE_DIR}"
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/compute"
    "${CMAKE_SOURCE_DIR}/src/compute/SceneGPU"
    "${CMAKE_SOURCE_DIR}/src/compute/OpenCL"
    "${CMAKE_SOURCE_DIR}/dependencies"
)

# Prefer CMake's FindOpenCL for cross-platform linking
find_package(OpenCL REQUIRED)
target_link_libraries(${ProjectName} PRIVATE OpenCL::OpenCL)

# If FindOpenCL fails on macOS only, uncomment this fallback:
if(APPLE)
  find_library(OPENCL_FRAMEWORK OpenCL)
  target_link_libraries(${ProjectName} PRIVATE "${OPENCL_FRAMEWORK}")
endif()