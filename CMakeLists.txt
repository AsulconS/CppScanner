cmake_minimum_required(VERSION 3.9)
project(PyScannerLibrary VERSION 0.0.1 DESCRIPTION "CMakeLists File for compiling C++ PyScannerLibrary")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY $<1:${CMAKE_SOURCE_DIR}>)

macro(build_cpp_source filename)
    add_executable(${filename} library/src/${filename}.cpp)
    target_include_directories(${filename} PRIVATE library/include)
    target_compile_options(${filename} PRIVATE /W0)
    target_compile_definitions(${filename} PRIVATE __SHARED_LIB)
    target_link_libraries(${filename} PUBLIC image)
endmacro(build_cpp_source)

add_library(image SHARED library/src/image.cpp)
set_target_properties(image PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION 0
    PUBLIC_HEADER library/include/image.hpp
)
target_compile_options(image PRIVATE /W0)
target_compile_definitions(image PRIVATE __SHARED_LIB __STDC_LIB_EXT1__)
target_include_directories(image PRIVATE library/include)

build_cpp_source(homography)
build_cpp_source(sobelFilter)
build_cpp_source(thresholding)
build_cpp_source(cornerDetection)
build_cpp_source(contrastStretching)
