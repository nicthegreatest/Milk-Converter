#!/usr/bin/env python3
"""RaymarchVibe shader specification regression tests.

This script ensures that generated shaders obey the baseline RaymarchVibe
contract (preamble, uniforms, balanced braces) and that unsupported wave
modes fall back to the no-op waveform renderer when complexity exceeds our
safe presets.
"""

from __future__ import annotations

import argparse
import subprocess
import tempfile
from pathlib import Path

RAYMARCH_SPEC_SNIPPETS = [
    "#version 330 core",
    "out vec4 FragColor;",
    "float float_from_bool",
    "uniform float iTime;",
    "uniform vec2 iResolution;",
    "uniform vec4 iAudioBands;",
    "uniform sampler2D iChannel0;",
    "void main()",
    "FragColor = vec4(",
]


class ShaderSpecError(AssertionError):
    """Domain specific assertion error for clearer failure output."""


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


def collect_shader(converter: Path, preset: Path) -> str:
    with tempfile.TemporaryDirectory() as tmp:
        output_path = Path(tmp) / "shader.frag"
        run_converter(converter, preset, output_path)
        return output_path.read_text()


def assert_raymarch_spec(fragment: str, preset_name: str) -> None:
    stripped = fragment.lstrip()
    if not stripped.startswith("#version 330 core"):
        raise ShaderSpecError(f"{preset_name}: shader must start with #version 330 core")

    for snippet in RAYMARCH_SPEC_SNIPPETS:
        if snippet not in fragment:
            raise ShaderSpecError(f"{preset_name}: missing required snippet '{snippet}'")

    if "gl_FragColor" in fragment:
        raise ShaderSpecError(f"{preset_name}: legacy gl_FragColor usage detected")

    if fragment.count("{") != fragment.count("}"):
        raise ShaderSpecError(f"{preset_name}: mismatched braces detected")

    if "layout(" in fragment:
        raise ShaderSpecError(f"{preset_name}: unexpected layout qualifier emitted")


def assert_waveform_fallback(fragment: str, preset_name: str) -> None:
    marker = "// Fallback waveform renderer when the mode is unsupported"
    if marker not in fragment:
        raise ShaderSpecError(f"{preset_name}: fallback waveform renderer not emitted")

    fallback_section = fragment.split(marker, 1)[1]
    if "return 0.0;" not in fallback_section:
        raise ShaderSpecError(f"{preset_name}: fallback renderer must return 0.0")

    if "wave_mode" in fallback_section:
        raise ShaderSpecError(f"{preset_name}: fallback renderer should not include mode helpers")


def resolve_preset_path(base: Path, candidate: Path) -> Path:
    if candidate.is_absolute():
        return candidate
    if candidate.exists():
        return candidate
    return base / candidate


def main() -> int:
    parser = argparse.ArgumentParser(description="RaymarchVibe shader spec regression")
    parser.add_argument("--converter", required=True, type=Path, help="Path to MilkdropConverter binary")
    parser.add_argument("--fixtures", required=True, type=Path, help="Preset directory for fixture lookups")
    parser.add_argument("--spec-presets", nargs="+", default=["acid.milk", "eos.milk"], help="Preset names to lint against the spec")
    parser.add_argument("--baseline", type=Path, help="Optional additional preset outside the fixtures directory (e.g. baked.milk)")
    parser.add_argument("--fallback-preset", default="unsupported_wave_mode.milk", help="Preset name or path that should trigger waveform fallback")
    args = parser.parse_args()

    if not args.converter.exists():
        raise SystemExit(f"Converter binary not found: {args.converter}")
    if not args.fixtures.is_dir():
        raise SystemExit(f"Fixture directory not found: {args.fixtures}")

    presets_to_check: list[Path] = []
    for preset_name in args.spec_presets:
        preset_path = resolve_preset_path(args.fixtures, Path(preset_name))
        if not preset_path.exists():
            raise SystemExit(f"Spec preset not found: {preset_path}")
        presets_to_check.append(preset_path)

    if args.baseline is not None:
        baseline_path = resolve_preset_path(args.fixtures, args.baseline)
        if not baseline_path.exists():
            raise SystemExit(f"Baseline preset not found: {baseline_path}")
        presets_to_check.append(baseline_path)

    fallback_path = resolve_preset_path(args.fixtures, Path(args.fallback_preset))
    if not fallback_path.exists():
        raise SystemExit(f"Fallback preset not found: {fallback_path}")

    for preset_path in presets_to_check:
        fragment = collect_shader(args.converter, preset_path)
        assert_raymarch_spec(fragment, preset_path.name)

    fallback_fragment = collect_shader(args.converter, fallback_path)
    assert_raymarch_spec(fallback_fragment, fallback_path.name)
    assert_waveform_fallback(fallback_fragment, fallback_path.name)

    return 0


if __name__ == "__main__":
    raise SystemExit(main())
