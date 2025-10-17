# Milk-Converter Test Suite

## Overview

This directory contains the regression test suite for the Milkdrop Converter, which validates that the converter produces correct GLSL output and that changes to the codebase don't break existing functionality.

The tests are designed to be robust, fast, and comprehensive—focusing on the core translation logic while avoiding dependencies on external rendering pipelines.

## Test Types

### 1. Per-Pixel Logic Regression (`regression_baked.py`)
- **Purpose**: Validates the core per-pixel translation pipeline
- **Fixture**: `baked.milk` – A complex preset with audio-reactive expressions, custom functions, and state variables
- **Method**: Extracts per-pixel GLSL and compares against a golden reference
- **Run Command**:
  ```bash
  ctest --test-dir build -R baked_per_pixel_regression -V
  ```
- **What it validates**:
  - Critical preset-specific calculations (bass/mid/treb audio band usage)
  - Q/T state variable assignments and flow from per-frame to per-pixel
  - Per-pixel variable rewrites (red/green/blue/alpha → pixelColor.xyz)
  - Mathematical expression accuracy

### 2. Waveform Mode Regression (`regression_wave_modes.py`)
- **Purpose**: Ensures all classic MilkDrop wave mode renderers generate safe, bounded GLSL
- **Fixtures**: `wave_mode_*.milk`, `wave_mode_*_dense.milk` – Minimal and stress variants for each supported wave mode (0,2,3,4,5,6,7,8)
- **Method**: Verifies mode-specific helper functions, loop caps, and distance safety guards are emitted
- **Run Command**:
  ```bash
  python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/
  ```
- **What it validates**:
  - `WaveModeRenderer` strategy classes generate the correct helper GLSL
  - Vertex helper functions are present per rendering mode
  - Loop iteration caps, safe distance helpers, and early-exit heuristics remain intact
  - `wave_quality` uniform and call signatures propagate quality hints correctly

### 3. Raymarch Spec & Fallback Regression (`regression_shader_spec.py`)
- **Purpose**: Guards the full generated shader against RaymarchVibe syntax/semantic drift and ensures unsupported wave modes trigger the no-op fallback
- **Fixtures**: `acid.milk`, `eos.milk`, dense wave presets, plus `unsupported_wave_mode.milk`
- **Method**: Scans converted shaders for required preamble/uniforms, balanced braces, and fallback markers
- **Run Command**:
  ```bash
  python3 tests/regression_shader_spec.py \
      --converter build/MilkdropConverter \
      --fixtures tests/presets \
      --baseline baked.milk \
      --spec-presets acid.milk eos.milk wave_mode_0_dense.milk wave_mode_6_dense.milk wave_mode_8_stress.milk \
      --fallback-preset unsupported_wave_mode.milk
  ```
- **What it validates**:
  - Presence of RaymarchVibe-required declarations (`#version 330 core`, `FragColor`, standard uniforms)
  - Balanced brace structure and absence of deprecated `gl_FragColor`
  - Fallback waveform renderer engages when a preset selects an unsupported wave mode or exceeds safe complexity

## Test Fixtures

### Presets (`tests/presets/`)
- **baked.milk**: Complex regression fixture for per-pixel logic (stored at project root)
- **wave_mode_*.milk**: Minimal wave mode smoketests for supported modes 0, 2, 3, 4, 5, 6, 7, 8
- **wave_mode_*_dense.milk**: Higher-complexity fixtures that stress iteration caps and safe-distance helpers
- **unsupported_wave_mode.milk**: Triggers the fallback waveform renderer for shader-spec validation

### Golden References (`tests/golden/`)
- **baked_per_pixel.glsl**: Expected per-pixel GLSL output from baked.milk
- *Waveform and spec tests perform structural validation and do not rely on additional golden files*

## Running Tests

### Prerequisites
1. Build the converter:
   ```bash
   cd Milk-Converter
   cmake -S . -B build
   cmake --build build
   ```
2. Ensure Python 3.8+ is available for the regression scripts

