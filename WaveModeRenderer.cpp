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
    return wave_polar_vertex(radius, angle, center, aspect, 0.0);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 0: Spectrum circle bars
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    const float WAVE_MODE_HINT = 0.0;
    float angleLimit;
    float distanceClamp;
    float epsilon;
    wave_resolve_mode(WAVE_MODE_HINT, angleLimit, distanceClamp, epsilon);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    float mystery = clamp(wave_mystery * 0.5 + 0.5, -1.0, 1.0);
    mystery = abs(fract(mystery));
    mystery = mystery * 2.0 - 1.0;

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE0_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);
    float sample_count_f = float(sample_count);
    float angle_base = iTime * 0.2;
    float angle_step = min(WAVE_TWO_PI / max(sample_count_f, 1.0), angleLimit);

    float load = wave_estimate_load(segment_count, 0.01);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(segment_count, adjustedQuality);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float dots = wave_fallback_dots(uv, center, distanceClamp);
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(dots, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        float radius1 = clamp(0.5 + 0.4 * displacement1 * wave_scale + mystery, -2.0, 2.0);
        float radius2 = clamp(0.5 + 0.4 * displacement2 * wave_scale + mystery, -2.0, 2.0);
        float angle1 = angle_base + angle_step * float(i);
        float angle2 = angle1 + angle_step;
        vec2 p1 = wave_mode0_vertex(radius1, angle1, center, aspect);
        vec2 p2 = wave_mode0_vertex(radius2, angle2, center, aspect);
        float dist = wave_distance_to_segment(uv, p1, p2, distanceClamp, epsilon);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 2.0;
    float distanceClamp = wave_select_distance_clamp(WAVE_MODE_HINT);
    float epsilon = wave_select_epsilon(WAVE_MODE_HINT);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    int sample_count = max(min(samples, MODE2_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    float load = wave_estimate_load(sample_count, 0.012);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(sample_count, adjustedQuality);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float dots = wave_fallback_dots(uv, center, distanceClamp);
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(dots, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio.x : audio.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio.x : audio.y;
        vec2 point = wave_mode2_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / max(sample_count_f, 1.0);
        float dist = wave_safe_distance(uv, point, distanceClamp);
        float softness = max(0.005 + 0.01 * fade, epsilon * 4.0);
        float contribution = wave_contribution(dist, softness);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 3.0;
    float distanceClamp = wave_select_distance_clamp(WAVE_MODE_HINT);
    float epsilon = wave_select_epsilon(WAVE_MODE_HINT);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float base_scale = 0.25;
    float wave_scale = base_scale * wave_volume_scale(volume_level);
    int sample_count = max(min(samples, MODE3_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    float load = wave_estimate_load(sample_count, 0.014);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(sample_count, adjustedQuality);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float dots = wave_fallback_dots(uv, center, distanceClamp);
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(dots, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio.x : audio.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio.x : audio.y;
        vec2 point = wave_mode3_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / max(sample_count_f, 1.0);
        float dist = wave_safe_distance(uv, point, distanceClamp);
        float softness = max(0.007 + 0.01 * fade, epsilon * 6.0);
        float contribution = wave_contribution(dist, softness);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 4.0;
    float distanceClamp = wave_select_distance_clamp(WAVE_MODE_HINT);
    float epsilon = wave_select_epsilon(WAVE_MODE_HINT);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;
    vec2 center = vec2(wave_x, wave_y);

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE4_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float load = wave_estimate_load(segment_count, 0.01);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(segment_count, adjustedQuality);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float bars = wave_fallback_bars(uv, center, distanceClamp);
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(bars, adjustedQuality, notice, load);
    }

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(0.0, wave_x, wave_y, float(sample_count), WAVE_MODE_HINT, edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = wave_distance_to_segment(uv, p1, p2, distanceClamp, epsilon);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    return wave_polar_vertex(radius, angle, center, aspect, 5.0);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 5: Explosive hash radial pattern
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float wave_quality)
{
    const float WAVE_MODE_HINT = 5.0;
    float distanceClamp = wave_select_distance_clamp(WAVE_MODE_HINT);
    float epsilon = wave_select_epsilon(WAVE_MODE_HINT);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;

    int raw_samples = max(samples / 2, 1);
    int sample_count = max(min(raw_samples, MODE5_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    float load = wave_estimate_load(sample_count, 0.012);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(sample_count, adjustedQuality);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float dots = wave_fallback_dots(uv, center, distanceClamp);
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(dots, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement = (i % 2 == 0) ? audio.x : audio.y;
        float t = float(i) / max(sample_count_f, 1.0);
        float angle = wave_mystery + WAVE_TWO_PI * t;
        float radius = clamp(0.5 + 0.5 * displacement * wave_scale, 0.0, 2.0);
        vec2 point = wave_mode5_vertex(radius, angle, center, aspect);
        float dist = wave_safe_distance(uv, point, distanceClamp);
        float softness = max(0.008, epsilon * 8.0);
        float contribution = wave_contribution(dist, softness);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 6.0;
    float angleLimit;
    float distanceClamp;
    float epsilon;
    wave_resolve_mode(WAVE_MODE_HINT, angleLimit, distanceClamp, epsilon);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;
    vec2 center = vec2(wave_x, wave_y);

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE6_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = clamp(1.57 + clamp(wave_mystery, -1.0, 1.0), -angleLimit, angleLimit);

    float load = wave_estimate_load(segment_count, 0.01);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(segment_count, adjustedQuality);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), WAVE_MODE_HINT, edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float lineFallback = wave_fallback_line(uv, center, orientation, distanceClamp);
        if (adjustedQuality <= 0.2)
        {
            lineFallback = wave_fallback_bars(uv, center, distanceClamp);
        }
        if (adjustedQuality <= 0.05)
        {
            lineFallback = wave_fallback_dots(uv, center, distanceClamp);
        }
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(lineFallback, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = wave_distance_to_segment(uv, p1, p2, distanceClamp, epsilon);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 7.0;
    float angleLimit;
    float distanceClamp;
    float epsilon;
    wave_resolve_mode(WAVE_MODE_HINT, angleLimit, distanceClamp, epsilon);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;
    vec2 center = vec2(wave_x, wave_y);

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE7_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = clamp(1.57 * max(wave_mystery, 0.1), -angleLimit, angleLimit);

    float load = wave_estimate_load(segment_count, 0.01);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(segment_count, adjustedQuality);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), WAVE_MODE_HINT, edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    float separation = pow(clamp(wave_y * 0.5 + 0.5, 0.0, 1.0), 2.0);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float doubleFallback = wave_fallback_line(uv, center, orientation, distanceClamp);
        if (adjustedQuality <= 0.35)
        {
            doubleFallback = wave_fallback_bars(uv, center, distanceClamp);
        }
        if (adjustedQuality <= 0.1)
        {
            doubleFallback = wave_fallback_dots(uv, center, distanceClamp);
        }
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(doubleFallback, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        vec2 p1L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio.x, wave_scale, separation);
        vec2 p2L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio.x, wave_scale, separation);
        float distL = wave_distance_to_segment(uv, p1L, p2L, distanceClamp, epsilon);
        float contributionL = wave_contribution(distL, 0.01);
        intensity += contributionL;

        vec2 p1R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio.y, wave_scale, -separation);
        vec2 p2R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio.y, wave_scale, -separation);
        float distR = wave_distance_to_segment(uv, p1R, p2R, distanceClamp, epsilon);
        float contributionR = wave_contribution(distR, 0.01);
        intensity += contributionR;

        if (wave_should_exit(i, contributionL + contributionR))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
    const float WAVE_MODE_HINT = 8.0;
    float angleLimit;
    float distanceClamp;
    float epsilon;
    wave_resolve_mode(WAVE_MODE_HINT, angleLimit, distanceClamp, epsilon);

    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);

    int raw_samples = max(min(samples, 256), 2);
    int sample_count = min(raw_samples, MODE8_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = clamp(1.57 * max(wave_mystery, 0.1), -angleLimit, angleLimit);

    float load = wave_estimate_load(segment_count, 0.01);
    float adjustedQuality = wave_adjust_quality(wave_quality, load);
    int iterationBudget = wave_iteration_budget(segment_count, adjustedQuality);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), WAVE_MODE_HINT, edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    if (wave_is_overloaded(adjustedQuality, load))
    {
        float spectrumFallback = wave_fallback_line(uv, center, orientation, distanceClamp);
        if (adjustedQuality <= 0.2)
        {
            spectrumFallback = wave_fallback_bars(uv, center, distanceClamp);
        }
        if (adjustedQuality <= 0.05)
        {
            spectrumFallback = wave_fallback_dots(uv, center, distanceClamp);
        }
        float notice = wave_quality_notice(adjustedQuality, wave_quality);
        return wave_finalize_fallback(spectrumFallback, adjustedQuality, notice, load);
    }

    for (int i = 0; i < iterationBudget; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1);
        vec2 p2 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2);
        float dist = wave_distance_to_segment(uv, p1, p2, distanceClamp, epsilon);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    float notice = wave_quality_notice(adjustedQuality, wave_quality);
    return wave_apply_progressive(intensity + notice, load) * (0.8 + 0.2 * adjustedQuality);
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
const float WAVE_EPSILON_BASE = 1e-5;
const float WAVE_EPSILON_FINE = 2.5e-6;
const float WAVE_INTENSITY_CUTOFF = 1e-4;
const float WAVE_DISTANCE_CLAMP_BASE = 8.0;
const float WAVE_DISTANCE_CLAMP_LINE = 6.75;
const float WAVE_DISTANCE_CLAMP_RADIAL = 7.5;
const float WAVE_DISTANCE_TOLERANCE = 2.5e-3;
const float WAVE_MAX_ANGLE_BASE = 8192.0;
const float WAVE_MAX_ANGLE_RADIAL = 9216.0;
const float WAVE_MAX_ANGLE_LINE = 6144.0;
const float WAVE_MAX_ANGLE_SPECTRUM = 5632.0;
const float WAVE_TWO_PI = 6.28318530718;
const float WAVE_PI = 3.14159265359;
const float WAVE_TRIG_TAYLOR_THRESHOLD = 0.78539816339;
const float WAVE_TRIG_EXTREME_THRESHOLD = 1024.0;
const int WAVE_MIN_WARMUP_ITERATIONS = 4;

const int MODE0_MAX_WAVE_ITERATIONS = 48;
const int MODE2_MAX_WAVE_ITERATIONS = 48;
const int MODE3_MAX_WAVE_ITERATIONS = 48;
const int MODE4_MAX_WAVE_ITERATIONS = 64;
const int MODE5_MAX_WAVE_ITERATIONS = 48;
const int MODE6_MAX_WAVE_ITERATIONS = 64;
const int MODE7_MAX_WAVE_ITERATIONS = 48;
const int MODE8_MAX_WAVE_ITERATIONS = 64;

const int WAVE_SEGMENT_SCAN_LIMIT = 6;

const float WAVE_AUDIO_SENSITIVITY = 1.35;
const float WAVE_AUDIO_BAND_SMOOTH = 0.32;
const float WAVE_AUDIO_ATTACK = 0.82;
const float WAVE_AUDIO_DECAY = 0.42;
const float WAVE_AUDIO_MIN_PEAK = 1e-3;
const float WAVE_AUDIO_DYNAMIC_RANGE = 1.6;
const float WAVE_COMPLEXITY_THRESHOLD = 360.0;
const float WAVE_COMPLEXITY_CRITICAL = 520.0;
const float WAVE_PROGRESSIVE_TARGET = 24.0;

const float WAVE_DISTANCE_CLAMP = WAVE_DISTANCE_CLAMP_BASE;
const float WAVE_MAX_ANGLE = WAVE_MAX_ANGLE_BASE;
const float WAVE_EPSILON = WAVE_EPSILON_BASE;

vec2 wave_aspect()
{
    return vec2(1.0, 1.0);
}

float wave_select_angle_limit(float modeId)
{
    if (modeId < 0.5) { return WAVE_MAX_ANGLE_RADIAL; }
    if (modeId < 3.5) { return WAVE_MAX_ANGLE_RADIAL; }
    if (modeId < 6.5) { return WAVE_MAX_ANGLE_LINE; }
    return WAVE_MAX_ANGLE_SPECTRUM;
}

float wave_select_distance_clamp(float modeId)
{
    if (modeId < 0.5) { return WAVE_DISTANCE_CLAMP_RADIAL; }
    if (modeId < 3.5) { return WAVE_DISTANCE_CLAMP_RADIAL; }
    if (modeId < 6.5) { return WAVE_DISTANCE_CLAMP_LINE; }
    return WAVE_DISTANCE_CLAMP_BASE;
}

float wave_select_epsilon(float modeId)
{
    return modeId < 6.5 ? WAVE_EPSILON_FINE : WAVE_EPSILON_BASE;
}

void wave_resolve_mode(float modeId, out float angleLimit, out float distanceClamp, out float epsilon)
{
    angleLimit = wave_select_angle_limit(modeId);
    distanceClamp = wave_select_distance_clamp(modeId);
    epsilon = wave_select_epsilon(modeId);
}

float wave_wrap_angle(float angle, float limit)
{
    float capped = clamp(angle, -limit, limit);
    return capped - WAVE_TWO_PI * floor((capped + WAVE_PI) / WAVE_TWO_PI);
}

vec2 wave_sin_cos_poly(float angle)
{
    float x2 = angle * angle;
    float sinPoly = angle * (1.0 - x2 * (1.0 / 6.0) + x2 * x2 * (1.0 / 120.0));
    float cosPoly = 1.0 - x2 * 0.5 + x2 * x2 * (1.0 / 24.0);
    return vec2(sinPoly, cosPoly);
}

vec2 wave_sin_cos_safe(float angle)
{
    return wave_sin_cos_safe(angle, -1.0);
}

vec2 wave_sin_cos_safe(float angle, float modeId)
{
    float limit = modeId < -0.5 ? WAVE_MAX_ANGLE_BASE : wave_select_angle_limit(modeId);
    float wrapped = wave_wrap_angle(angle, limit);
    float absAngle = abs(wrapped);
    vec2 poly = wave_sin_cos_poly(wrapped);
    vec2 exact = vec2(sin(wrapped), cos(wrapped));
    float mixFactor = smoothstep(WAVE_TRIG_TAYLOR_THRESHOLD, WAVE_TRIG_TAYLOR_THRESHOLD * 2.0, absAngle);
    vec2 result = mix(poly, exact, mixFactor);
    float extreme = smoothstep(WAVE_TRIG_EXTREME_THRESHOLD, limit, absAngle);
    result *= (1.0 - 0.08 * extreme);
    return result;
}

float wave_safe_sin(float angle)
{
    return wave_safe_sin(angle, -1.0);
}

float wave_safe_cos(float angle)
{
    return wave_safe_cos(angle, -1.0);
}

float wave_safe_sin(float angle, float modeId)
{
    return wave_sin_cos_safe(angle, modeId).x;
}

float wave_safe_cos(float angle, float modeId)
{
    return wave_sin_cos_safe(angle, modeId).y;
}

vec2 wave_clamp_vec(vec2 value, float limit)
{
    return clamp(value, vec2(-limit), vec2(limit));
}

vec2 wave_clamp_vec(vec2 value)
{
    return wave_clamp_vec(value, WAVE_DISTANCE_CLAMP_BASE);
}

float wave_safe_length(vec2 value, float limit)
{
    vec2 clamped = wave_clamp_vec(value, limit);
    float lenSq = dot(clamped, clamped);
    return sqrt(lenSq);
}

float wave_safe_length(vec2 value)
{
    return wave_safe_length(value, WAVE_DISTANCE_CLAMP_BASE);
}

float wave_safe_distance(vec2 a, vec2 b, float limit)
{
    vec2 diff = wave_clamp_vec(a - b, limit);
    float lenSq = dot(diff, diff);
    if (lenSq <= WAVE_DISTANCE_TOLERANCE * WAVE_DISTANCE_TOLERANCE)
    {
        return sqrt(lenSq);
    }
    float maxAxis = max(abs(diff.x), abs(diff.y));
    float minAxis = min(abs(diff.x), abs(diff.y));
    float manhattanApprox = maxAxis + 0.5 * minAxis;
    return min(sqrt(lenSq), manhattanApprox);
}

float wave_safe_distance(vec2 a, vec2 b)
{
    return wave_safe_distance(a, b, WAVE_DISTANCE_CLAMP_BASE);
}

float wave_contribution(float distance, float softness)
{
    if (distance <= WAVE_DISTANCE_TOLERANCE * 0.5)
    {
        return 1.0;
    }
    float safeSoftness = max(softness, WAVE_EPSILON_BASE);
    if (distance >= WAVE_DISTANCE_CLAMP_BASE)
    {
        return 0.0;
    }
    if (distance >= safeSoftness * 5.0)
    {
        return 0.0;
    }
    float clampedDistance = clamp(distance, 0.0, WAVE_DISTANCE_CLAMP_BASE);
    return 1.0 - smoothstep(0.0, safeSoftness, clampedDistance);
}

float wave_distance_to_segment(vec2 p, vec2 v, vec2 w)
{
    return wave_distance_to_segment(p, v, w, WAVE_DISTANCE_CLAMP_BASE, WAVE_EPSILON_BASE);
}

float wave_distance_to_segment(vec2 p, vec2 v, vec2 w, float clampLimit, float epsilon)
{
    vec2 diff = wave_clamp_vec(w - v, clampLimit);
    float l2 = max(dot(diff, diff), epsilon);
    float invLength = inversesqrt(l2);
    float segmentLength = l2 * invLength;
    int scanCount = int(ceil(segmentLength * 0.5));
    scanCount = min(max(scanCount, 1), WAVE_SEGMENT_SCAN_LIMIT);
    float minDistance = WAVE_DISTANCE_CLAMP_BASE;
    for (int i = 0; i < WAVE_SEGMENT_SCAN_LIMIT; ++i)
    {
        if (i >= scanCount)
        {
            break;
        }
        float t = float(i) / float(max(scanCount - 1, 1));
        vec2 samplePoint = v + diff * t;
        float distance = wave_safe_distance(p, samplePoint, clampLimit);
        minDistance = min(minDistance, distance);
        if (minDistance <= WAVE_DISTANCE_TOLERANCE)
        {
            return minDistance;
        }
    }
    float tProj = clamp(dot(p - v, diff) / l2, 0.0, 1.0);
    vec2 projection = v + diff * tProj;
    float projectionDistance = wave_safe_distance(p, projection, clampLimit);
    return min(minDistance, projectionDistance);
}

float wave_safe_divide(float numerator, float denominator)
{
    return wave_safe_divide(numerator, denominator, WAVE_EPSILON_BASE);
}

float wave_safe_divide(float numerator, float denominator, float epsilon)
{
    float denom = abs(denominator) < epsilon
        ? (denominator >= 0.0 ? epsilon : -epsilon)
        : denominator;
    return numerator / denom;
}

float wave_fast_tanh(float x)
{
    float e2x = exp(-2.0 * abs(x));
    return sign(x) * (1.0 - e2x) / (1.0 + e2x);
}

vec2 wave_fast_tanh(vec2 x)
{
    return vec2(wave_fast_tanh(x.x), wave_fast_tanh(x.y));
}

vec2 wave_smoothstep_clamp(vec2 audio)
{
    vec2 stepped = smoothstep(vec2(-1.0), vec2(1.0), audio);
    return stepped * 2.0 - 1.0;
}

vec2 wave_clamp_audio(vec2 audio)
{
    vec2 soft = wave_smoothstep_clamp(audio);
    vec2 clipped = wave_fast_tanh(soft * WAVE_AUDIO_SENSITIVITY);
    float average = (clipped.x + clipped.y) * 0.5;
    vec2 bandAverage = vec2(average);
    vec2 smoothed = mix(clipped, bandAverage, WAVE_AUDIO_BAND_SMOOTH);
    vec2 attackMix = vec2(
        clipped.x >= smoothed.x ? WAVE_AUDIO_ATTACK : WAVE_AUDIO_DECAY,
        clipped.y >= smoothed.y ? WAVE_AUDIO_ATTACK : WAVE_AUDIO_DECAY);
    vec2 preserved = mix(smoothed, clipped, attackMix);
    float peak = max(max(abs(preserved.x), abs(preserved.y)), WAVE_AUDIO_MIN_PEAK);
    float rangeScale = clamp(WAVE_AUDIO_DYNAMIC_RANGE / (WAVE_AUDIO_DYNAMIC_RANGE + peak - 1.0), 0.75, WAVE_AUDIO_DYNAMIC_RANGE);
    return preserved * rangeScale;
}

float wave_volume_scale(float volume)
{
    float soft = wave_fast_tanh(volume * WAVE_AUDIO_SENSITIVITY);
    float normalized = (soft + 1.0) * 0.5;
    return mix(0.1, 2.5, pow(normalized, 1.2));
}

float wave_estimate_load(int iterations, float softness)
{
    return float(iterations) * (1.0 + 60.0 * softness);
}

float wave_adjust_quality(float wave_quality, float estimatedLoad)
{
    if (estimatedLoad <= WAVE_COMPLEXITY_THRESHOLD)
    {
        return wave_quality;
    }
    float scale = clamp(WAVE_COMPLEXITY_THRESHOLD / max(estimatedLoad, 1.0), 0.2, 1.0);
    return wave_quality * scale;
}

bool wave_is_overloaded(float wave_quality, float estimatedLoad)
{
    return (wave_quality <= 0.0) || (estimatedLoad >= WAVE_COMPLEXITY_CRITICAL);
}

int wave_iteration_budget(int iterationCount, float adjustedQuality)
{
    float scaled = float(iterationCount) * clamp(adjustedQuality, 0.1, 1.0);
    int budget = int(floor(scaled + 0.5));
    budget = max(1, budget);
    return min(budget, iterationCount);
}

float wave_progressive_factor()
{
#ifdef iFrame
    return clamp(float(iFrame) / WAVE_PROGRESSIVE_TARGET, 0.3, 1.0);
#else
    return 1.0;
#endif
}

float wave_apply_progressive(float intensity, float estimatedLoad)
{
    float progressive = wave_progressive_factor();
    float damping = 1.0 - smoothstep(WAVE_COMPLEXITY_THRESHOLD, WAVE_COMPLEXITY_CRITICAL, estimatedLoad) * 0.15;
    return intensity * progressive * damping;
}

float wave_finalize_fallback(float intensity, float adjustedQuality, float notice, float load)
{
    float fallbackScale = 0.8 + 0.2 * adjustedQuality;
    return wave_apply_progressive(intensity * fallbackScale + notice, load);
}

float wave_quality_notice(float adjustedQuality, float originalQuality)
{
    return max(originalQuality - adjustedQuality, 0.0) * 0.01;
}

float wave_fallback_dots(vec2 uv, vec2 center, float distanceClamp)
{
    float dist = wave_safe_distance(uv, center, distanceClamp);
    return wave_contribution(dist, 0.035);
}

float wave_fallback_line(vec2 uv, vec2 center, float orientation, float distanceClamp)
{
    vec2 direction = vec2(wave_safe_cos(orientation, 6.0), wave_safe_sin(orientation, 6.0));
    vec2 start = center - direction * 0.3;
    vec2 end = center + direction * 0.3;
    float dist = wave_distance_to_segment(uv, start, end, distanceClamp, wave_select_epsilon(6.0));
    return wave_contribution(dist, 0.02);
}

float wave_fallback_bars(vec2 uv, vec2 center, float distanceClamp)
{
    vec2 a = center + vec2(-0.4, 0.0);
    vec2 b = center + vec2(0.4, 0.0);
    float dist = wave_distance_to_segment(uv, a, b, distanceClamp, wave_select_epsilon(4.0));
    return wave_contribution(dist, 0.03);
}

bool wave_should_exit(int index, float contribution)
{
    return (index >= WAVE_MIN_WARMUP_ITERATIONS) && (contribution <= WAVE_INTENSITY_CUTOFF);
}

vec2 wave_polar_vertex(float radius, float angle, vec2 center, vec2 aspect, float modeId)
{
    float distanceClamp = wave_select_distance_clamp(modeId);
    float safeRadius = clamp(radius, -2.0, 2.0);
    float c = wave_safe_cos(angle, modeId);
    float s = wave_safe_sin(angle, modeId);
    vec2 offset = vec2(safeRadius * c * aspect.y,
                       safeRadius * s * aspect.x);
    offset = wave_clamp_vec(offset, distanceClamp);
    return center + offset;
}

void clip_waveform_edges(float angle, float wave_x, float wave_y, float sample_count, float modeId,
                         out float edge_x, out float edge_y,
                         out float distance_x, out float distance_y,
                         out float perpendicular_dx, out float perpendicular_dy)
{
    float angleLimit;
    float distanceClamp;
    float epsilon;
    wave_resolve_mode(modeId, angleLimit, distanceClamp, epsilon);

    float safeAngle = wave_wrap_angle(angle, angleLimit);
    float c = wave_safe_cos(safeAngle, modeId);
    float s = wave_safe_sin(safeAngle, modeId);
    float orthoAngle = safeAngle + 1.57;
    float orthoCos = wave_safe_cos(orthoAngle, modeId);
    float orthoSin = wave_safe_sin(orthoAngle, modeId);

    vec2 direction = vec2(c, s);
    vec2 edge[2];
    edge[0] = wave_clamp_vec(vec2(wave_x * orthoCos - direction.x * 3.0,
                                  wave_y * orthoSin - direction.y * 3.0), distanceClamp);
    edge[1] = wave_clamp_vec(vec2(wave_x * orthoCos + direction.x * 3.0,
                                  wave_y * orthoSin + direction.y * 3.0), distanceClamp);

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float t = 0.0;
            bool clip = false;
            if (j == 0 && edge[i].x > 1.1)
            {
                t = wave_safe_divide(1.1 - edge[1 - i].x, edge[i].x - edge[1 - i].x, epsilon);
                clip = true;
            }
            else if (j == 1 && edge[i].x < -1.1)
            {
                t = wave_safe_divide(-1.1 - edge[1 - i].x, edge[i].x - edge[1 - i].x, epsilon);
                clip = true;
            }
            else if (j == 2 && edge[i].y > 1.1)
            {
                t = wave_safe_divide(1.1 - edge[1 - i].y, edge[i].y - edge[1 - i].y, epsilon);
                clip = true;
            }
            else if (j == 3 && edge[i].y < -1.1)
            {
                t = wave_safe_divide(-1.1 - edge[1 - i].y, edge[i].y - edge[1 - i].y, epsilon);
                clip = true;
            }

            if (clip)
            {
                t = clamp(t, 0.0, 1.0);
                vec2 diff = edge[i] - edge[1 - i];
                edge[i] = wave_clamp_vec(edge[1 - i] + diff * t, distanceClamp);
            }
        }
    }

    vec2 diff = wave_clamp_vec(edge[1] - edge[0], distanceClamp);
    float inv_samples = wave_safe_divide(1.0, max(sample_count, 1.0), epsilon);
    vec2 delta = diff * inv_samples;

    edge_x = edge[0].x;
    edge_y = edge[0].y;
    distance_x = delta.x;
    distance_y = delta.y;

    float angle2 = atan(delta.y, delta.x);
    perpendicular_dx = wave_safe_cos(angle2 + 1.57, modeId);
    perpendicular_dy = wave_safe_sin(angle2 + 1.57, modeId);
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
