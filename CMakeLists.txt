cmake_minimum_required(VERSION 3.20)
project(cpp_raytracer LANGUAGES CXX)

add_executable(raytracer main.cpp)
target_compile_features(raytracer PRIVATE cxx_std_20)
target_include_directories(raytracer PRIVATE raytracer raytracer-geom raytracer-reader)

include(CTest)
if(BUILD_TESTING)
  foreach(mode IN ITEMS full depth normals)
    add_test(NAME render_${mode} COMMAND raytracer
      "${CMAKE_CURRENT_SOURCE_DIR}/examples/demo.obj"
      "${CMAKE_CURRENT_BINARY_DIR}/demo-${mode}.ppm"
      ${mode}
    )
  endforeach()
endif()
