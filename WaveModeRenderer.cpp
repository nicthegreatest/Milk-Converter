#include "WaveModeRenderer.hpp"

namespace {

class CircleWaveRenderer final : public WaveModeRenderer {
public:
    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode0_vertex(float radius, float angle, vec2 center, vec2 aspect)
{
    float safeRadius = clamp(radius, -2.0, 2.0);
    float c = wave_safe_cos(angle);
    float s = wave_safe_sin(angle);
    return vec2(safeRadius * c * aspect.y + center.x,
                safeRadius * s * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 0: Spectrum circle bars
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
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
    float angle_step = WAVE_TWO_PI / max(sample_count_f, 1.0);

    for (int i = 0; i < segment_count; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        float radius1 = clamp(0.5 + 0.4 * displacement1 * wave_scale + mystery, -2.0, 2.0);
        float radius2 = clamp(0.5 + 0.4 * displacement2 * wave_scale + mystery, -2.0, 2.0);
        float angle1 = angle_base + angle_step * float(i);
        float angle2 = angle1 + angle_step;
        vec2 p1 = wave_mode0_vertex(radius1, angle1, center, aspect);
        vec2 p2 = wave_mode0_vertex(radius2, angle2, center, aspect);
        float dist = wave_distance_to_segment(uv, p1, p2);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(
draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery)
)___";
    }
};

class CenteredSpiroRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;
    int sample_count = max(min(samples, MODE2_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    for (int i = 0; i < sample_count; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio.x : audio.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio.x : audio.y;
        vec2 point = wave_mode2_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / max(sample_count_f, 1.0);
        float dist = wave_safe_distance(uv, point);
        float contribution = wave_contribution(dist, 0.005 + 0.01 * fade);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
    }
};

class CenteredSpiroVolumeRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery, float volume_level)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float base_scale = 0.25;
    float volume = clamp(volume_level, -1.0, 1.0);
    float volume_factor = clamp(volume * volume * 1.3, 0.1, 2.5);
    float wave_scale = base_scale * volume_factor;
    int sample_count = max(min(samples, MODE3_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    for (int i = 0; i < sample_count; ++i)
    {
        float displacement_x = (i % 2 == 0) ? audio.x : audio.y;
        float displacement_y = ((i + 32) % 2 == 0) ? audio.x : audio.y;
        vec2 point = wave_mode3_vertex(displacement_x, displacement_y, center, aspect, wave_scale);
        float fade = 1.0 - float(i) / max(sample_count_f, 1.0);
        float dist = wave_safe_distance(uv, point);
        float contribution = wave_contribution(dist, 0.007 + 0.01 * fade);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(
draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery, iAudioBands.z)
)___";
    }
};

class DerivativeLineRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE4_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(0.0, wave_x, wave_y, float(sample_count), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < segment_count; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode_line_vertex(edge_x, edge_y, distance_x, distance_y,
                                        perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = wave_distance_to_segment(uv, p1, p2);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
    }
};

class ExplosiveHashRenderer final : public WaveModeRenderer {
public:
    std::string vertexFunction() const override {
        return R"___(
vec2 wave_mode5_vertex(float radius, float angle, vec2 center, vec2 aspect)
{
    float safeRadius = clamp(radius, -2.0, 2.0);
    float c = wave_safe_cos(angle);
    float s = wave_safe_sin(angle);
    return vec2(safeRadius * c * aspect.y + center.x,
                safeRadius * s * aspect.x + center.y);
}
)___";
    }

    std::string drawFunction() const override {
        return R"___(
// Mode 5: Explosive hash radial pattern
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    vec2 center = vec2(wave_x, wave_y);
    vec2 aspect = wave_aspect();
    float wave_scale = 0.25;

    int raw_samples = max(samples / 2, 1);
    int sample_count = max(min(raw_samples, MODE5_MAX_WAVE_ITERATIONS), 1);
    float sample_count_f = float(sample_count);

    for (int i = 0; i < sample_count; ++i)
    {
        float displacement = (i % 2 == 0) ? audio.x : audio.y;
        float t = float(i) / max(sample_count_f, 1.0);
        float angle = wave_mystery + WAVE_TWO_PI * t;
        float radius = clamp(0.5 + 0.5 * displacement * wave_scale, 0.0, 2.0);
        vec2 point = wave_mode5_vertex(radius, angle, center, aspect);
        float dist = wave_safe_distance(uv, point);
        float contribution = wave_contribution(dist, 0.008);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
    }
};

class LineWaveRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE6_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = 1.57 + clamp(wave_mystery, -1.0, 1.0);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < segment_count; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1, wave_scale);
        vec2 p2 = wave_mode6_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2, wave_scale);
        float dist = wave_distance_to_segment(uv, p1, p2);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
    }
};

class DoubleLineWaveRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;
    float wave_scale = 0.25;

    int raw_samples = max(samples / 2, 2);
    int sample_count = min(raw_samples, MODE7_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = 1.57 * max(wave_mystery, 0.1);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    float separation = pow(clamp(wave_y * 0.5 + 0.5, 0.0, 1.0), 2.0);

    for (int i = 0; i < segment_count; ++i)
    {
        vec2 p1L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio.x, wave_scale, separation);
        vec2 p2L = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio.x, wave_scale, separation);
        float distL = wave_distance_to_segment(uv, p1L, p2L);
        float contributionL = wave_contribution(distL, 0.01);
        intensity += contributionL;

        vec2 p1R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i), audio.y, wave_scale, -separation);
        vec2 p2R = wave_mode7_vertex(edge_x, edge_y, distance_x, distance_y,
                                     perpendicular_dx, perpendicular_dy, float(i + 1), audio.y, wave_scale, -separation);
        float distR = wave_distance_to_segment(uv, p1R, p2R);
        float contributionR = wave_contribution(distR, 0.01);
        intensity += contributionR;

        if (wave_should_exit(i, contributionL + contributionR))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
    }
};

class SpectrumLineRenderer final : public WaveModeRenderer {
public:
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
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    vec2 audio = wave_clamp_audio(audio_data);
    float intensity = 0.0;

    int raw_samples = max(min(samples, 256), 2);
    int sample_count = min(raw_samples, MODE8_MAX_WAVE_ITERATIONS + 1);
    int segment_count = max(sample_count - 1, 1);

    float orientation = 1.57 * max(wave_mystery, 0.1);

    float edge_x;
    float edge_y;
    float distance_x;
    float distance_y;
    float perpendicular_dx;
    float perpendicular_dy;
    clip_waveform_edges(orientation, wave_x, wave_y, float(sample_count), edge_x, edge_y,
                        distance_x, distance_y, perpendicular_dx, perpendicular_dy);

