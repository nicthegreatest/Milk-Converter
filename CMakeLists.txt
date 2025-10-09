cmake_minimum_required(VERSION 3.15)
project(MilkdropConverter VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(ENABLE_SYSTEM_PROJECTM_EVAL OFF CACHE BOOL "Use vendored projectm-eval")
set(ENABLE_CXX_INTERFACE ON CACHE BOOL "Enable C++ interface for libprojectM")
add_subdirectory(vendor/projectm-master)

add_executable(MilkdropConverter
  MilkdropConverter.cpp
)

target_include_directories(MilkdropConverter PRIVATE
  ${CMAKE_BINARY_DIR}/vendor/projectm-master/src/api/include
  vendor/projectm-master/src/libprojectM
  vendor/projectm-master/src/api/include
)

target_link_libraries(MilkdropConverter PRIVATE
  libprojectM::projectM
)