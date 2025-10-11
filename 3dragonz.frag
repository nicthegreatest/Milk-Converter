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
uniform float u_mv_b = 0.499900; // {"widget":"slider","default":0.499900,"min":0.0,"max":1.0,"step":0.01}
uniform float u_mv_dy = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_dx = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_x = 12.0; // {"widget":"slider","default":12.0,"min":0.0,"max":64.0,"step":1.0}
uniform float u_mv_y = 9.0; // {"widget":"slider","default":9.0,"min":0.0,"max":48.0,"step":1.0}
uniform float u_ib_b = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_g = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_b = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_g = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_r = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_y = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sx = 1.000000; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_zoom = 1.001600; // {"widget":"slider","default":1.001600,"min":0.5,"max":1.5,"step":0.01}
uniform float u_cy = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sy = 1.000000; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_mv_l = 0.850000; // {"widget":"slider","default":0.850000,"min":0.0,"max":2.0,"step":0.01}
uniform float u_wave_x = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_cx = 0.500000; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_warp = 0.010000; // {"widget":"slider","default":0.010000,"min":0.0,"max":2.0,"step":0.01}
uniform float u_zoomexp = 1.0; // {"widget":"slider","default":1.0,"min":0.5,"max":2.0,"step":0.01}
uniform float u_dy = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_mv_g = 0.499900; // {"widget":"slider","default":0.499900,"min":0.0,"max":1.0,"step":0.01}
uniform float u_rot = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_wave_a = 1.0; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_a = 1.0; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_g = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_decay = 0.98; // {"widget":"slider","default":0.98,"min":0.9,"max":1.0,"step":0.001}
uniform float u_r = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_a = 1.000000; // {"widget":"slider","default":1.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_dx = 0.000000; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_g = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_b = 0.0; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_mystery = 0.0; // {"widget":"slider","default":0.0,"min":-1.0,"max":1.0,"step":0.01}
uniform float u_ob_size = 0.005000; // {"widget":"slider","default":0.005000,"min":0.0,"max":0.1,"step":0.001}
uniform float u_ob_r = 1.000000; // {"widget":"slider","default":1.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_mv_r = 0.499900; // {"widget":"slider","default":0.499900,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ob_b = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_a = 0.900000; // {"widget":"slider","default":0.900000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_size = 0.005000; // {"widget":"slider","default":0.005000,"min":0.0,"max":0.1,"step":0.001}
uniform float u_mv_a = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_ib_r = 0.000000; // {"widget":"slider","default":0.000000,"min":0.0,"max":1.0,"step":0.01}

