# TODO

## Completed (v0.8.2)
- [x] Finalize the build process for the `MilkdropConverter`.
- [x] Implement the core translation logic in `MilkdropConverter.cpp`.
- [x] Create a comprehensive mapping from Milkdrop built-in variables to GLSL uniforms.
- [x] Implement a robust system for translating Milkdrop expression syntax to GLSL.
- [x] Handle the `q` and `t` variables correctly, ensuring data flow from per-frame to per-pixel logic.
- [x] Add support for generating UI controls (JSON annotations) for the converted shaders.
- [x] Implement rendering for built-in waveforms (`nWaveMode=6`).
- [x] Fix projectM-eval parser to handle complex multi-line expressions
- [x] Implement complete per_frame translation with audio reactivity and beat detection
- [x] **Implement complete per_pixel translation** with proper variable rewrites and coordinate-dependent calculations
- [x] **Add per-pixel variable mapping** (red/green/blue/alpha → pixelColor components)
- [x] **Create GLSLGenerator class** with AST traversal and MilkDrop→GLSL function/operator mapping
- [x] **Implement per-pixel state management** enabling q/t variable access in per-pixel expressions
- [x] **Add automated regression testing** for per-pixel translation (baked.milk test fixture)
- [x] Remove dummy placeholder code from converter
- [x] Document RaymarchVibe GLSL compliance and SHADERS.md reference

## Current Status (v0.8.2)
The converter is now feature-complete for core MilkDrop preset translation:

- ✅ **Per-frame Logic:** Successfully converts complete MilkDrop beat detection, color modulation, and audio-reactive calculations
- ✅ **Per-pixel Logic:** Full translation of coordinate-dependent expressions, color manipulation, and state variable access
- ✅ **Shader Generation:** Produces grammatically correct GLSL with full semantic accuracy
- ✅ **RaymarchVibe Integration:** Generated shaders compile without errors and conform to GLSL specifications
- ✅ **Regression Coverage:** Automated CTest suite validates translation correctness

## Remaining Tasks (v0.9.0+)
- [x] **feat(waveform): add full spectrum MilkDrop wave mode rendering with strategy class** ✅ COMPLETED
- [x] **fix(waveform): resolve uniform declaration ordering issues in all wave modes** ✅ COMPLETED
- [x] **feat(architecture): implement mode-aware callPattern() for all wave renderers** ✅ COMPLETED
- [x] **build(ci): achieve successful compilation and linking** ✅ COMPLETED
- [ ] Add support for custom shapes
- [ ] Expand regression test suite with additional preset fixtures
- [ ] Investigate and address feedback buffer handling differences (if patterns emerge from user testing)
- [ ] (Stretch Goal) Investigate and implement translation for `warp` and `comp` HLSL shaders
- [ ] (Stretch Goal) Pass full audio waveform data via texture for enhanced rendering

## Regression Coverage
- [x] baked.milk per-pixel translation is locked down via `ctest -R baked_per_pixel_regression`
  - Validates per-pixel expression translation accuracy
  - Ensures critical preset-specific calculations are preserved
  - Guards against parser and GLSLGenerator regressions
- [ ] Expand the regression suite with additional preset fixtures leveraging `tests/regression_baked.py`
  - Add coverage for different per-frame/per-pixel patterns
  - Test edge cases (deeply nested conditionals, complex math)
  - Validate additional wave modes as they're implemented

## Known Issues & Considerations
- **Feedback Buffer Patterns:** Converted shaders may exhibit rendering differences compared to manually-written shaders due to feedback loop initialization patterns. Use [SHADERS.md](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md) diagnostics to identify and correct conversion inconsistencies.
- **Wave Mode Coverage:** Currently only nWaveMode=6 (Line Wave) is implemented. Additional modes require translating corresponding C++ rendering logic from projectM.
- **Custom Shapes:** Shape rendering not yet implemented; presets relying on custom shapes will not display these elements.
