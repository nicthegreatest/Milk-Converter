# Milk-Converter Instructions: Converting MilkDrop to RaymarchVibe GLSL

**This document provides technical reference for Milk-Converter's conversion logic and variable mappings.**

For complete RaymarchVibe shader specifications, see [SHADERS.md](https://github.com/nicthegreatest/raymarchvibe/blob/main/documentation/SHADERS.md).

## Variable Mappings

### Per-Frame and Per-Pixel Built-in Variables

MilkDrop variables map to RaymarchVibe uniforms as follows:

| MilkDrop | RaymarchVibe | Context | Notes |
|---|---|---|---|
| `time` | `iTime` | Both | Elapsed time in seconds |
| `fps` | `iFps` | Both | Frames per second |
| `frame` | `iFrame` | Both | Frame counter |
| `bass` | `iAudioBands.x` | Both | Low frequency band |
| `mid` | `iAudioBands.y` | Both | Mid frequency band |
| `treb` | `iAudioBands.z` | Both | High frequency band |
| `bass_att` | `iAudioBandsAtt.x` | Both | Smoothed bass |
| `mid_att` | `iAudioBandsAtt.y` | Both | Smoothed mid |
| `treb_att` | `iAudioBandsAtt.z` | Both | Smoothed treble |
| `x` | `uv.x` | Per-pixel | Horizontal coordinate [0,1] |
| `y` | `uv.y` | Per-pixel | Vertical coordinate [0,1] |
| `rad` | `length(uv - vec2(0.5))` | Per-pixel | Distance from center |
| `ang` | `atan(uv.y - 0.5, uv.x - 0.5)` | Per-pixel | Angle from center |
| `aspectx` | `(iResolution.y / iResolution.x)` | Both | Aspect ratio X |
| `aspecty` | `(iResolution.x / iResolution.y)` | Both | Aspect ratio Y |

### Per-Pixel Color Variables

Per-pixel color variables are automatically rewritten to access components of the `pixelColor` vec4:

| MilkDrop (per-pixel) | GLSL | Notes |
|---|---|---|
| `red` | `pixelColor.r` | Red channel [0,1] |
| `green` | `pixelColor.g` | Green channel [0,1] |
| `blue` | `pixelColor.b` | Blue channel [0,1] |
| `alpha` | `pixelColor.a` | Alpha channel [0,1] |

### State Variables (Q and T)

| Variables | Scope | Purpose |
|---|---|---|
| `q1` through `q32` | Per-frame → Per-pixel | Frame-persistent state that flows to per-pixel logic |
| `t1` through `t8` | Per-frame → Per-pixel | Temporary state for per-pixel calculations |

## UI Control Generation

Converts MilkDrop preset values to RaymarchVibe JSON UI controls:

```glsl
// MilkDrop preset default: zoom=1.0
uniform float u_zoom = 1.0; // {"widget":"slider", "min":0.5, "max":2.0, "step":0.01}
```

## Compilation Requirements

- **GLSL Version:** `#version 330 core`
- **Output:** `out vec4 FragColor;`
- **Input:** `in vec2 uv;`
- **Main:** `void main() { ... FragColor = finalColor; }`

## Conversion Features Implemented (v0.8.2)

1. ✅ **Per-Frame Translation:** Complete translation of beat detection, color modulation, and audio-reactive calculations
2. ✅ **Per-Pixel Translation:** Full translation of coordinate-dependent expressions and color manipulation
3. ✅ **Variable Rewrites:** Automatic translation of per-pixel color variables (red/green/blue/alpha → pixelColor.r/g/b/a)
4. ✅ **State Flow:** Q1-Q32 and T1-T8 variables correctly flow from per-frame to per-pixel logic
5. ✅ **Function Mapping:** All MilkDrop functions (sin, cos, sqrt, if, rand, etc.) translated to GLSL equivalents
6. ✅ **UI Controls:** JSON-annotated uniforms for real-time parameter adjustment
7. ✅ **Feedback Loop:** Proper sampling from `iChannel0` with UV clamping
8. ✅ **Waveform Rendering:** nWaveMode=6 (Line Wave) support

## Known Limitations

1. **Wave Modes:** Only nWaveMode=6 currently implemented; modes 0-5 and 7+ pending
2. **Custom Shapes:** Shape rendering not yet implemented
3. **Feedback Buffer:** Rendering differences may occur vs manually-written shaders (see SHADERS.md diagnostics)
