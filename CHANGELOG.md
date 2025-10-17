# Changelog

All notable changes to this project will be documented in this file.

**Please read carefully before making any changes.**

This file is the single source of truth for the project's changelog. To ensure consistency and maintain a clear historical record, please adhere to the following rules:

1.  **Chronological Order:** All new entries must be added to the `[Unreleased]` section at the top. Do not create new version sections.
2.  **Versioning:** Follow the [Semantic Versioning](https://semver.org/) guidelines.
3.  **Formatting:** Maintain the existing format for all entries (`### Added`, `### Changed`, `### Fixed`, etc.).
4.  **Do Not Create New Files:** Do not create separate changelog files. All changes must be recorded in this file.

By following these instructions, you will help us maintain a clean, accurate, and easily parsable changelog.

## [0.9.1] - 2025-10-18

### Added
- **Shader Spec Regression:** Introduced `tests/regression_shader_spec.py` and a matching CTest target to assert RaymarchVibe preamble requirements, balanced braces, and waveform fallback behaviour for unsupported presets.
- **Waveform Safety Coverage:** Expanded the waveform regression harness with dense fixtures, loop-cap pattern checks, and safe-distance helper assertions across all supported modes.
- **Documentation Updates:** Captured a 60 fps baseline performance plan, updated project docs with the new regression harnesses, and refreshed raymarch instructions to reflect multi-mode support.

### Changed
- **Test Fixtures:** Added high-complexity waveform presets and an unsupported-mode fallback fixture to broaden compatibility coverage.
- **CI Configuration:** CTest now executes the shader spec regression alongside existing baked and waveform suites, tightening release gates around shader safety.

## [0.9.0] - 2025-10-17

### Added
- **Waveform Quality Controls:** Introduced the `wave_quality` UI control/uniform so RaymarchVibe users can trade waveform fidelity for throughput without breaking MilkDrop aesthetics. The converter now emits mode-aware draw call patterns that pipe the quality slider through every `draw_wave` signature.
- **Performance Regression Coverage:** Updated `tests/regression_wave_modes.py` to assert the presence of quality-aware call patterns and uniforms for every supported MilkDrop wave mode.

### Changed
- **Mode Calibration:** Tuned per-mode smoothing, intensity scaling, and sample budgeting to remain visually faithful under loop caps while scaling with `wave_quality`.
- **Fallback Safety:** Added lightweight fallbacks for heavy wave modes when quality is reduced, keeping audio-reactive outlines legible even at aggressive performance caps.
## [0.8.4] - 2025-10-17

### Fixed
- **Waveform Safety Hardening:** Introduced GLSL safety primitives, bounded trig wrappers, and per-mode iteration caps to prevent GPU timeout crashes when rendering built-in waveforms.
- **Regression Coverage:** Extended waveform regression tests to assert iteration caps and helper usage, ensuring future changes retain the safety guardrails.

## [0.8.3] - 2025-10-16

### Fixed
- **Uniform Slider Range Validation:** Fixed issue where preset default values could exceed their UI slider ranges. Converter now dynamically expands slider min/max bounds when defaults fall outside the predefined ranges (e.g., eos.milk zoom=13.29 now gets slider range 0.5-13.29 instead of 0.5-1.5)

## [0.8.2] - 2025-10-16

### Added
- **Complete Per-Pixel Translation:** Implemented full per-pixel expression translation with proper variable rewrites (red/green/blue/alpha → pixelColor components) and coordinate-dependent calculations
- **GLSLGenerator Architecture:** Custom AST walker translates projectm-eval expression trees into GLSL, mapping MilkDrop variables/functions to RaymarchVibe equivalents
- **Per-Pixel State Management:** Q1-Q32 and T1-T8 variables correctly accessible in per-pixel logic, enabling complex inter-frame state management
- **Automated Regression Testing:** Added CTest-driven regression suite with baked.milk test fixture to validate per-pixel translation correctness
- **Shader Standards Documentation:** Created comprehensive single-source-of-truth specification document at `../../../documentation/SHADERS.md` consolidating all shader standards, uniform mappings, and conversion requirements
- **Documentation Consolidation:** Resolved conflicting documentation by establishing clear separation of concerns - SHADERS.md for technical specs, guide files for usage instructions
- **Per-Pixel Test Coverage:** `tests/regression_baked.py` validates generated per-pixel GLSL blocks against golden references, ensuring translation consistency

### Fixed
- **Per-Pixel Expression Translation:** Fixed the root cause of identical shader output across presets by implementing complete per-pixel logic translation (previously was using placeholder code)
- **Variable Scope Handling:** Properly scope per-pixel variables to enable coordinate-dependent transformations and color manipulation
- **RaymarchVibe Rendering:** Feedback buffer handling patterns identified as root cause of white screen issues
- **Converter Standards:** Unified shader generation patterns to match SHADERS.md specification exactly

### Changed
- **Documentation Updates:** README, TODO, and CHANGELOG now accurately reflect completed per-pixel translation implementation
- **Project Status:** Converter is now feature-complete for core MilkDrop preset translation (per-frame + per-pixel logic)

## [0.8.1] - 2025-10-16

### Added
- **Build Fixes:** Resolved projectM dependency and header inclusion issues
- **Test Shader:** Created working "Blackhole with Mirror Effects" shader demonstrating full SHADERS.md compliance

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

These changes result in shaders that are more interactive, visually accurate, and fully integrated with the RaymarchVibe environment... apparantly.

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
