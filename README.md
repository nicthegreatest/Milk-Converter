# Milkdrop to RaymarchVibe Shader Converter

## 1. Project Summary

This document outlines the purpose, design, and build process for the Milkdrop Converter, a command-line utility designed to translate legacy MilkDrop presets (`.milk` files) into modern GLSL fragment shaders compatible with the RaymarchVibe application.

### 1.1. Session Achievements & Investigation

This project was initiated to explore the feasibility of integrating MilkDrop presets into RaymarchVibe. Our investigation involved several key phases:

1.  **Initial Analysis:** We began by examining the `.milk` preset files. We quickly determined they were not direct GLSL shaders but rather `.ini`-style configuration files containing parameters and a custom scripting language.

2.  **Source Code Analysis:** To understand how to interpret these files, we acquired the source code for `projectM`, a modern, cross-platform implementation of the MilkDrop engine. This was a critical step.

3.  **Build Process:** A significant portion of the session was dedicated to successfully compiling `libprojectM` and its dependencies (`projectm-eval`, `Poco`, `SDL2`, etc.) on a Fedora-based Linux environment. This process involved debugging CMake configurations, resolving dependency issues, and correcting source code incompatibilities related to C++ standards and library versions.

4.  **Decompiling the Magic:** By analyzing the `projectm` source code, specifically the `PresetFileParser` and the `projectm-eval` expression evaluation library, we reverse-engineered the exact mechanism by which `.milk` files are rendered:
    *   **Parsing:** Keys and values are read from the `.milk` file.
    *   **Code Blocks:** Special keys like `per_frame_...` and `per_pixel_...` contain snippets of a custom expression language.
    *   **Evaluation:** The `projectm-eval` library parses these expressions, which are similar but not identical to C or GLSL, and compiles them into an intermediate representation.
    *   **Rendering:** The main `libprojectM` library executes this intermediate code, using the results to drive the visuals. It uses a multi-pass rendering pipeline involving warping, custom shapes/waves, and a final compositing pass.

5.  **Acquiring the Authoring Guide:** We located the official MilkDrop Preset Authoring Guide, which provided a definitive reference for all built-in variables (`time`, `bass`, `fps`, `q1-q32`, etc.) and functions (`sin`, `cos`, `abs`, etc.) available in the expression language. This is our Rosetta Stone for the translation process.

### 1.2. Project Organization: A Self-Contained Tool

Based on the investigation, we decided that the most robust and maintainable approach was to create this converter as a **completely separate, self-contained application**. It resides in this `Converter/` directory.

*   **Why Separate?** This avoids polluting the main RaymarchVibe application with the significant complexity and dependencies of the entire `projectm` library. The conversion is a one-time, offline process, so it doesn't need to be part of the real-time visualizer.
*   **Dependencies:** All necessary source code for `libprojectM` is vendored within this directory to ensure it can be built without any reliance on the parent `raymarchvibe` project or system-wide installations.

## 2. Technical Conversion Plan

This tool will function as a translator, converting the logic from a `.milk` preset into a single, functional GLSL fragment shader.

### 2.1. How It Will Work

1.  **Parsing:** The converter will use the `PresetFileParser` class from the included `libprojectM` source to read and parse the input `.milk` file, extracting the key-value pairs and, most importantly, the code for the `per_frame_` and `per_pixel_` equations.

2.  **Expression Translation:** The core of the converter will be a new translation unit that programmatically converts the MilkDrop expression language into valid GLSL syntax.
    *   **Variable Mapping:** Built-in Milkdrop variables will be mapped to RaymarchVibe's standard uniforms (e.g., `time` -> `iTime`, `bass` -> `iAudioBands.x`).
    *   **Function Mapping:** Milkdrop functions (`sin`, `cos`, `pow`, etc.) will be mapped to their direct GLSL equivalents.
    *   **Q & T Variables:** The special `q1-q32` and `t1-t8` variables, used for passing data between different stages in MilkDrop, will be declared as local variables in the GLSL `main()` function. The `per_frame` logic will be translated first to calculate their values, which will then be available for the subsequent translated `per_pixel` logic.

3.  **Shader Generation:** The tool will generate a complete `.frag` file containing:
    *   A `#version 330 core` directive.
    *   Declarations for all necessary `uniform` variables (for `iTime`, `iResolution`, `iAudioBands`, etc.).
    *   The translated GLSL code within the `main()` function.
    *   The final color calculation from the `per_pixel` logic will be assigned to `FragColor`.

4.  **UI Controls (Future Goal):** To make the converted shaders interactive, the converter will eventually add JSON-formatted comments to the generated `uniform` declarations, which RaymarchVibe's UI can parse to create real-time control widgets.

## 3. Build Instructions

This project is self-contained. All dependencies required to build the converter are located in the `vendor` directory.

### 3.1. Prerequisites

Ensure you have the following tools installed on your system:
*   CMake (3.20 or higher)
*   A C++17 compliant compiler (e.g., GCC, Clang)
*   Git
*   Bison
*   Flex

On a Fedora-based system, you can install these with:
```bash
sudo dnf install cmake gcc-c++ git bison flex
```

### 3.2. Building the Converter

The converter is built in two main stages. First, we build the `libprojectM` dependency, then we build the converter executable itself.

**Stage 1: Build `libprojectM`**

1.  **Initialize Submodules:** Navigate to the `vendor/projectm-master` directory and initialize the git submodules. This only needs to be done once.
    ```bash
    cd vendor/projectm-master
    git submodule init
    git submodule update
    cd ../.. 
    ```

2.  **Configure `libprojectM`:** Create a build directory and run CMake. We must disable the system-wide search for the `projectm-eval` library to force it to use the one included in the source tree.
    ```bash
    cmake -S vendor/projectm-master -B vendor/projectm-master/build -DENABLE_SYSTEM_PROJECTM_EVAL=OFF -DCMAKE_CXX_STANDARD=17
    ```

3.  **Compile `libprojectM`:**
    ```bash
    cmake --build vendor/projectm-master/build
    ```

**Stage 2: Build the `MilkdropConverter`**

1.  **Configure the Converter:** Now, configure the main converter project. It will automatically find the `libprojectM` we just built.
    ```bash
    cmake -S . -B build
    ```

2.  **Compile the Converter:**
    ```bash
    cmake --build build
    ```

An executable named `MilkdropConverter` will be created in the `build` directory.

## 4. Usage

Once built, you can run the converter from the `Converter` directory with the following command:

```bash
./build/MilkdropConverter /path/to/input.milk /path/to/output.frag
```
