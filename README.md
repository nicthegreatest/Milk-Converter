# Milkdrop to RaymarchVibe Shader Converter

## 1. Project Summary

This document outlines the purpose, design, and build process for the Milkdrop Converter, a command-line utility designed to translate legacy MilkDrop presets (`.milk` files) into modern GLSL fragment shaders compatible with the [RaymarchVibe](https://github.com/nicthegreatest/raymarchvibe) application.

### 1.1. Session Achievements & Investigation

This project was initiated to explore the feasibility of integrating MilkDrop presets into RaymarchVibe. Our investigation involved several key phases:

1.  **Initial Analysis:** We began by examining the `.milk` preset files. We quickly determined they were not direct GLSL shaders but rather `.ini`-style configuration files containing parameters and a custom scripting language.

2.  **Source Code Analysis:** To understand how to interpret these files, we acquired the source code for `projectM`, a modern, cross-platform implementation of the MilkDrop engine. This was a critical step.

3.  **Build Process:** A significant portion of the session was dedicated to successfully compiling `libprojectM` and its dependencies on a Linux environment. This process involved debugging CMake configurations, resolving dependency issues, and correcting source code incompatibilities.

4.  **Decompiling the Magic:** By analyzing the `projectm` source code, specifically the `PresetFileParser` and the `projectm-eval` expression evaluation library, we reverse-engineered the exact mechanism by which `.milk` files are rendered:
    *   **Parsing:** Keys and values are read from the `.milk` file.
    *   **Code Blocks:** Special keys like `per_frame_...` and `per_pixel_...` contain snippets of a custom expression language.
    *   **Evaluation:** The `projectm-eval` library parses these expressions and compiles them into an intermediate representation.
    *   **Rendering:** The main `libprojectM` library executes this intermediate code, using the results to drive the visuals.

5.  **Acquiring the Authoring Guide:** We located the official MilkDrop Preset Authoring Guide, which provided a definitive reference for all built-in variables and functions available in the expression language. This is our Rosetta Stone for the translation process.

### 1.2. Project Organization: A Self-Contained Tool

The converter is a **completely separate, self-contained application**. This avoids polluting the main RaymarchVibe application with the significant complexity and dependencies of the `projectm` library. The conversion is a one-time, offline process, so it doesn't need to be part of the real-time visualizer.

## 2. Technical Conversion Plan

This tool functions as a translator, converting the logic from a `.milk` preset into a single, functional GLSL fragment shader through a multi-stage pipeline.

### 2.1. How It Works

1.  **Parsing:** The converter uses the `PresetFileParser` class from the included `libprojectM` source to read and parse the input `.milk` file into key-value pairs and code blocks.

2.  **Expression Compilation:** Per-frame and per-pixel code blocks are compiled using the `projectm-eval` library, which parses MilkDrop expressions and generates Abstract Syntax Trees (ASTs) representing the expression logic.

3.  **GLSL Translation:** The custom `GLSLGenerator` class traverses the AST nodes and generates GLSL code:
    *   **Variable Mapping:** Built-in MilkDrop variables are mapped to RaymarchVibe's standard uniforms (e.g., `time` → `iTime`, `bass` → `iAudioBands.x`, `x` → `uv.x`, `y` → `uv.y`).
    *   **Function Mapping:** MilkDrop functions (`sin`, `sqr`, `if`, etc.) are mapped to their GLSL equivalents or helper functions.
    *   **Operator Translation:** Expression operators (`+`, `-`, `*`, `/`, `==`, `>`, etc.) are translated to corresponding GLSL operators.
    *   **Per-Pixel Variable Rewrites:** Per-pixel color outputs (`red`, `green`, `blue`, `alpha`) are automatically rewritten to GLSL vec4 component access (`pixelColor.r`, `pixelColor.g`, etc.).
    *   **Q & T State Variables:** The special `q1-q32` and `t1-t8` variables are declared as local variables in the GLSL `main()` function. The `per_frame` logic is translated first to calculate their values, which are then available for the `per_pixel` logic, enabling complex inter-frame state management.

4.  **Shader Generation:** The tool generates a complete `.frag` file containing:
    *   `#version 330 core` directive
    *   Helper functions for MilkDrop-specific operations (rand, sigmoid, boolean operators)
    *   Standard RaymarchVibe uniforms (iTime, iResolution, iAudioBands, iChannel0-3)
    *   Preset-specific uniforms with JSON-annotated UI controls
    *   A `main()` function with translated per-frame and per-pixel logic
    *   Coordinate transformation and feedback buffer composition logic

5.  **UI Controls:** To make the converted shaders interactive, the converter adds JSON-formatted comments to the generated `uniform` declarations. RaymarchVibe's UI can parse these annotations to create real-time control widgets for the preset's parameters (zoom, rot, decay, colors, etc.).

6.  **Waveform Integration:** For presets with waveforms enabled, the converter generates a `draw_wave()` function that replicates the waveform rendering logic across built-in modes 0, 2, 3, 4, 5, 6, 7, and 8. Version 0.8.4 hardens these shaders with EPSILON guards, bounded trigonometric wrappers, capped iteration counts (≤64), and early-out logic to prevent GPU driver timeouts on long-running waveform loops.

## 3. Build Instructions

This project is self-contained. The `projectM` dependency must be cloned into the `vendor` directory before building.

### 3.1. Prerequisites

Ensure you have the following tools installed on your system:
*   CMake (3.15 or higher)
*   A C++17 compliant compiler (e.g., GCC, Clang)
*   Git
*   Bison
*   Flex
*   OpenGL Development Libraries (required by projectM dependency)

**On a Debian/Ubuntu-based system, you can install these with:**
```bash
sudo apt-get update && sudo apt-get install cmake g++ git bison flex libgl1-mesa-dev
```

**On a Fedora-based system, you can install these with:**
```bash
sudo dnf install cmake gcc-c++ git bison flex mesa-libGL-devel
```

### 3.2. Building the Converter

The project uses a unified build process. The main CMake configuration will automatically configure and build the `libprojectM` dependency before building the converter itself.

1.  **Clone the `projectM` Dependency:**
    First, clone the `projectM` repository into the `vendor` directory. This only needs to be done once.
    ```bash
    git clone https://github.com/projectM-visualizer/projectm.git vendor/projectm-master
    ```

2.  **Initialize Submodules:**
    Navigate to the newly cloned repository and initialize its submodules.
    ```bash
    cd vendor/projectm-master
    git submodule init
    git submodule update
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

## 5. Project Status

### Completed Features (v0.8.2)
- ✅ **Complete per_frame Logic Conversion:** Converts complex MilkDrop expressions including beat detection, color modulation, and audio-reactive calculations
- ✅ **Complete per_pixel Logic Conversion:** Full implementation of per-pixel expression translation with proper variable mapping (red/green/blue/alpha → pixelColor components)
- ✅ **Multi-line Expression Support:** Handles nested conditionals like `pulsefreq=if(equal(pulsefreq,0),2,if(pulse,...))`
- ✅ **Full Variable Mapping:** Supports q1-q32, t1-t8, audio bands, and all built-in functions
- ✅ **UI Controls Generation:** Produces JSON-annotated uniforms for real-time parameter adjustment
- ✅ **Waveform Rendering:** Built-in modes 0, 2, 3, 4, 5, 6, 7, and 8 now emit hardened waveform GLSL with bounded trig helpers, ≤64 iteration caps, and early-out safeguards.
- ✅ **Build System:** Standalone CMake-based build with vendored dependencies
- ✅ **Regression Testing:** Automated CTest suite validates per-pixel translation correctness
- ✅ **RaymarchVibe GLSL Compliance:** Generated shaders conform to [RaymarchVibe GLSL specifications](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md)

### Current Status
The converter successfully translates both per-frame and per-pixel logic from MilkDrop presets into RaymarchVibe-compatible GLSL fragment shaders:

- ✅ **Per-frame Logic:** Complete translation of beat detection, color modulation, and audio-reactive calculations
- ✅ **Per-pixel Logic:** Full implementation including coordinate-dependent calculations, color manipulation, and state variable access
- ✅ **Shader Generation:** Produces grammatically correct GLSL with full semantic accuracy
- ✅ **RaymarchVibe Integration:** UI controls parse correctly; shaders compile without errors
- ✅ **Technical Documentation:** External [SHADERS.md](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md) provides single source of truth for conversion standards and diagnostics

### Architecture Highlights
- **GLSLGenerator Class:** Custom AST walker translates projectm-eval expression trees into GLSL, mapping MilkDrop variables/functions to RaymarchVibe equivalents
- **Per-Pixel Variable Rewrites:** Automatic translation of per-pixel color outputs (red/green/blue/alpha) to proper GLSL vec4 component access
- **State Flow:** Q1-Q32 and T1-T8 variables correctly flow from per-frame to per-pixel logic, enabling complex inter-frame state management
- **Audio Integration:** Full mapping of MilkDrop audio bands (bass/mid/treb) to iAudioBands uniform components

### Known Issues & Next Steps
- **Feedback Buffer Handling:** Converted shaders may exhibit rendering differences compared to manually-written shaders due to feedback loop initialization patterns (see SHADERS.md diagnostics)
- **Remaining Wave Modes:** Modes 1 and experimental/custom waveform variants are still pending translation; the supported set (0, 2, 3, 4, 5, 6, 7, 8) now ships with capped loops and safety guards.
- **Custom Shapes:** Shape rendering not yet implemented

### Development Priorities
1. Add support for remaining waveform modes (1, 9+, and other custom variants)
2. Implement custom shape rendering
3. Expand regression test coverage with additional preset fixtures
4. (Future Enhancement) Integrate HLSL shader translation for warp/comp shaders

## 6. Regression Testing

The build enables a CTest-driven regression suite to guard against translation regressions and ensure consistent output across changes to the GLSLGenerator and parser logic.

### 6.1 baked.milk per-pixel regression

The `baked.milk` preset serves as our primary test fixture for validating per-pixel logic translation. This preset was chosen because it exercises:
- Multiple audio-reactive variables (bass, mid, treb)
- Complex mathematical expressions with trigonometric functions
- User-defined variables that interact with q/t state
- Per-pixel warp calculations and color manipulation

After configuring and building the project, run the baked preset regression:

```bash
cmake -S . -B build
cmake --build build
ctest --test-dir build -R baked_per_pixel_regression -V
```

**What the test validates:**
1. The converter successfully processes `baked.milk` without errors
2. The generated per-pixel GLSL block exactly matches the golden reference in `tests/golden/baked_per_pixel.glsl`
3. Critical preset-specific expressions are present:
   - `warp = 1.42;` - Per-pixel warp override
   - `q3 = (iAudioBands.z * bom);` - Treble-reactive state variable
   - `q8 = (iAudioBands.x * boom);` - Bass-reactive state variable
   - `q5 = (iAudioBands.y * rox);` - Mid-reactive state variable

The `tests/regression_baked.py` script:
- Runs the converter against `baked.milk`
- Extracts lines between `// Per-pixel logic` and `// Apply coordinate transformations` markers
- Compares generated output to the golden reference
- On failure, prints missing essential expressions or a unified diff for easy diagnosis

This regression test locks down the per-pixel translation behavior, ensuring that parser fixes or GLSLGenerator improvements don't accidentally break working preset conversions.

### 6.2 Multi-waveform mode regression testing

With the addition of full waveform mode support, new regression tests validate all classic MilkDrop wave modes. Test fixtures for modes 0,2,3,4,5,6,7,8 ensure proper GLSL generation for each rendering style.

**Wave Mode Test Coverage:**
- **Mode 0**: CircularBarsRenderer (spectrum circle bars)
- **Mode 2**: CenteredSpiroRenderer (centered dots)
- **Mode 3**: CenteredSpiroVolumeRenderer (volume-modulated dots)
- **Mode 4**: DerivativeLineRenderer (angled line displays)
- **Mode 5**: ExplosiveHashRenderer (radial patterns)
- **Mode 6**: LineWaveRenderer (angle-adjustable spectrum lines) ⭐
- **Mode 7**: DoubleLineWaveRenderer (dual spectrum lines)
- **Mode 8**: SpectrumLineRenderer (spectrum analyzer lines)

Run the waveform regression tests:

```bash
python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/
```

**Test Details:**
- Each wave mode preset sets a specific `nWaveMode` value and minimal parameters (fixtures cover modes 0, 2, 3, 4, 5, 6, 7, and 8)
- The test script verifies that the appropriate mode-specific GLSL helpers are generated and that each shader declares the expected `MODE*_MAX_WAVE_ITERATIONS` cap.
- Confirms that the hardened waveform helpers (`wave_clamp_audio`, `wave_contribution`, `wave_should_exit`) are emitted so draw loops respect the new safety bounds.
### 7.1. Project Structure

```
MilkdropConverter/
├── MilkdropConverter.cpp          # Main converter implementation
├── CMakeLists.txt                 # Build configuration
├── baked.milk                     # Test preset fixture
├── tests/
│   ├── regression_baked.py        # Automated regression test script
│   └── golden/
│       └── baked_per_pixel.glsl   # Golden reference for per-pixel translation
├── vendor/
│   └── projectm-master/           # Vendored projectM dependency
│       ├── vendor/projectm-eval/  # Expression parser and AST generator
│       └── src/libprojectM/       # PresetFileParser
└── documentation/
    ├── README.md                  # This file
    ├── TODO.md                    # Task tracking
    ├── CHANGELOG.md               # Version history
    ├── raymarch_instructions.md   # Conversion rules reference
    └── milk-authoring-guide.md    # MilkDrop language reference
```

### 7.2. Key Components

**GLSLGenerator Class** (`MilkdropConverter.cpp`)
- Traverses projectm-eval AST nodes recursively
- Maps MilkDrop operators/functions to GLSL equivalents
- Handles variable rewrites for per-pixel context (red/green/blue → pixelColor.r/g/b)
- Generates GLSL expressions with proper type conversions and parenthesization

**Variable Mapping Tables** (`milkToGLSLVars`, `uniformControls`)
- `milkToGLSLVars`: Maps MilkDrop built-in variables to RaymarchVibe uniforms
- `uniformControls`: Defines UI widget metadata for interactive parameters
- `perPixelVariableRewrites()`: Handles context-specific variable translations

**Expression Compilation Pipeline**
1. `PresetFileParser` extracts per_frame/per_pixel code blocks from .milk files
2. `projectm_eval_context_create()` initializes the expression compiler
3. `projectm_eval_code_compile()` parses code and builds AST
4. `GLSLGenerator::generate()` walks AST and emits GLSL code

**Testing Infrastructure**
- CTest integration for automated regression testing
- `regression_baked.py` validates per-pixel translation against golden references
- Checks for critical preset-specific expressions to catch regressions

### 7.3. Adding Support for New Features

**Adding a new wave mode:**
1. Study the corresponding C++ implementation in `projectM/src/libprojectM/MilkdropPreset/Waveforms/`
2. Translate the vertex generation logic into a GLSL function
3. Update `generateWaveformGLSL()` to handle the new mode
4. Add test coverage with a preset using that wave mode

**Adding a new MilkDrop function:**
1. Add the function pointer mapping in `GLSLGenerator` constructor
2. Implement GLSL equivalent in shader preamble if needed (e.g., `sigmoid_eel`)
3. Update `getFunctionName()` to handle the new function
4. Test with a preset that uses the function

**Extending variable mappings:**
1. Update `milkToGLSLVars` for read-only built-in variables
2. Update `uniformControls` for writable parameters that need UI widgets
3. Update `perPixelVariableRewrites()` for context-specific translations
4. Document in `raymarch_instructions.md`

## 8. Shader Documentation for Future Coding Agents
For detailed information on shader standards and diagnostics, please refer to the [SHADERS.md documentation](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md).
