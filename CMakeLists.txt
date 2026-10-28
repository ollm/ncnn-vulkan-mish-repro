cmake_minimum_required(VERSION 3.16)
project(ncnn_vulkan_mish_repro LANGUAGES CXX)

set(NCNN_VULKAN ON CACHE BOOL "" FORCE)
set(NCNN_BUILD_BENCHMARK OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(NCNN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NCNN_INSTALL_SDK OFF CACHE BOOL "" FORCE)

add_subdirectory(ncnn EXCLUDE_FROM_ALL)

add_executable(ncnn-vulkan-mish-repro main.cpp)
target_link_libraries(ncnn-vulkan-mish-repro PRIVATE ncnn)
target_compile_features(ncnn-vulkan-mish-repro PRIVATE cxx_std_11)
