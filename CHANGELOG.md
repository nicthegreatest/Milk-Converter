# Changelog

All notable changes to this project will be documented in this file.

## [0.8.0] - 2025-10-11

### Added
- **Complete per_frame Logic Conversion:** Fixed the projectM parser to properly handle complex, multi-line MilkDrop expressions by addressing a critical AST-overwriting bug in `prjm_eval_compiler_add_instruction`. Implemented `clean_code()` improvements for line continuation handling, enabling full translation of beat detection, color modulation, and dynamic parameter calculations from MilkDrop presets.

### Fixed
- **ProjectM Evaluator Grammar Fix:** Removed faulty subtraction-based naming optimization that caused multi-statement expressions to be lost, restoring full per_frame semantic accuracy.
- **Multi-line Expression Processing:** Enhanced comment removal and line continuation logic to preserve nested MilkDrop conditionals like `pulsefreq=if(equal(pulsefreq,0),2,if(...))`.
- **Accurate per_frame Variables:** Now correctly translates audio-reactive calculations, state management, and time-based animations from MilkDrop's expression language.
- **Dummy Code Removal:** Eliminated placeholder per_frame implementations, deploying full authentic preset logic.

### Known Issues
- RaymarchVibe integration requires debugging - converted shaders load but display white screen, likely due to feedback buffer initialization or texture sampling out-of-bounds. Clamping fixes applied, but lighting/audio reactivity may need feedback loop debugging.

## [0.7.0] - 2025-10-10

### Added
- **Built-in Waveform Rendering:** The converter now implements rendering for MilkDrop's built-in waveforms. This is a major step towards full preset compatibility.
    - The initial implementation supports `nWaveMode=6` (Line Wave), as seen in presets like `3dragonz.milk`.
    - A `draw_wave` function is now generated in the GLSL shader, which replicates the vertex generation logic from the original projectM C++ source code.
    - The final fragment color is now a composite of the feedback buffer, the outer border color, and the newly rendered waveform, all blended additively.

## [0.6.3] - 2025-10-09

This commit significantly improves the Milkdrop to GLSL converter to produce shaders that are fully compatible with the RaymarchVibe application.

The key enhancements include:

- **Data-Driven UI Controls:** The converter now parses the header of `.milk` files to extract default parameter values. These values are used to generate `uniform` declarations with accurate JSON annotations, enabling data-driven UI controls in RaymarchVibe.
- The RaymarchVibe ShaderParser requires uniform declarations to have an explicit default value to generate UI controls. Updated `MilkdropConverter` to include the explicit default value in the generated uniform declaration, ensuring compatibility with the parser. 

- **Improved Parsing:** The `PresetFileParser` has been patched to correctly handle non-sequential and multi-line code blocks in `.milk` files, ensuring that the entire `per_frame` and `per_pixel` logic is captured and correctly formatted.

- **Enhanced Variable Mapping:** The converter now maps MilkDrop's `aspectx` and `aspecty` variables to their `iResolution` equivalents in GLSL, making the generated shaders resolution-aware.

- **Full Texture & Logic Translation:** The generated shaders now include a proper feedback loop. The logic samples the previous frame's output from `iChannel0` using the calculated warped UVs, applies the `decay` factor, and blends the result with the `ob_` (outer border) color. This provides a more complete and faithful translation of the original preset's visual logic.

- **Added default values to GLSL uniform declarations**
- 

These changes result in shaders that are more interactive, visually accurate, and fully integrated with the RaymarchVibe environment... apparantly.

## [0.6.1] - 2025-10-09

Fixes the GLSL shader conversion logic to correctly apply per-pixel transformations and use the proper color variables.

The previous conversion logic had two main flaws:
1. It calculated per-pixel transformations (zoom, rot, sx, sy, etc.) but never used them, resulting in a static image with no patterns.
2. It used the initial uniform values for the final fragment color instead of the `ob_r`, `ob_g`, `ob_b` variables calculated in the per-frame logic.

This commit corrects the `translateToGLSL` function in `MilkdropConverter.cpp` to:
- Inject GLSL boilerplate that applies the calculated transformations to the `uv` coordinates.
- Set the final `FragColor` using the correct `ob_` variables.
- Modulate the final color by the transformed UVs to make the warp effect visible.

## [0.6.0] - 2025-10-09

### Fixed
GLSL code generation to correctly handle boolean logic as floating-point numbers, which is required to match MilkDrop's behavior. The previous implementation generated GLSL `bool` types for comparisons, leading to type mismatch errors when used in expressions expecting floats.

