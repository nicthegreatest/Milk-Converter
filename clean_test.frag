#version 330 core

out vec4 FragColor;
in vec2 uv;

float float_from_bool(bool b) { return b ? 1.0 : 0.0; }


float rand(vec2 co){
    return fract(sin(dot(co.xy ,vec2(12.9898,78.233))) * 43758.5453);
}

// Helper function to calculate the shortest distance from a point to a line segment.
float distance_to_line_segment(vec2 p, vec2 v, vec2 w) {
    float l2 = pow(distance(v, w), 2.0);
    if (l2 == 0.0) return distance(p, v);
    float t = max(0.0, min(1.0, dot(p - v, w - v) / l2));
    vec2 projection = v + t * (w - v);
    return distance(p, projection);
}

float draw_wave(vec2 uv, vec2 audio_data, int samples, float wave_x, float wave_y, float wave_mystery) {
    float line_intensity = 0.0;
    float wave_scale = 0.25; // wave_scale equivalent

    // These parameters are usually calculated in a ClipWaveformEdges function.
    // We replicate the core logic for nWaveMode=6 here.
    float angle = 1.57 + wave_mystery;
    float c = cos(angle);
    float s = sin(angle);

    // Simplified line definition based on wave_x, wave_y and angle
    vec2 start_point = vec2(wave_x - c*0.5, wave_y - s*0.5);
    vec2 end_point = vec2(wave_x + c*0.5, wave_y + s*0.5);
    vec2 direction = normalize(end_point - start_point);
    vec2 perpendicular = vec2(-direction.y, direction.x);


    for (int i = 0; i < samples - 1; i++) {
        float p1_idx = float(i) / float(samples);
        float p2_idx = float(i+1) / float(samples);

        // Use audio data to displace points.
        // We use two components of iAudioBands as a proxy for the stereo waveform.
        float displacement1 = (i % 2 == 0) ? audio_data.x : audio_data.y;
        float displacement2 = ((i+1) % 2 == 0) ? audio_data.x : audio_data.y;

        vec2 p1 = start_point + direction * p1_idx + perpendicular * displacement1 * wave_scale;
        vec2 p2 = start_point + direction * p2_idx + perpendicular * displacement2 * wave_scale;

        float dist = distance_to_line_segment(uv, p1, p2);
        line_intensity += (1.0 - smoothstep(0.0, 0.01, dist));
    }

    return line_intensity;
}

// Standard RaymarchVibe uniforms
uniform float iTime;
uniform vec2 iResolution;
uniform float iFps;
uniform float iFrame;
uniform float iProgress;
uniform vec4 iAudioBands;
uniform vec4 iAudioBandsAtt;
uniform sampler2D iChannel0; // Feedback buffer
uniform sampler2D iChannel1;
uniform sampler2D iChannel2;
uniform sampler2D iChannel3;

// Preset-specific uniforms with UI annotations
uniform float u_echo_alpha = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_mv_b = 1.000000; // {"widget":"slider","default":1.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_mv_dy = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_dx = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_x = 12.0; // {"widget":"slider","default":12.0,"min":0.0,"max":64.0,"step":1.0}
uniform float u_mv_y = 9.0; // {"widget":"slider","default":9.0,"min":0.0,"max":48.0,"step":1.0}
uniform float u_ib_b = 0.250000; // {"widget":"slider","default":0.250000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_g = 0.450000; // {"widget":"slider","default":0.450000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_echo_zoom = 1.0; // {"widget":"slider","default":1.0,"min":0.5,"max":2.0,"step":0.01}
uniform float u_wave_b = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_g = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_r = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_y = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sx = 1.000000; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_zoom = 1.000223; // {"widget":"slider","default":1.000223,"min":0.5,"max":1.5,"step":0.01}
uniform float u_cy = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sy = 1.000000; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_mv_l = 2.500000; // {"widget":"slider","default":2.500000,"min":0.0,"max":2.0,"step":0.01}
uniform float u_wave_x = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_cx = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_warp = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":2.0,"step":0.01}
uniform float u_zoomexp = 1.0; // {"widget":"slider","default":1.0,"min":0.5,"max":2.0,"step":0.01}
uniform float u_dy = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_g = 1.000000; // {"widget":"slider","default":1.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_rot = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_wave_a = 1.0; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_a = 1.0; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_g = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_decay = 0.98; // {"widget":"slider","default":0.98,"min":0.9,"max":1.0,"step":0.001}
uniform float u_r = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_a = 0.060000; // {"widget":"slider","default":0.060000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_dx = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_g = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_b = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_mystery = 0.0; // {"widget":"slider","default":0.0,"min":-1.0,"max":1.0,"step":0.01}
uniform float u_ob_size = 0.100000; // {"widget":"slider","default":0.100000,"min":0.0,"max":0.1,"step":0.001}
uniform float u_echo_orient = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":3.0,"step":1.0}
uniform float u_ob_r = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_mv_r = 0.060000; // {"widget":"slider","default":0.060000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_b = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_a = 0.290000; // {"widget":"slider","default":0.290000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_size = 0.035000; // {"widget":"slider","default":0.035000,"min":0.0,"max":0.1,"step":0.001}
uniform float u_mv_a = 0.200000; // {"widget":"slider","default":0.200000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_r = 0.250000; // {"widget":"slider","default":0.250000,"min":0.0,"max":1.0,"step":0.01}