    for (int i = 0; i < segment_count; ++i)
    {
        float displacement1 = (i % 2 == 0) ? audio.x : audio.y;
        float displacement2 = ((i + 1) % 2 == 0) ? audio.x : audio.y;
        vec2 p1 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i), displacement1);
        vec2 p2 = wave_mode8_vertex(edge_x, edge_y, distance_x, distance_y,
                                    perpendicular_dx, perpendicular_dy, float(i + 1), displacement2);
        float dist = wave_distance_to_segment(uv, p1, p2);
        float contribution = wave_contribution(dist, 0.01);
        intensity += contribution;
        if (wave_should_exit(i, contribution))
        {
            break;
        }
    }

    return intensity;
}
)___";
    }

    std::string callPattern() const override {
        return R"___(draw_wave(pixelUV, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery))___";
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

std::unique_ptr<WaveModeRenderer> WaveModeRenderer::create(int nWaveMode, const std::map<std::string, std::string>& /*presetValues*/)
{
    switch (nWaveMode)
    {
        case 0:
            return std::make_unique<CircleWaveRenderer>();
        case 2:
            return std::make_unique<CenteredSpiroRenderer>();
        case 3:
            return std::make_unique<CenteredSpiroVolumeRenderer>();
        case 4:
            return std::make_unique<DerivativeLineRenderer>();
        case 5:
            return std::make_unique<ExplosiveHashRenderer>();
        case 6:
            return std::make_unique<LineWaveRenderer>();
        case 7:
            return std::make_unique<DoubleLineWaveRenderer>();
        case 8:
            return std::make_unique<SpectrumLineRenderer>();
        default:
            return nullptr;
    }
}

std::string WaveModeRenderer::generateCommonHelpers()
{
    return R"___(
const float WAVE_EPSILON = 1e-5;
const float WAVE_INTENSITY_CUTOFF = 1e-4;
const float WAVE_DISTANCE_CLAMP = 8.0;
const float WAVE_MAX_ANGLE = 8192.0;
const float WAVE_TWO_PI = 6.28318530718;
const int WAVE_MIN_WARMUP_ITERATIONS = 4;

const int MODE0_MAX_WAVE_ITERATIONS = 48;
const int MODE2_MAX_WAVE_ITERATIONS = 48;
const int MODE3_MAX_WAVE_ITERATIONS = 48;
const int MODE4_MAX_WAVE_ITERATIONS = 64;
const int MODE5_MAX_WAVE_ITERATIONS = 48;
const int MODE6_MAX_WAVE_ITERATIONS = 64;
const int MODE7_MAX_WAVE_ITERATIONS = 48;
const int MODE8_MAX_WAVE_ITERATIONS = 64;

vec2 wave_aspect()
{
    return vec2(1.0, 1.0);
}

float wave_clamp_angle(float angle)
{
    return clamp(angle, -WAVE_MAX_ANGLE, WAVE_MAX_ANGLE);
}

float wave_safe_cos(float angle)
{
    return cos(wave_clamp_angle(angle));
}

float wave_safe_sin(float angle)
{
    return sin(wave_clamp_angle(angle));
}

vec2 wave_clamp_vec(vec2 value)
{
    return clamp(value, vec2(-WAVE_DISTANCE_CLAMP), vec2(WAVE_DISTANCE_CLAMP));
}

float wave_safe_length(vec2 value)
{
    vec2 clamped = wave_clamp_vec(value);
    return length(clamped);
}

float wave_safe_distance(vec2 a, vec2 b)
{
    return wave_safe_length(a - b);
}

float wave_contribution(float distance, float softness)
{
    float safeSoftness = max(softness, WAVE_EPSILON);
    float clampedDistance = clamp(distance, 0.0, WAVE_DISTANCE_CLAMP);
    return 1.0 - smoothstep(0.0, safeSoftness, clampedDistance);
}

float wave_distance_to_segment(vec2 p, vec2 v, vec2 w)
{
    vec2 clampedDiff = wave_clamp_vec(w - v);
    float l2 = max(dot(clampedDiff, clampedDiff), WAVE_EPSILON);
    vec2 clampedP = wave_clamp_vec(p - v);
    float t = clamp(dot(clampedP, clampedDiff) / l2, 0.0, 1.0);
    vec2 projection = v + clampedDiff * t;
    return wave_safe_distance(p, projection);
}

float wave_safe_divide(float numerator, float denominator)
{
    float denom = abs(denominator) < WAVE_EPSILON
        ? (denominator >= 0.0 ? WAVE_EPSILON : -WAVE_EPSILON)
        : denominator;
    return numerator / denom;
}

vec2 wave_clamp_audio(vec2 audio)
{
    return clamp(audio, vec2(-1.0), vec2(1.0));
}

bool wave_should_exit(int index, float contribution)
{
    return (index >= WAVE_MIN_WARMUP_ITERATIONS) && (contribution <= WAVE_INTENSITY_CUTOFF);
}

void clip_waveform_edges(float angle, float wave_x, float wave_y, float sample_count,
                         out float edge_x, out float edge_y,
                         out float distance_x, out float distance_y,
                         out float perpendicular_dx, out float perpendicular_dy)
{
    float safeAngle = wave_clamp_angle(angle);
    float orthoAngle = safeAngle + 1.57;
    vec2 direction = vec2(wave_safe_cos(safeAngle), wave_safe_sin(safeAngle));
    float orthoCos = wave_safe_cos(orthoAngle);
    float orthoSin = wave_safe_sin(orthoAngle);

    vec2 edge[2];
    edge[0] = wave_clamp_vec(vec2(wave_x * orthoCos - direction.x * 3.0,
                                  wave_y * orthoSin - direction.y * 3.0));
    edge[1] = wave_clamp_vec(vec2(wave_x * orthoCos + direction.x * 3.0,
                                  wave_y * orthoSin + direction.y * 3.0));

    for (int i = 0; i < 2; ++i)
    {
        for (int j = 0; j < 4; ++j)
        {
            float t = 0.0;
            bool clip = false;
            if (j == 0 && edge[i].x > 1.1)
            {
                t = wave_safe_divide(1.1 - edge[1 - i].x, edge[i].x - edge[1 - i].x);
                clip = true;
            }
            else if (j == 1 && edge[i].x < -1.1)
            {
                t = wave_safe_divide(-1.1 - edge[1 - i].x, edge[i].x - edge[1 - i].x);
                clip = true;
            }
            else if (j == 2 && edge[i].y > 1.1)
            {
                t = wave_safe_divide(1.1 - edge[1 - i].y, edge[i].y - edge[1 - i].y);
                clip = true;
            }
            else if (j == 3 && edge[i].y < -1.1)
            {
                t = wave_safe_divide(-1.1 - edge[1 - i].y, edge[i].y - edge[1 - i].y);
                clip = true;
            }

            if (clip)
            {
                t = clamp(t, 0.0, 1.0);
                vec2 diff = edge[i] - edge[1 - i];
                edge[i] = wave_clamp_vec(edge[1 - i] + diff * t);
            }
        }
    }

    vec2 diff = wave_clamp_vec(edge[1] - edge[0]);
    float inv_samples = 1.0 / max(sample_count, 1.0);
    vec2 delta = diff * inv_samples;

    edge_x = edge[0].x;
    edge_y = edge[0].y;
    distance_x = delta.x;
    distance_y = delta.y;

    float angle2 = atan(delta.y, delta.x);
    perpendicular_dx = wave_safe_cos(angle2 + 1.57);
    perpendicular_dy = wave_safe_sin(angle2 + 1.57);
}
)___";
}

std::string WaveModeRenderer::generateFallback()
{
    return R"___(
// Fallback waveform renderer when the mode is unsupported
float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery)
{
    (void)uv;
    (void)audio_data;
    (void)samples;
    (void)wave_x;
    (void)wave_y;
    (void)wave_mystery;
    return 0.0;
}
)___";
}
