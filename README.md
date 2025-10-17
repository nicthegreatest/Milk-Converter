# Milkdrop to RaymarchVibe Shader Converter

## 1. Project Summary

This document outlines the purpose, design, and build process for the Milkdrop Converter, a command-line utility designed to translate legacy MilkDrop presets (`.milk` files) into modern GLSL fragment shaders compatible with the [RaymarchVibe](https://github.com/nicthegreatest/raymarchvibe) application.

The converter is a **completely separate, self-contained application**. This avoids polluting the main RaymarchVibe application with the significant complexity and dependencies of the `projectm` library. The conversion is a one-time, offline process, so it doesn't need to be part of the real-time visualizer.

## 2. Features

- **Logic Conversion:** Translates both `per_frame` and `per_pixel` logic from MilkDrop presets into RaymarchVibe-compatible GLSL fragment shaders.
- **Variable Mapping:** Supports `q1-q32`, `t1-t8`, audio bands, and all built-in functions.
- **UI Controls Generation:** Produces JSON-annotated uniforms for real-time parameter adjustment in RaymarchVibe.
- **Waveform Rendering:** Supports classic wave modes (0, 2, 3, 4, 5, 6, 7, and 8) with quality-aware tuning. The generated GLSL is hardened with bounded helpers, iteration caps, and early-out safeguards to prevent GPU timeouts.
- **Performance Controls:** A `wave_quality` uniform allows balancing visual fidelity vs. throughput.
- **Compliance:** Generated shaders conform to RaymarchVibe GLSL specifications.

## 3. Build Instructions

This project is self-contained. The `projectM` dependency must be cloned into the `vendor` directory before building.

### 3.1. Prerequisites

Ensure you have the following tools installed on your system:
*   CMake (3.15 or higher)
*   A C++17 compliant compiler (e.g., GCC, Clang)
*   Git
*   Bison
*   Flex

**On a Debian/Ubuntu-based system, you can install these with:**
```bash
sudo apt-get update && sudo apt-get install cmake g++ git bison flex
```

**On a Fedora-based system, you can install these with:**
```bash
sudo dnf install cmake gcc-c++ git bison flex
```

### 3.2. Building the Converter

1.  **Clone the `projectM` Dependency:**
    First, clone the `projectM` repository into the `vendor` directory. This only needs to be done once.
    ```bash
    git clone https://github.com/projectM-visualizer/projectm.git vendor/projectm-master
    ```

2.  **Initialize Submodules:**
    Navigate to the newly cloned repository and initialize its submodules.
    ```bash
    cd vendor/projectm-master
    git submodule update --init --recursive
    cd ../..
    ```

3.  **Configure and Build:**
    From the root of the project directory, run the following commands to create a build directory, configure the project, and compile the executable.
    ```bash
    cmake -S . -B build
    cmake --build build
    ```

An executable named `MilkdropConverter` will be created in the `build` directory.

## 4. Usage

Once built, you can run the converter from the project's root directory with the following command:

```bash
./build/MilkdropConverter /path/to/input.milk /path/to/output.frag
```

## 5. Known Issues & Next Steps

- **Remaining Wave Modes:** Mode 1 and other custom variants are not yet supported.
- **Custom Shapes:** Shape rendering is not yet implemented.
- **Feedback Buffer Handling:** Converted shaders may exhibit minor rendering differences compared to native shaders due to feedback loop initialization patterns.

### Development Priorities
1. Add support for remaining waveform modes.
2. Implement custom shape rendering.
3. Expand regression test coverage with additional preset fixtures.
4. (Future) Integrate HLSL shader translation for warp/comp shaders.

## 6. Regression Testing

The build enables a CTest-driven regression suite to guard against translation regressions.

- **`baked_per_pixel_regression`**: Validates per-pixel logic translation against a golden reference file.
- **`wave_mode_regression`**: Verifies that all supported wave modes generate correct and safe GLSL.
- **`shader_spec_regression`**: Performs a "shaderlint" pass to ensure generated GLSL honors the RaymarchVibe contract and that unsupported presets generate a safe fallback implementation.

To run the full test suite after building:
```bash
ctest --test-dir build -V
```

## 7. Project Structure

```
MilkdropConverter/
├── MilkdropConverter.cpp          # Main converter implementation
├── WaveModeRenderer.cpp           # Waveform GLSL generation logic
├── WaveModeRenderer.hpp           # Header for WaveModeRenderer
├── CMakeLists.txt                 # Build configuration
├── baked.milk                     # Test preset fixture
├── tests/
│   ├── regression_baked.py        # Per-pixel regression test
│   ├── regression_wave_modes.py   # Waveform safety regression harness
│   ├── regression_shader_spec.py  # Raymarch spec and fallback checks
│   ├── presets/                   # Test preset fixtures (minimal, dense, fallback)
│   └── golden/
│       └── baked_per_pixel.glsl   # Golden reference for per-pixel translation
└── vendor/
    └── projectm-master/           # Vendored projectM dependency
        ├── vendor/projectm-eval/  # Expression parser and AST generator
        └── src/libprojectM/       # PresetFileParser
```

For detailed information on shader standards and diagnostics, please refer to the [RaymarchVibe SHADERS.md documentation](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md).