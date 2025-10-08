cmake_minimum_required(VERSION 3.15)
project(MilkdropConverter LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_subdirectory(vendor/projectm-master)

add_executable(MilkdropConverter
  MilkdropConverter.cpp
)

target_include_directories(MilkdropConverter PRIVATE
  vendor/projectm-master/src/libprojectM
  vendor/projectm-master/src/api/include
)

target_link_libraries(MilkdropConverter PRIVATE
  libprojectM::projectM
)
