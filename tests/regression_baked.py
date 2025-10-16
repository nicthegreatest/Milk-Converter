#!/usr/bin/env python3
"""Regression test runner for baked.milk per-pixel translation.

This script runs the MilkdropConverter binary against the baked.milk preset
and compares the generated per-pixel GLSL block to a golden expectation.
"""

from __future__ import annotations

import argparse
import difflib
import subprocess
import sys
import tempfile
from pathlib import Path

IMPORTANT_LINES = [
    "warp = 1.42;",
    "q3 = (iAudioBands.z * bom);",
    "q8 = (iAudioBands.x * boom);",
    "q5 = (iAudioBands.y * rox);",
]


def extract_per_pixel_lines(fragment_source: str) -> list[str]:
    """Return canonicalised per-pixel lines from the generated fragment source."""

    try:
        start = fragment_source.index("// Per-pixel logic")
    except ValueError as exc:  # pragma: no cover - defensive clause
        raise RuntimeError("Could not locate '// Per-pixel logic' marker in output") from exc

    try:
        end = fragment_source.index("// Apply coordinate transformations", start)
    except ValueError as exc:  # pragma: no cover - defensive clause
        raise RuntimeError(
            "Could not locate '// Apply coordinate transformations' marker in output"
        ) from exc

    block = fragment_source[start:end]
    lines = [
        line.strip()
        for line in block.splitlines()
        if line.strip() and not line.strip().startswith("//")
    ]
    return lines


def run_converter(converter: Path, preset: Path, output: Path) -> None:
    """Invoke the converter and ensure it produces the requested output file."""

    result = subprocess.run(
        [str(converter), str(preset), str(output)],
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
    )
    if result.returncode != 0:
        raise RuntimeError(
            "MilkdropConverter exited with a non-zero status\n"
            f"stdout:\n{result.stdout}\n"
            f"stderr:\n{result.stderr}"
        )


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description="Run baked.milk regression test")
    parser.add_argument("--converter", type=Path, required=True, help="Path to MilkdropConverter executable")
    parser.add_argument("--preset", type=Path, required=True, help="Path to baked.milk preset")
    parser.add_argument("--golden", type=Path, required=True, help="Golden per-pixel GLSL fragment")

    args = parser.parse_args(argv)

    for path in (args.converter, args.preset, args.golden):
        if not path.exists():
            raise SystemExit(f"Required path does not exist: {path}")

    with tempfile.TemporaryDirectory() as tmp:
        output_path = Path(tmp) / "baked.frag"
        run_converter(args.converter, args.preset, output_path)
        fragment_source = output_path.read_text()

    generated_lines = extract_per_pixel_lines(fragment_source)
    golden_lines = [
        line.strip()
        for line in args.golden.read_text().splitlines()
        if line.strip()
    ]

    missing_essentials = [line for line in IMPORTANT_LINES if line not in generated_lines]
    if missing_essentials:
        print("Missing expected per-pixel expressions:")
        for line in missing_essentials:
            print(f"  - {line}")
        return 1

    if generated_lines != golden_lines:
        print("Per-pixel GLSL block drift detected (generated vs. golden):")
        diff = difflib.unified_diff(
            golden_lines,
            generated_lines,
            fromfile="golden",
            tofile="generated",
            lineterm="",
        )
        for line in diff:
            print(line)
        return 1

    return 0


if __name__ == "__main__":
    sys.exit(main())
