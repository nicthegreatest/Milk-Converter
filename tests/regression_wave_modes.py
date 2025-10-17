"""Regression tests for WaveModeRenderer generated GLSL.

This harness verifies that every wave mode emits its hardened helper
functions, loop caps, and safety guards. It also exercises additional
fixtures that stress higher-complexity presets so we keep coverage on the
boundaries that should trigger the safety heuristics.
"""

from __future__ import annotations

import argparse
import copy
import re
import subprocess
import tempfile
from pathlib import Path

WAVE_MODE_EXPECTATIONS = {
    0: {
        "mode_comment": "// Mode 0: Spectrum circle bars",
        "functions": ["wave_mode0_vertex"],
        "max_constant": "MODE0_MAX_WAVE_ITERATIONS",
        "helpers": ["wave_distance_to_segment("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = min(raw_samples, MODE0_MAX_WAVE_ITERATIONS + 1);",
            "int segment_count = max(sample_count - 1, 1);",
        ],
    },
    2: {
        "mode_comment": "// Mode 2: Centered dots with trails",
        "functions": ["wave_mode2_vertex"],
        "max_constant": "MODE2_MAX_WAVE_ITERATIONS",
        "helpers": ["wave_safe_distance("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = max(min(samples, MODE2_MAX_WAVE_ITERATIONS), 1);",
        ],
    },
    3: {
        "mode_comment": "// Mode 3: Volume-modulated centered dots",
        "functions": ["wave_mode3_vertex"],
        "max_constant": "MODE3_MAX_WAVE_ITERATIONS",
        "helpers": ["wave_safe_distance("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, iAudioBands.z, wave_quality)",
        "cap_patterns": [
            "int sample_count = max(min(samples, MODE3_MAX_WAVE_ITERATIONS), 1);",
        ],
    },
    4: {
        "mode_comment": "// Mode 4: Derivative line (scripted horizontal display)",
        "functions": ["wave_mode_line_vertex"],
        "max_constant": "MODE4_MAX_WAVE_ITERATIONS",
        "helpers": ["clip_waveform_edges(", "wave_distance_to_segment("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = min(raw_samples, MODE4_MAX_WAVE_ITERATIONS + 1);",
            "int segment_count = max(sample_count - 1, 1);",
        ],
    },
    5: {
        "mode_comment": "// Mode 5: Explosive hash radial pattern",
        "functions": ["wave_mode5_vertex"],
        "max_constant": "MODE5_MAX_WAVE_ITERATIONS",
        "helpers": ["wave_safe_distance("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = max(min(raw_samples, MODE5_MAX_WAVE_ITERATIONS), 1);",
        ],
    },
    6: {
        "mode_comment": "// Mode 6: Angle-adjustable line spectrum",
        "functions": ["wave_mode6_vertex"],
        "max_constant": "MODE6_MAX_WAVE_ITERATIONS",
        "helpers": ["clip_waveform_edges(", "wave_distance_to_segment("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = min(raw_samples, MODE6_MAX_WAVE_ITERATIONS + 1);",
            "int segment_count = max(sample_count - 1, 1);",
        ],
    },
    7: {
        "mode_comment": "// Mode 7: Double spectrum lines",
        "functions": ["wave_mode7_vertex"],
        "max_constant": "MODE7_MAX_WAVE_ITERATIONS",
        "helpers": ["clip_waveform_edges(", "wave_distance_to_segment("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = min(raw_samples, MODE7_MAX_WAVE_ITERATIONS + 1);",
            "int segment_count = max(sample_count - 1, 1);",
        ],
    },
    8: {
        "mode_comment": "// Mode 8: Spectrum line (angled analyser)",
        "functions": ["wave_mode8_vertex"],
        "max_constant": "MODE8_MAX_WAVE_ITERATIONS",
        "helpers": ["clip_waveform_edges(", "wave_distance_to_segment("],
        "call": "draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)",
        "cap_patterns": [
            "int sample_count = min(raw_samples, MODE8_MAX_WAVE_ITERATIONS + 1);",
            "int segment_count = max(sample_count - 1, 1);",
        ],
    },
}

