# TODO

## Completed (v0.8.0)
- [x] Finalize the build process for the `MilkdropConverter`.
- [x] Implement the core translation logic in `MilkdropConverter.cpp`.
- [x] Create a comprehensive mapping from Milkdrop built-in variables to GLSL uniforms.
- [x] Implement a robust system for translating Milkdrop expression syntax to GLSL.
- [x] Handle the `q` and `t` variables correctly, ensuring data flow from per-frame to per-pixel logic.
- [x] Add support for generating UI controls (JSON annotations) for the converted shaders.
- [x] Implement rendering for built-in waveforms (`nWaveMode=6`).
- [x] Fix projectM-eval parser to handle complex multi-line expressions
- [x] Implement complete per_frame translation with audio reactivity and beat detection
- [x] Remove dummy placeholder code from converter

## Current Status
- [x] **Per-frame Logic:** Successfully converts complete MilkDrop beat detection, color modulation, and audio-reactive calculations
- [x] **Shader Generation:** Produces grammatically correct GLSL with full semantic accuracy
- [x] **RaymarchVibe Integration:** UI controls parse correctly; shaders compile without errors

## Remaining Tasks (v0.9.0+)
- [ ] Complete final verification of converted shader loading in RaymarchVibe
- [ ] Add support for more built-in wave modes (0-5, 7+)
- [ ] Add support for custom shapes
- [ ] (Stretch Goal) Investigate and implement translation for `warp` and `comp` HLSL shaders
- [ ] (Stretch Goal) Pass full audio waveform data to the shader via a texture for more accurate rendering

## Regression Coverage
- [x] baked.milk per-pixel translation is locked down via `ctest -R baked_per_pixel_regression`
- [ ] Expand the regression suite with additional preset fixtures leveraging `tests/regression_baked.py`

## Known Issues
- Feedback buffer handling patterns may differ between manually written shaders and converted output
- Use SHADERS.md specification to diagnose and correct conversion inconsistencies
