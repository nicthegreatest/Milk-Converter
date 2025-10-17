"""Regression tests for WaveModeRenderer generated GLSL.

Each test preset is minimal and focuses on a specific built-in wave mode.
The script ensures that the converter emits the expected mode-specific helper
functions and annotations in the generated GLSL.
"""

from __future__ import annotations

import argparse
import subprocess
import tempfile
from pathlib import Path

WAVE_FIXTURES = {
    "wave_mode_0.milk": {
        "mode_comment": "// Mode 0: Spectrum circle bars",
        "functions": ["wave_mode0_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
    "wave_mode_2.milk": {
        "mode_comment": "// Mode 2: Centered dots with trails",
        "functions": ["wave_mode2_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
    "wave_mode_3.milk": {
        "mode_comment": "// Mode 3: Volume-modulated centered dots",
        "functions": ["wave_mode3_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, iAudioBands.z, wave_quality)",
    },
    "wave_mode_4.milk": {
        "mode_comment": "// Mode 4: Derivative line (scripted horizontal display)",
        "functions": ["wave_mode_line_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
    "wave_mode_5.milk": {
        "mode_comment": "// Mode 5: Explosive hash radial pattern",
        "functions": ["wave_mode5_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
    "wave_mode_6.milk": {
        "mode_comment": "// Mode 6: Angle-adjustable line spectrum",
        "functions": ["wave_mode6_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
    "wave_mode_7.milk": {
        "mode_comment": "// Mode 7: Double spectrum lines",
        "functions": ["wave_mode7_vertex"],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
    },
}


def run_converter(converter: Path, preset: Path, output: Path) -> None:
    result = subprocess.run(
        [str(converter), str(preset), str(output)],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        text=True,
        check=False,
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"MilkdropConverter failed for {preset}\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )


def validate_fixture(converter: Path, fixtures_dir: Path, preset_name: str) -> None:
    preset_path = fixtures_dir / preset_name
    if not preset_path.exists():
        raise RuntimeError(f"Preset fixture missing: {preset_path}")

    with tempfile.TemporaryDirectory() as tmp:
        output_path = Path(tmp) / "wave.frag"
        run_converter(converter, preset_path, output_path)
        fragment = output_path.read_text()

    if "uniform float u_wave_quality" not in fragment:
        raise AssertionError("Wave quality uniform not present in generated GLSL")

    expectations = WAVE_FIXTURES[preset_name]
    comment = expectations["mode_comment"]
    if comment not in fragment:
        raise AssertionError(f"Expected comment '{comment}' not found in output")

    for function_name in expectations["functions"]:
        if function_name not in fragment:
            raise AssertionError(
                f"Expected helper '{function_name}' not found in GLSL for {preset_name}"
            )
    
    call_pattern = expectations["call"]
    if call_pattern not in fragment:
        raise AssertionError(
            f"Expected call pattern '{call_pattern}' not found in GLSL for {preset_name}"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description="Wave mode regression tests")
    parser.add_argument("--converter", required=True, type=Path, help="Path to MilkdropConverter binary")
    parser.add_argument("--fixtures", required=True, type=Path, help="Directory containing wave mode presets")
    args = parser.parse_args()

    for preset_name in sorted(WAVE_FIXTURES):
        validate_fixture(args.converter, args.fixtures, preset_name)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
