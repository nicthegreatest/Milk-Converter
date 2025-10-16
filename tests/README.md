# Milk-Converter Test Suite

## Overview

This directory contains the regression test suite for the Milkdrop Converter, which validates that the converter produces correct GLSL output and that changes to the codebase don't break existing functionality.

The tests are designed to be robust, fast, and comprehensive - focusing on the core translation logic while avoiding dependencies on external rendering pipelines.

## Test Types

### 1. Per-Pixel Logic Regression (`regression_baked.py`)
- **Purpose**: Validates the core per-pixel translation pipeline
- **Fixture**: `baked.milk` - A complex preset with audio-reactive expressions, custom functions, and state variables
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
- **Purpose**: Ensures all classic MilkDrop wave mode renderers generate correct GLSL
- **Fixtures**: `wave_mode_*.milk` - Minimal presets for each wave mode (0,2,3,4,5,6,7,8)
- **Method**: Verifies mode-specific GLSL helper functions are present in output
- **Run Command**:
  ```bash
  python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/
  ```
- **What it validates**:
  - WaveModeRenderer strategy class generates correct mode-specific code
  - Vertex helper functions for each rendering mode
  - Uniform parameterization (no direct uniform access in helper functions)
  - Shader compilation readiness

## Test Fixtures

### Presets (`tests/presets/`)
- **baked.milk**: Complex regression fixture for per-pixel logic
- **wave_mode_*.milk**: Minimal wave mode test fixtures

### Golden References (`tests/golden/`)
- **baked_per_pixel.glsl**: Expected per-pixel GLSL output from baked.milk
- *Note: Waveform tests use preset-side verification instead of golden files*

## Running Tests

### Prerequisites
1. Build the converter:
```bash
cd Milk-Converter
cmake -S . -B build
cmake --build build
```

2. Ensure Python 3.6+ is available for regression scripts

### Complete Test Run
```bash
cd Milk-Converter
# Run CTest regression (baked.milk per-pixel)
ctest --test-dir build -R baked_per_pixel_regression -V

# Run waveform mode regression
python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/
```

### Individual Test Execution
```bash
# Run only baked regression
python3 tests/regression_baked.py --converter build/MilkdropConverter --preset built.milk --golden tests/golden/baked_per_pixel.glsl

# Run only waveform regression
python3 tests/regression_wave_modes.py --converter build/MilkdropConverter --fixtures tests/presets/
```

## Test Failure Diagnosis

### Per-Pixel Regression Failures
- **Missing Lines**: Check that `IMPORTANT_LINES` are present in output
- **Diff Output**: Review the unified diff to see what changed
- **Glitches**: Parser or GLSLGenerator changes may introduce regressions

### Waveform Regression Failures
- **Missing Function**: A mode-specific helper function wasn't generated
- **Missing Comment**: The mode identification comment is absent
- **Uniform Issues**: Direct uniform access in helpers (indicates parameterization problem)

## Adding New Tests

### For New Wave Modes
1. Create `wave_mode_X.milk` with appropriate nWaveMode setting
2. Add entry to `WAVE_FIXTURES` in `regression_wave_modes.py`
3. Specify mode comment and required functions
4. Test with minimal preset parameters

### For New Translation Features
1. Create appropriate `.milk` test fixture
2. Generate golden reference GLSL
3. Add Python regression script
4. Integrate with CTest if automated

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
