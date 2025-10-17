#include "WaveModeRenderer.hpp"

namespace {

class CircleWaveRenderer final : public WaveModeRenderer {
public:
    explicit CircleWaveRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode0_vertex(float radius, float angle, vec2 center, vec2 aspect)
{
    return vec2(radius * cos(angle) * aspect.y + center.x,
                radius * sin(angle) * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 0: Spectrum circle bars
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    float mystery = wave_mystery * 0.5 + 0.5;
    mystery = abs(fract(mystery));
    mystery = mystery * 2.0 - 1.0;

    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples / 2, 256);
    float smoothing_width = 0.01 / clamp(wave_quality, 0.5, 1.0);
    
    for (int i = 0; i < num_samples - 1; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio_data.x : audio_data.y;
        float radius1 = 0.5 + 0.4 * displacement1 * wave_scale + mystery;
        float radius2 = 0.5 + 0.4 * displacement2 * wave_scale + mystery;
        float angle1 = float(i) / float(num_samples) * 6.28318 + iTime * 0.2;
        float angle2 = float(i + 1) / float(num_samples) * 6.28318 + iTime * 0.2;
        vec2 p1 = wave_mode0_vertex(radius1, angle1, center, aspect);
        vec2 p2 = wave_mode0_vertex(radius2, angle2, center, aspect);
        float dist = distance_to_line_segment(uv, p1, p2);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(
draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality)
)___";
    }
};

class CenteredSpiroRenderer final : public WaveModeRenderer {
public:
    explicit CenteredSpiroRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode2_vertex(float displacement_x, float displacement_y, vec2 center, vec2 aspect, float wave_scale)
{
    return vec2(displacement_x * wave_scale * aspect.y + center.x,
                displacement_y * wave_scale * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 2: Centered dots with trails
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples, 512);
    float size_adjust = 1.0 + (1.0 - wave_quality) * 0.5;

    for (int i = 0; i < num_samples; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio_data.x : audio_data.y;
        vec2 point = wave_mode2_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / float(num_samples);
        float dist = distance(uv, point);
        intensity += (1.0 - smoothstep(0.0, (0.005 + 0.01 * fade) * size_adjust, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

class CenteredSpiroVolumeRenderer final : public WaveModeRenderer {
public:
    explicit CenteredSpiroVolumeRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode3_vertex(float displacement_x, float displacement_y, vec2 center, vec2 aspect, float wave_scale)
{
    return vec2(displacement_x * wave_scale * aspect.y + center.x,
                displacement_y * wave_scale * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 3: Volume-modulated centered dots
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float volume_level, float wave_quality)
{
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float base_scale = 0.25;
    float volume_factor = clamp(volume_level * volume_level * 1.3, 0.1, 2.5);
    float wave_scale = base_scale * volume_factor;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples, 512);
    float size_adjust = 1.0 + (1.0 - wave_quality) * 0.5;

    for (int i = 0; i < num_samples; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio_data.x : audio_data.y;
        vec2 point = wave_mode3_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / float(num_samples);
        float dist = distance(uv, point);
        intensity += (1.0 - smoothstep(0.0, (0.007 + 0.01 * fade) * size_adjust, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(
draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, iAudioBands.z, wave_quality)
)___";
    }
};

class DerivativeLineRenderer final : public WaveModeRenderer {
public:
    explicit DerivativeLineRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode_line_vertex(float edge_x, float edge_y, float distance_x, float distance_y,
                           float perpendicular_dx, float perpendicular_dy, float index,
                           float displacement, float wave_scale)
{
    return vec2(edge_x + distance_x * index + perpendicular_dx * 0.25 * displacement * wave_scale,
                edge_y + distance_y * index + perpendicular_dy * 0.25 * displacement * wave_scale);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 4: Derivative line (scripted horizontal display)
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    float wave_scale = 0.25;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples / 2, 256);
    float smoothing_width = 0.01 / clamp(wave_quality, 0.5, 1.0);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(0.0, wave_x, wave_y, float(num_samples), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < num_samples - 1; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio_data.x : audio_data.y;
        vec2 p1 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = distance_to_line_segment(uv, p1, p2);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

class ExplosiveHashRenderer final : public WaveModeRenderer {
public:
    explicit ExplosiveHashRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode5_vertex(float radius, float angle, vec2 center, vec2 aspect)
{
    return vec2(radius * cos(angle) * aspect.y + center.x,
                radius * sin(angle) * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 5: Explosive hash radial pattern
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples / 2, 256);
    float size_adjust = 1.0 + (1.0 - wave_quality) * 0.4;

    for (int i = 0; i < num_samples; ++i)
    {
        float displacement = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float t = float(i) / max(float(num_samples), 1.0);
        float angle = t * 6.28318 + wave_mystery;
        float radius = 0.5 + 0.5 * displacement * wave_scale;
        vec2 point = wave_mode5_vertex(radius, angle, center, aspect);
        float dist = distance(uv, point);
        intensity += (1.0 - smoothstep(0.0, 0.008 * size_adjust, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

class LineWaveRenderer final : public WaveModeRenderer {
public:
    explicit LineWaveRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode6_vertex(float edge_x, float edge_y, float distance_x, float distance_y,
                       float perpendicular_dx, float perpendicular_dy, float index,
                       float displacement, float wave_scale)
{
    return vec2(edge_x + distance_x * index + perpendicular_dx * 0.25 * displacement * wave_scale,
                edge_y + distance_y * index + perpendicular_dy * 0.25 * displacement * wave_scale);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 6: Angle-adjustable line spectrum
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    float wave_scale = 0.25;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples / 2, 256);
    float smoothing_width = 0.01 / clamp(wave_quality, 0.5, 1.0);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(1.57 + wave_mystery, wave_x, wave_y, float(num_samples), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < num_samples - 1; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio_data.x : audio_data.y;
        vec2 p1 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = distance_to_line_segment(uv, p1, p2);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

class DoubleLineWaveRenderer final : public WaveModeRenderer {
public:
    explicit DoubleLineWaveRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode7_vertex(float edge_x, float edge_y, float distance_x, float distance_y,
                       float perpendicular_dx, float perpendicular_dy, float index,
                       float displacement, float wave_scale, float separation)
{
    return vec2(edge_x + distance_x * index + perpendicular_dx * (0.25 * displacement * wave_scale + separation),
                edge_y + distance_y * index + perpendicular_dy * (0.25 * displacement * wave_scale + separation));
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 7: Double spectrum lines
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    float wave_scale = 0.25;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(quality_samples / 2, 256);
    float smoothing_width = 0.01 / clamp(wave_quality, 0.5, 1.0);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(1.57 * max(wave_mystery, 0.1), wave_x, wave_y, float(num_samples), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    float separation = pow(wave_y * 0.5 + 0.5, 2.0);

    for (int i = 0; i < num_samples - 1; ++i)
    {
        vec2 p1L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio_data.x, wave_scale, separation);
        vec2 p2L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio_data.x, wave_scale, separation);
        float distL = distance_to_line_segment(uv, p1L, p2L);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, distL));

        vec2 p1R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio_data.y, wave_scale, -separation);
        vec2 p2R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio_data.y, wave_scale, -separation);
        float distR = distance_to_line_segment(uv, p1R, p2R);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, distR));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

class SpectrumLineRenderer final : public WaveModeRenderer {
public:
    explicit SpectrumLineRenderer(const std::map<std::string, std::string>& presetValues)
        : WaveModeRenderer(presetValues) {}

    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode8_vertex(float edge_x, float edge_y, float distance_x, float distance_y,
                       float perpendicular_dx, float perpendicular_dy, float index, float displacement)
{
    float f = 0.1 * log(max(abs(displacement), 0.0001));
    return vec2(edge_x + distance_x * index + perpendicular_dx * f,
                edge_y + distance_y * index + perpendicular_dy * f);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 8: Spectrum line (angled analyser)
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    float intensity = 0.0;
    int quality_samples = int(float(samples) * clamp(wave_quality, 0.1, 1.0));
    int num_samples = min(256, quality_samples);
    float smoothing_width = 0.01 / clamp(wave_quality, 0.5, 1.0);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(1.57 * max(wave_mystery, 0.1), wave_x, wave_y, float(num_samples), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < num_samples - 1; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio_data.x : audio_data.y;
        vec2 p1 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1);
        vec2 p2 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2);
        float dist = distance_to_line_segment(uv, p1, p2);
        intensity += (1.0 - smoothstep(0.0, smoothing_width, dist));
    }

    return intensity * (0.8 + 0.2 * wave_quality);
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }
};

} // namespace

std::string WaveModeRenderer::helperFunctions() const
{
    return generateCommonHelpers();
}

std::string WaveModeRenderer::generateWaveformGLSL(int nWaveMode, const std::map<std::string, std::string>& presetValues)
{
    (void)presetValues;

    auto renderer = create(nWaveMode, presetValues);
    if (!renderer)
    {
        return generateCommonHelpers() + generateFallback();
    }

    std::string glsl = renderer->helperFunctions();
    glsl += renderer->vertexFunction();
    glsl += renderer->drawFunction();
    return glsl;
}

std::string WaveModeRenderer::generateCallPattern(int nWaveMode, const std::map<std::string, std::string>& presetValues)
{
    auto renderer = create(nWaveMode, presetValues);
    if (!renderer)
    {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, wave_quality))___";
    }

    return renderer->callPattern();
}

std::unique_ptr<WaveModeRenderer> WaveModeRenderer::create(int nWaveMode, const std::map<std::string, std::string>& presetValues)
{
    switch (nWaveMode)
    {
        case 0:
            return std::make_unique<CircleWaveRenderer>(presetValues);
        case 2:
            return std::make_unique<CenteredSpiroRenderer>(presetValues);
        case 3:
            return std::make_unique<CenteredSpiroVolumeRenderer>(presetValues);
        case 4:
            return std::make_unique<DerivativeLineRenderer>(presetValues);
        case 5:
            return std::make_unique<ExplosiveHashRenderer>(presetValues);
        case 6:
            return std::make_unique<LineWaveRenderer>(presetValues);
        case 7:
            return std::make_unique<DoubleLineWaveRenderer>(presetValues);
        case 8:
            return std::make_unique<SpectrumLineRenderer>(presetValues);
        default:
            return nullptr;
    }
}

std::string WaveModeRenderer::generateCommonHelpers()
{
    return R"___(
vec2 wave_aspect()
{
    return vec2(1.0, 1.0);
}

float distance_to_line_segment(vec2 p, vec2 v, vec2 w)
{
    float l2 = dot(w - v, w - v);
    if (l2 == 0.0)
    {
        return distance(p, v);
    }
    float t = clamp(dot(p - v, w - v) / l2, 0.0, 1.0);
    vec2 projection = mix(v, w, t);
    return distance(p, projection);
}

void clip_waveform_edges(float angle, float wave_x, float wave_y, float sample_count,
                         out float edge_x, out float edge_y,
                         out float distance_x, out float distance_y,
                         out float perpendicular_dx, out float perpendicular_dy)
{
    vec2 direction = vec2(cos(angle), sin(angle));
    vec2 edge[2];
    edge[0] = vec2(wave_x * cos(angle + 1.57) - direction.x * 3.0,
                   wave_y * sin(angle + 1.57) - direction.y * 3.0);
    edge[1] = vec2(wave_x * cos(angle + 1.57) + direction.x * 3.0,
                   wave_y * sin(angle + 1.57) + direction.y * 3.0);

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float t = 0.0;
            bool clip = false;
            if (j == 0 && edge[i].x > 1.1)
            {
                t = (1.1 - edge[1 - i].x) / (edge[i].x - edge[1 - i].x);
                clip = true;
            }
            else if (j == 1 && edge[i].x < -1.1)
            {
                t = (-1.1 - edge[1 - i].x) / (edge[i].x - edge[1 - i].x);
                clip = true;
            }
            else if (j == 2 && edge[i].y > 1.1)
            {
                t = (1.1 - edge[1 - i].y) / (edge[i].y - edge[1 - i].y);
                clip = true;
            }
            else if (j == 3 && edge[i].y < -1.1)
            {
                t = (-1.1 - edge[1 - i].y) / (edge[i].y - edge[1 - i].y);
                clip = true;
            }

            if (clip)
            {
                vec2 diff = edge[i] - edge[1 - i];
                edge[i] = edge[1 - i] + diff * t;
            }
        }
    }

    float inv_samples = 1.0 / max(sample_count, 1.0);
    distance_x = (edge[1].x - edge[0].x) * inv_samples;
    distance_y = (edge[1].y - edge[0].y) * inv_samples;

    edge_x = edge[0].x;
    edge_y = edge[0].y;

    float angle2 = atan(distance_y, distance_x);
    perpendicular_dx = cos(angle2 + 1.57);
    perpendicular_dy = sin(angle2 + 1.57);
}
)___";
}

std::string WaveModeRenderer::generateFallback()
{
    return R"___(
// Fallback waveform renderer when the mode is unsupported
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    (void)uv;
    (void)audio_data;
    (void)samples;
    (void)wave_x;
    (void)wave_y;
    (void)wave_mystery;
    (void)wave_quality;
    return 0.0;
}
)___";
}

float WaveModeRenderer::presetFloat(const std::string& key, float fallback) const
{
    auto it = m_presetValues.find(key);
    if (it == m_presetValues.end())
    {
        return fallback;
    }

    try
    {
        return std::stof(it->second);
    }
    catch (const std::exception&)
    {
        return fallback;
    }
}

int WaveModeRenderer::presetInt(const std::string& key, int fallback) const
{
    auto it = m_presetValues.find(key);
    if (it == m_presetValues.end())
    {
        return fallback;
    }

    try
    {
        return std::stoi(it->second);
    }
    catch (const std::exception&)
    {
        return fallback;
    }
}
