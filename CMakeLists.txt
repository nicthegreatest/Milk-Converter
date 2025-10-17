cmake_minimum_required(VERSION 3.15)
project(MilkdropConverter VERSION 1.0.0 LANGUAGES C CXX)

include(CTest)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# projectm-eval requires bison and flex
find_package(BISON)
find_package(FLEX)

# Add the projectm-eval subdirectory directly, bypassing the main projectM build
set(MILKDROP_BUILD_TESTING_SAVED ${BUILD_TESTING})
set(BUILD_TESTING OFF)
add_subdirectory(vendor/projectm-master/vendor/projectm-eval)
set(BUILD_TESTING ${MILKDROP_BUILD_TESTING_SAVED})
unset(MILKDROP_BUILD_TESTING_SAVED)

add_executable(MilkdropConverter
  MilkdropConverter.cpp
  WaveModeRenderer.cpp
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

if(BUILD_TESTING)
  find_package(Python3 COMPONENTS Interpreter REQUIRED)
  add_test(
    NAME baked_per_pixel_regression
    COMMAND Python3::Interpreter
      ${CMAKE_SOURCE_DIR}/tests/regression_baked.py
      --converter $<TARGET_FILE:MilkdropConverter>
      --preset ${CMAKE_SOURCE_DIR}/baked.milk
      --golden ${CMAKE_SOURCE_DIR}/tests/golden/baked_per_pixel.glsl
  )

  add_test(
    NAME wave_mode_regression
    COMMAND Python3::Interpreter
      ${CMAKE_SOURCE_DIR}/tests/regression_wave_modes.py
      --converter $<TARGET_FILE:MilkdropConverter>
      --fixtures ${CMAKE_SOURCE_DIR}/tests/presets
  )

  add_test(
    NAME shader_spec_regression
    COMMAND Python3::Interpreter
      ${CMAKE_SOURCE_DIR}/tests/regression_shader_spec.py
      --converter $<TARGET_FILE:MilkdropConverter>
      --fixtures ${CMAKE_SOURCE_DIR}/tests/presets
      --baseline ${CMAKE_SOURCE_DIR}/baked.milk
      --spec-presets acid.milk eos.milk wave_mode_0_dense.milk wave_mode_6_dense.milk wave_mode_8_stress.milk
      --fallback-preset unsupported_wave_mode.milk
  )
endif()