void main() {
    // Initialize local variables from uniforms
    float echo_alpha = u_echo_alpha;
    float mv_b = u_mv_b;
    float mv_dy = u_mv_dy;
    float mv_dx = u_mv_dx;
    float mv_x = u_mv_x;
    float mv_y = u_mv_y;
    float ib_b = u_ib_b;
    float ib_g = u_ib_g;
    float echo_zoom = u_echo_zoom;
    float wave_b = u_wave_b;
    float wave_g = u_wave_g;
    float wave_r = u_wave_r;
    float wave_y = u_wave_y;
    float sx = u_sx;
    float zoom = u_zoom;
    float cy = u_cy;
    float sy = u_sy;
    float mv_l = u_mv_l;
    float wave_x = u_wave_x;
    float cx = u_cx;
    float warp = u_warp;
    float zoomexp = u_zoomexp;
    float dy = u_dy;
    float mv_g = u_mv_g;
    float rot = u_rot;
    float wave_a = u_wave_a;
    float a = u_a;
    float ob_g = u_ob_g;
    float decay = u_decay;
    float r = u_r;
    float ob_a = u_ob_a;
    float dx = u_dx;
    float g = u_g;
    float b = u_b;
    float wave_mystery = u_wave_mystery;
    float ob_size = u_ob_size;
    float echo_orient = u_echo_orient;
    float ob_r = u_ob_r;
    float mv_r = u_mv_r;
    float ob_b = u_ob_b;
    float ib_a = u_ib_a;
    float ib_size = u_ib_size;
    float mv_a = u_mv_a;
    float ib_r = u_ib_r;

    // State variables
    float q1 = 0.0;
    float q2 = 0.0;
    float q3 = 0.0;
    float q4 = 0.0;
    float q5 = 0.0;
    float q6 = 0.0;
    float q7 = 0.0;
    float q8 = 0.0;
    float q9 = 0.0;
    float q10 = 0.0;
    float q11 = 0.0;
    float q12 = 0.0;
    float q13 = 0.0;
    float q14 = 0.0;
    float q15 = 0.0;
    float q16 = 0.0;
    float q17 = 0.0;
    float q18 = 0.0;
    float q19 = 0.0;
    float q20 = 0.0;
    float q21 = 0.0;
    float q22 = 0.0;
    float q23 = 0.0;
    float q24 = 0.0;
    float q25 = 0.0;
    float q26 = 0.0;
    float q27 = 0.0;
    float q28 = 0.0;
    float q29 = 0.0;
    float q30 = 0.0;
    float q31 = 0.0;
    float q32 = 0.0;
    float t1 = 0.0;
    float t2 = 0.0;
    float t3 = 0.0;
    float t4 = 0.0;
    float t5 = 0.0;
    float t6 = 0.0;
    float t7 = 0.0;
    float t8 = 0.0;
    float bccl = 0.0;
    float beat = 0.0;
    float beatfreq = 0.0;
    float bt = 0.0;
    float btblock = 0.0;
    float dle = 0.0;
    float dqv = 0.0;
    float eo = 0.0;
    float hccp = 0.0;
    float lastbeat = 0.0;
    float lastpulse = 0.0;
    float le = 0.0;
    float leccl = 0.0;
    float pulse = 0.0;
    float pulsefreq = 0.0;
    float th = 0.0;
    float thccl = 0.0;

    // Per-frame logic
    le = (((1.4 * iAudioBandsAtt.x) + (0.1 * iAudioBands.x)) + (0.5 * iAudioBands.z));
    pulse = float_from_bool((le > th));
    pulsefreq = (((pulsefreq == 0.0)) ? (2.0) : (((pulse != 0.0) ? (((0.8 * pulsefreq) + (0.2 * (iTime - lastpulse)))) : (pulsefreq))));
    lastpulse = ((pulse != 0.0) ? (iTime) : (lastpulse));
    bt = ((iTime - lastbeat) / ((0.5 * beatfreq) + (0.5 * pulsefreq)));
    hccp = ((0.03 / (bt + 0.2)) + (0.5 * (((float_from_bool((bt > 0.8)) != 0.0) && (float_from_bool((bt < 1.2)) != 0.0)) ? ((pow(sin(((bt - 1.0) * 7.854)), 4.0) - 1.0)) : (0.0))));
    beat = float_from_bool((float_from_bool((le > (th + hccp))) != 0.0) && (btblock != 0.0));
    btblock = (1.0 - float_from_bool((le > (th + hccp))));
    lastbeat = ((beat != 0.0) ? (iTime) : (lastbeat));
    beatfreq = (((beatfreq == 0.0)) ? (2.0) : (((beat != 0.0) ? (((0.8 * beatfreq) + (0.2 * (iTime - lastbeat)))) : (beatfreq))));
    th = (((le > th)) ? (((le + (114.0 / (le + 10.0))) - 7.407)) : (((th + ((th * 0.07) / (th - 12.0))) + ((float_from_bool((th < 2.7)) * 0.1) * (2.7 - th)))));
    th = (((th > 6.0)) ? (6.0) : (th));
    thccl = (thccl + (th - 2.5144));
    q1 = le;
    q2 = (thccl + (0.2 * leccl));
    leccl = (leccl + (dle * le));
    dle = ((beat != 0.0) ? ((dle)) : (dle));
    bccl = (bccl + beat);
    wave_r = ((0.1 + (0.8 * ((sin((0.011 * thccl)))*(sin((0.011 * thccl)))))) + (0.1 * sin((leccl * 0.061))));
    wave_g = ((0.1 + (0.8 * ((sin((0.013 * thccl)))*(sin((0.013 * thccl)))))) + (0.1 * cos((leccl * 0.067))));
    wave_b = ((0.1 + (0.8 * ((cos((0.017 * thccl)))*(cos((0.017 * thccl)))))) + (0.1 * sin((leccl * 0.065))));
    ib_r = (ib_r + (0.1 * sin(((1.3 * iTime) + (0.012 * leccl)))));
    ib_g = (ib_g + (0.1 * sin(((1.7 * iTime) + (0.019 * leccl)))));
    ib_b = (ib_b + (0.1 * sin(((1.9 * iTime) + (0.017 * leccl)))));
    mv_r = (0.5 * (ib_r + wave_r));
    mv_g = (0.5 * (ib_g + wave_g));
    mv_b = (0.5 * (ib_b + wave_b));
    mv_a = (0.5 * ((sin(((0.01 * leccl) + bccl)))*(sin(((0.01 * leccl) + bccl)))));
    echo_alpha = (0.5 + (0.2 * cos(((0.07 * leccl) + (0.02 * thccl)))));
    eo = (((float_from_bool((mod(bccl, 3.0) == 0.0)) != 0.0) && (beat != 0.0)) ? ((rand(uv) * 4.0)) : (eo));
    q3 = ((float_from_bool((eo == 2.0)) + float_from_bool((eo == 1.0))) * float_from_bool((mod(bccl, 2.0) == 0.0)));
    q4 = ((float_from_bool((eo == 0.0)) + float_from_bool((eo == 3.0))) * float_from_bool((mod(bccl, 2.0) == 0.0)));
    echo_orient = eo;

    // Per-pixel logic
    dqv = (float_from_bool((uv.x > 0.5)) - float_from_bool((uv.y > 0.5)));
    rot = sin(((sin(((length(uv - vec2(0.5)) * (13.0 + (5.0 * sin((0.01 * q2))))) + (0.06 * q2))) * q1) * 0.01));
    zoom = (1.0 + ((((q3 != 0.0) ? (dqv) : (1.0)) * 0.1) * sin(((7.0 * atan(uv.y - 0.5, uv.x - 0.5)) + (0.03 * q2)))));
    zoom = ((q4 != 0.0) ? ((((length(uv - vec2(0.5)) < (0.8 * ((sin((0.016 * q2)))*(sin((0.016 * q2))))))) ? ((0.75 + (0.4 * cos((0.021 * q2))))) : (zoom))) : (zoom));

    // Apply coordinate transformations calculated in per-pixel logic.
    // This emulates the 'warp' part of a MilkDrop shader.
    vec2 transformed_uv = uv - vec2(cx, cy); // Center on cx, cy

    mat2 rotation_matrix = mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
    transformed_uv = rotation_matrix * transformed_uv;

    transformed_uv *= warp;
    transformed_uv /= zoom;
    transformed_uv /= vec2(sx, sy);

    transformed_uv += vec2(dx, dy); // Pan
    transformed_uv += vec2(cx, cy); // Un-center


    // Final color composition
    // Sample the previous frame's output (feedback buffer) with warped UVs.
vec2 clamped_uv = clamp(transformed_uv, 0.0, 1.0);
vec4 feedback = texture(iChannel0, clamped_uv);

    // Apply decay, which is essential for the classic MilkDrop fade effect.
    feedback.rgb *= decay;

    // The 'ob_' variables are for the outer border. We'll use them to tint the feedback color.
    vec4 border_color = vec4(ob_r, ob_g, ob_b, ob_a);
    FragColor = mix(feedback, border_color, border_color.a);

    // Additive blending for waves and shapes
    vec4 wave_color = vec4(wave_r, wave_g, wave_b, wave_a);
    float wave_intensity = draw_wave(uv, iAudioBands.xy, 128, wave_x, wave_y, wave_mystery) + draw_wave(uv, iAudioBands.zw, 128, wave_x, wave_y, wave_mystery);
    FragColor = mix(FragColor, wave_color, wave_intensity * wave_a);
}