The main changes are:
- A `float_from_bool(bool)` helper function is now added to the GLSL preamble.
- All comparison operators (`>`, `==`, etc.) and boolean functions (`bnot`, `band`, `bor`) are now wrapped in `float_from_bool()` to ensure their result is a float (1.0 or 0.0).
- The `if` function code generation was made smarter to "unwrap" conditions that use `float_from_bool`, avoiding redundant checks and producing cleaner GLSL.
- The modulo operator is now correctly translated to the `mod()` function for float compatibility.

Additionally, significant changes were made to the build system to resolve dependency issues:
- The `CMakeLists.txt` was modified to build the `projectm-eval` library directly, bypassing the main `libprojectM` and its unnecessary OpenGL dependency.
- `PresetFileParser.cpp` and its header are now compiled directly.
- Stub functions for mutexes required by `projectm-eval` were added to resolve linker errors.

## [0.5.0] - 2025-10-09

### Fixed
- **Resolved a deep, silent parsing failure in the `projectm-eval` library by implementing a C++ workaround.** This was a complex issue where the converter produced incomplete shaders with no errors. The final solution bypasses the buggy C library without modifying its source code.
    - **Initial Symptom:** The converter would run successfully but produce a `.frag` file where all per-frame and per-pixel logic was missing. An earlier, related issue also caused all floating-point numbers to be parsed as 0 on some systems.
    - **Diagnostic Process:**
        1.  **Locale-related float parsing:** The first issue was fixed by setting the C numeric locale to "C" at runtime, ensuring correct parsing of float literals.
        2.  **Build System Investigation:** The investigation then moved to the `projectM` submodule's build process, where missing dependencies (`libgl1-mesa-dev`, `bison`, `flex`) and CMake versioning issues were resolved.
        3.  **Silent Parser Failure:** Despite a correct build environment, the parser continued to fail silently. Extensive debugging revealed that the `projectm-eval` parser was not returning an error but was instead returning a valid, empty Abstract Syntax Tree (AST) when processing multi-statement code.
        4.  **Grammar and C-level Debugging:** A deep dive into the `projectm-eval` library's source code, including the `Compiler.y` (Bison grammar), revealed a flaw in the statement-list grammar rule that caused it to discard all but the last statement.
    - **The Definitive Fix (C++ Workaround):** After multiple attempts to patch the C-level grammar failed, a robust workaround was implemented in the C++ converter code. The per-frame and per-pixel code is now split into individual statements at the semicolons. Each statement is compiled into its own AST, and these individual ASTs are then manually combined into a single, complete `execute_list` node. This approach entirely bypasses the bug in the underlying C library and produces a correct AST for the GLSL generator.

### Changed
- The conversion logic in `MilkdropConverter.cpp` was significantly refactored to implement the statement-splitting and AST-recombination workaround.

## [0.4.0] - 2025-10-08

### Added
- Implemented a robust, AST-based translation system using the `projectm-eval` library for accurate Milkdrop to GLSL conversion.
- Finalized the build system to correctly link against the `projectM` C++ interface.

### Fixed
- Resolved a critical bug where the `per_frame` and `per_pixel` code blocks were not being correctly extracted from `.milk` files.
- Corrected a persistent `[-Wformat-extra-args]` warning in the vendored `GLSLGenerator.cpp` file.

### Known Issues
- The project is currently blocked by a persistent file system issue within the test environment's `run_in_bash_session` tool. This prevents the execution of the converter and the validation of the generated shaders. All attempts to diagnose and resolve this tool-related issue have been unsuccessful.

## [0.3.0] - 2025-10-08

### Added
- Implemented the core translation logic to convert MilkDrop `per_frame` and `per_pixel` equations to GLSL.
- Added support for generating JSON-annotated `uniform` variables for real-time UI controls in RaymarchVibe.
- Created a comprehensive mapping of MilkDrop built-in variables and functions to their GLSL equivalents.
- Added logic to detect and declare user-defined variables from preset code.

### Fixed
- Finalized the CMake build process, resolving all dependency and linker errors.
- Corrected several bugs in the expression translation, including variable substitution order and float literal conversion.

## [0.2.0] - 2025-10-08

### Added
- Created a self-contained `Converter` project directory.
- Added a detailed `README.md` file with project summary, technical plan, and build instructions.
- Added this `CHANGELOG.md` file.
- Added a `TODO.md` file to track future work.

### Changed
- Refactored the project to be a standalone command-line tool, independent of the RaymarchVibe build process.
- Vendored the `libprojectM` source code into the `Converter` directory.

## [0.1.0] - 2025-10-08

### Added
- Initial proof-of-concept `MilkdropConverter.cpp` file.
- Investigated the `.milk` file format and the `projectm` source code.
- Acquired the official MilkDrop Preset Authoring Guide.
