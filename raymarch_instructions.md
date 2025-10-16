# Milk-Converter Instructions: Converting MilkDrop to RaymarchVibe GLSL

**This document provides technical instructions for Milk-Converter's conversion logic.**
See `../../../documentation/SHADERS.md` for the complete RaymarchVibe shader specification.

## Variable Mappings

MilkDrop variables MUST map to RaymarchVibe uniforms:

| MilkDrop | RaymarchVibe | Conversion |
|---|---|---|
| `time` | `iTime` | Direct |
| `fps` | `iFps` | Direct |
| `frame` | `iFrame` | Direct |
| `bass` | `iAudioBands.x` | Frequency band |
| `mid` | `iAudioBands.y` | Frequency band |
| `treb` | `iAudioBands.z` | Frequency band |
| `bass_att` | `iAudioBandsAtt.x` | Attacked (smoothed) audio |
| `mid_att` | `iAudioBandsAtt.y` | Attacked (smoothed) audio |
| `treb_att` | `iAudioBandsAtt.z` | Attacked (smoothed) audio |
| `rad` | `length(uv - vec2(0.5))` | Computed expression |
| `ang` | `atan(uv.y - 0.5, uv.x - 0.5)` | Computed expression |

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

## Bug Fixes Required

1. **Circular self-reference:** Remove statements like `iAudioBands = iAudioBands;`
2. **Invalid syntax:** Fix malformed GLSL expressions
3. **Feedback loop:** Implement proper sampling from `iChannel0`
4. **UV clamping:** Add texture coordinate boundaries for stability