WAVE_FIXTURES = {
    "wave_mode_0.milk": {"mode": 0},
    "wave_mode_0_dense.milk": {"mode": 0},
    "wave_mode_2.milk": {"mode": 2},
    "wave_mode_3.milk": {"mode": 3},
    "wave_mode_4.milk": {"mode": 4},
    "wave_mode_5.milk": {"mode": 5},
    "wave_mode_6.milk": {"mode": 6},
    "wave_mode_6_dense.milk": {"mode": 6},
    "wave_mode_7.milk": {"mode": 7},
    "wave_mode_8.milk": {"mode": 8},
    "wave_mode_8_stress.milk": {"mode": 8},
}

COMMON_SNIPPETS = [
    "const float WAVE_EPSILON",
    "const float WAVE_DISTANCE_CLAMP",
    "const int WAVE_MIN_WARMUP_ITERATIONS",
    "wave_contribution(",
    "wave_clamp_audio(",
    "wave_should_exit(",
]


def collapse_whitespace(value: str) -> str:
    return "".join(value.split())


def expect_whitespace_insensitive(fragment: str, snippet: str, preset_name: str, label: str) -> None:
    if collapse_whitespace(snippet) not in collapse_whitespace(fragment):
        raise AssertionError(
            f"Expected {label} '{snippet.strip()}' in GLSL for {preset_name}"
        )


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


def build_expectations(preset_name: str) -> dict[str, object]:
    metadata = WAVE_FIXTURES[preset_name]
    base = copy.deepcopy(WAVE_MODE_EXPECTATIONS[metadata["mode"]])
    for key in ("helpers", "cap_patterns"):
        base.setdefault(key, [])
        extras = metadata.get(f"extra_{key}")
        if extras:
            base[key].extend(extras)
    return base


def validate_common_snippets(fragment: str, preset_name: str) -> None:
    for snippet in COMMON_SNIPPETS:
        if snippet not in fragment:
            raise AssertionError(
                f"Expected shared helper '{snippet}' not found in GLSL for {preset_name}"
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

    expectations = build_expectations(preset_name)

    comment = expectations["mode_comment"]
    if comment not in fragment:
        raise AssertionError(f"Expected comment '{comment}' not found in output")

    for function_name in expectations["functions"]:
        if function_name not in fragment:
            raise AssertionError(
                f"Expected helper '{function_name}' not found in GLSL for {preset_name}"
            )

    call_pattern = expectations["call"]
    expect_whitespace_insensitive(fragment, call_pattern, preset_name, "call pattern")

    validate_common_snippets(fragment, preset_name)

    max_constant = expectations.get("max_constant")
    if max_constant and max_constant not in fragment:
        raise AssertionError(
            f"Expected iteration cap '{max_constant}' not found in GLSL for {preset_name}"
        )

    for helper_snippet in expectations.get("helpers", []):
        if helper_snippet not in fragment:
            raise AssertionError(
                f"Expected helper usage '{helper_snippet}' not found in GLSL for {preset_name}"
            )

    for cap_snippet in expectations.get("cap_patterns", []):
        expect_whitespace_insensitive(fragment, cap_snippet, preset_name, "loop cap pattern")

    if "wave_safe_distance(" not in fragment and "wave_distance_to_segment(" not in fragment:
        raise AssertionError(
            f"Waveform GLSL for {preset_name} is missing safe distance helper calls"
        )

    if not re.search(r"for\s*\(\s*int\s+", fragment):
        raise AssertionError(
            f"Waveform GLSL for {preset_name} does not contain a bounded for-loop"
        )


def main() -> int:
    parser = argparse.ArgumentParser(description="Wave mode regression tests")
    parser.add_argument("--converter", required=True, type=Path, help="Path to MilkdropConverter binary")
    parser.add_argument("--fixtures", required=True, type=Path, help="Directory containing wave mode presets")
    args = parser.parse_args()

    if not args.converter.exists():
        raise SystemExit(f"Converter binary not found: {args.converter}")
    if not args.fixtures.is_dir():
        raise SystemExit(f"Fixture directory not found: {args.fixtures}")

    for preset_name in sorted(WAVE_FIXTURES):
        validate_fixture(args.converter, args.fixtures, preset_name)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
