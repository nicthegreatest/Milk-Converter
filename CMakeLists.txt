cmake_minimum_required(VERSION 3.15)
project(MilkdropConverter VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# projectm-eval requires bison and flex
find_package(BISON)
find_package(FLEX)

# Add the projectm-eval subdirectory directly, bypassing the main projectM build
add_subdirectory(vendor/projectm-master/vendor/projectm-eval)

add_executable(MilkdropConverter
  MilkdropConverter.cpp
  # Manually add the preset parser files to the build
  vendor/projectm-master/src/libprojectM/PresetFileParser.cpp
)

# Add the include directory for PresetFileParser.hpp
target_include_directories(MilkdropConverter PRIVATE
  vendor/projectm-master/src/libprojectM
)

# Link against the static projectm-eval library.
# This will also automatically handle include directories.
target_link_libraries(MilkdropConverter PRIVATE
  projectM_eval
)