### Complete Test Run
```bash
cd Milk-Converter

# Run CTest regression (baked.milk per-pixel)
ctest --test-dir build -R baked_per_pixel_regression -V

# Run waveform mode regression
python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/

# Run Raymarch spec & fallback regression
python3 tests/regression_shader_spec.py \
    --converter build/MilkdropConverter \
    --fixtures tests/presets \
    --baseline baked.milk \
    --spec-presets acid.milk eos.milk wave_mode_0_dense.milk wave_mode_6_dense.milk wave_mode_8_stress.milk \
    --fallback-preset unsupported_wave_mode.milk
```

### Individual Test Execution
```bash
# Run only baked regression
python3 tests/regression_baked.py --converter build/MilkdropConverter --preset baked.milk --golden tests/golden/baked_per_pixel.glsl

# Run only waveform regression
python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/

# Run only shader spec regression
python3 tests/regression_shader_spec.py --converter build/MilkdropConverter --fixtures tests/presets --baseline baked.milk
```

### Manual Performance Baseline (60 FPS Target)
1. **Hardware**: Validate on a 1920×1080 display with a mid-tier GPU (GTX 1650 / RX 6500 XT class) or better. Integrated graphics should still sustain ≥45 fps at `u_wave_quality = 0.5`.
2. **Build**: Compile a Release build of the converter and regenerate shaders for the target presets.
3. **RaymarchVibe Validation**:
   - Load the converted shader in RaymarchVibe.
   - Set `u_wave_quality` to `1.0` and confirm scene FPS stays at or above 60.
   - Reduce `u_wave_quality` to `0.5` on lower-end hardware and confirm visual stability (no strobing or runaway loops).
4. **Fallback Safety Check**:
   - Convert `unsupported_wave_mode.milk` and confirm the rendered shader produces no waveform output (fallback returning 0.0) while the UI remains responsive.
5. **Driver Compatibility**: Toggle between AMD/NVIDIA (or Intel) drivers when available, ensuring no shader compilation warnings or validation layer errors occur in RaymarchVibe logs.

## Test Failure Diagnosis

### Per-Pixel Regression Failures
- **Missing Lines**: Check that `IMPORTANT_LINES` are present in output
- **Diff Output**: Review the unified diff to see what changed
- **Glitches**: Parser or GLSLGenerator changes may introduce regressions

### Waveform Regression Failures
- **Missing Function**: A mode-specific helper function wasn't generated
- **Missing Comment**: The mode identification comment is absent
- **Uniform Issues**: Direct uniform access in helpers (indicates parameterization problem)
- **Safety Drift**: Loop cap pattern or safe distance helper references disappeared

### Shader Spec Regression Failures
- **Missing Preamble**: Ensure the shader still starts with `#version 330 core` and declares `FragColor`
- **Brace Imbalance**: Search for unbalanced braces or partial edits in emitted helper code
- **Fallback Missing**: Unsupported wave modes must output the fallback `draw_wave()` implementation

## Adding New Tests

### For New Wave Modes
1. Create `wave_mode_X.milk` with appropriate `nWaveMode` setting
2. Add an entry to `WAVE_FIXTURES` in `regression_wave_modes.py`
3. Specify mode comment, helper expectations, and loop cap patterns
4. Test with both minimal and stress fixtures if the mode exposes quality/iteration heuristics

### For New Translation Features
1. Create appropriate `.milk` test fixture(s)
2. Generate golden reference GLSL if byte-for-byte diffing is required
3. Add a Python regression script or extend an existing one
4. Register the script with CTest for automated coverage

## Continuous Integration

The test suite is designed for CI/CD integration:

- All tests are self-contained and don't require OpenGL contexts
- Failures are reported with clear diagnostic information
- Tests can run in parallel when CTest jobs are available
- Exit codes indicate test success/failure for automation

## Future Test Enhancements

- **Performance Regression**: Time-based tests for conversion speed
- **Memory Leak Detection**: Valgrind integration for memory safety
- **Additional Presets**: Expand coverage beyond baked.milk with diverse fixtures
- **GLSL Validation**: Compile generated shaders to catch syntax errors early
- **Integration Tests**: End-to-end tests loading shaders into RaymarchVibe