void main() {
    // Initialize local variables from uniforms
    float mv_b = u_mv_b;
    float mv_dy = u_mv_dy;
    float mv_dx = u_mv_dx;
    float mv_x = u_mv_x;
    float mv_y = u_mv_y;
    float ib_b = u_ib_b;
    float ib_g = u_ib_g;
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
    float bblock = 0.0;
    float bpulse = 0.0;
    float bres = 0.0;
    float bth = 0.0;
    float ccl = 0.0;
    float grid = 0.0;
    float le = 0.0;
    float mblock = 0.0;
    float mod_state = 0.0;
    float mpulse = 0.0;
    float mres = 0.0;
    float mth = 0.0;
    float pulse = 0.0;
    float snee = 0.0;
    float snur = 0.0;
    float tblock = 0.0;
    float tpulse = 0.0;
    float tres = 0.0;
    float tth = 0.0;

    // Per-frame logic
    warp = 0.0;
    le = (1.5 + (2.0 * sin(iAudioBandsAtt.x)));
    bpulse = float_from_bool((float_from_bool((le > bth)) != 0.0) && (float_from_bool(((le - bth) > bblock)) != 0.0));
    bblock = (le - bth);
    bth = (((le > bth)) ? (((le + (114.0 / (le + 10.0))) - 7.407)) : (((bth + ((bth * 0.07) / (bth - 12.0))) + ((float_from_bool((bth < 2.7)) * 0.1) * (2.7 - bth)))));
    bth = (((bth > 6.0)) ? (6.0) : (bth));
    bres = ((bpulse * sin((pulse + (le * 0.5)))) + (float_from_bool(bpulse == 0.0) * bres));
    le = (1.5 + (2.0 * sin(iAudioBandsAtt.z)));
    tpulse = float_from_bool((float_from_bool((le > tth)) != 0.0) && (float_from_bool(((le - tth) > tblock)) != 0.0));
    tblock = (le - tth);
    tth = (((le > tth)) ? (((le + (114.0 / (le + 10.0))) - 7.407)) : (((tth + ((tth * 0.07) / (tth - 12.0))) + ((float_from_bool((tth < 2.7)) * 0.1) * (2.7 - tth)))));
    tth = (((tth > 6.0)) ? (6.0) : (tth));
    tres = ((tpulse * sin((pulse + (le * 0.5)))) + (float_from_bool(tpulse == 0.0) * tres));
    le = (1.5 + (2.0 * sin(iAudioBandsAtt.y)));
    mpulse = float_from_bool((float_from_bool((le > mth)) != 0.0) && (float_from_bool(((le - mth) > mblock)) != 0.0));
    mblock = (le - mth);
    mth = (((le > mth)) ? (((le + (114.0 / (le + 10.0))) - 7.407)) : (((mth + ((mth * 0.07) / (mth - 12.0))) + ((float_from_bool((mth < 2.7)) * 0.1) * (2.7 - mth)))));
    mth = (((mth > 6.0)) ? (6.0) : (mth));
    mres = ((mpulse * sin((pulse + (le * 0.5)))) + (float_from_bool(mpulse == 0.0) * mres));
    pulse = (((abs(pulse) > 3.14)) ? (-3.14) : ((pulse + (((bth + mth) + tth) * 0.003))));
    q1 = bres;
    q2 = tres;
    q3 = mres;
    q4 = sin(pulse);
    mod_state = (((float_from_bool((q1 > 0.0)) + float_from_bool((q2 > 0.0))) + float_from_bool((q3 > 0.0))) * (1.0 + float_from_bool((q4 > 0.0))));
    ccl = (((ccl + tpulse) + mpulse) - bpulse);
    q5 = cos((pulse * (0.5 + (0.1 * mod_state))));
    q6 = sin((pulse * (0.5 + pow(0.25, mod_state))));
    q7 = mod_state;
    q8 = ccl;
    ob_r = (0.5 + (0.5 * cos((q1 + q7))));
    ob_g = (0.5 + (0.5 * cos(((q2 * 3.14) + q7))));
    ob_b = (0.5 + (0.5 * cos(((q3 * 2.0) + sin((iTime * 0.0816))))));
    ib_size = (0.025 + (0.02 * q2));
    ob_size = ((0.03 + (0.02 * q3)) - (0.002 * q7));
    wave_r = (0.5 + (0.5 * sin(((q1 * q7) + (iTime * 2.183)))));
    wave_g = (0.5 + (0.5 * sin(((q2 * 3.0) + (iTime * 1.211)))));
    wave_b = (0.5 + (0.5 * sin((q3 + (iTime * 1.541)))));
    decay = ((0.997 + (0.0015 * q3)) + (0.0015 * q1));
    ob_a = (0.8 + (0.2 * q2));

    // Per-pixel logic
    snee = float_from_bool(((float_from_bool(((sin(atan(uv.y - 0.5, uv.x - 0.5)) - uv.x) > 0.5)) * float_from_bool((q2 > 0.0))) + (float_from_bool(((uv.y - cos(atan(uv.y - 0.5, uv.x - 0.5))) > 0.5)) * float_from_bool((q1 > 0.0)))) == 0.0);
    snur = float_from_bool(((float_from_bool((uv.x < 0.5)) * float_from_bool((q3 > 0.0))) + (float_from_bool((uv.y < 0.5)) * float_from_bool((q7 < 4.0)))) == 0.0);
    grid = sin(((sin(((uv.y * 6.28) * q2)), sin(((uv.x * 6.28) * q6))) * (10.0 + q7)));
    rot = (((float_from_bool((float_from_bool((uv.x > 0.5)) + (uv.y * 0.0)) == 0.0) * cos((length(uv - vec2(0.5)) + ((3.14 * (((grid > 0.0)) ? (snur) : (float_from_bool(snur == 0.0)))) * (0.5 + (0.5 * sin(((length(uv - vec2(0.5)) * 3.14) * q1)))))))) * q6) * 0.3);
    sx = (sx - ((float_from_bool((snee != 0.0) || (snur != 0.0)) * cos((uv.y * q2))) * 0.05));
    sy = (sy - ((float_from_bool((float_from_bool(snee == 0.0) != 0.0) || (snur != 0.0)) * cos((uv.x * q1))) * 0.03));
    cx = (cx + ((sin(((uv.x * 3.14) * q6)) * snur) * 0.45));
    cy = (cx + ((sin(((uv.y * 3.14) * q5)) * snee) * 0.5));
    zoom = ((zoom + ((0.1 * snee) * float_from_bool(snur == 0.0))) - ((0.1 * snur) * float_from_bool(snee == 0.0)));

    // Apply coordinate transformations calculated in per-pixel logic.
    // This emulates the 'warp' part of a MilkDrop shader.
    vec2 transformed_uv = uv - vec2(cx, cy); // Center on cx, cy

    mat2 rotation_matrix = mat2(cos(rot), -sin(rot), sin(rot), cos(rot));
    transformed_uv = rotation_matrix * transformed_uv;

    transformed_uv /= zoom;
    transformed_uv /= vec2(sx, sy);

    transformed_uv += vec2(dx, dy); // Pan
    transformed_uv += vec2(cx, cy); // Un-center


    // Final color composition
    // Sample the previous frame's output (feedback buffer) with warped UVs.
    vec4 feedback = texture(iChannel0, transformed_uv);

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
