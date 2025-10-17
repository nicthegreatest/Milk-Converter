# Changelog

All notable changes to this project will be documented in this file.

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
