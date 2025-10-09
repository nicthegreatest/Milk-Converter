#version 330 core

out vec4 FragColor;
in vec2 uv;

float float_from_bool(bool b) { return b ? 1.0 : 0.0; }

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
uniform float u_b; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_g; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_r; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_decay; // {"widget":"slider","default":0.98,"min":0.9,"max":1.0,"step":0.001}
uniform float u_a; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_mystery; // {"widget":"slider","default":0.0,"min":-1.0,"max":1.0,"step":0.01}
uniform float u_wave_a; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_rot; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_dy; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_zoomexp; // {"widget":"slider","default":1.0,"min":0.5,"max":2.0,"step":0.01}
uniform float u_warp; // {"widget":"slider","default":0.010000,"min":0.0,"max":2.0,"step":0.01}
uniform float u_wave_x; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sy; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_cx; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_dx; // {"widget":"slider","default":0.000000,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_cy; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sx; // {"widget":"slider","default":1.000000,"min":0.5,"max":1.5,"step":0.01}
uniform float u_wave_y; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_zoom; // {"widget":"slider","default":1.001600,"min":0.5,"max":1.5,"step":0.01}
uniform float u_wave_r; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_g; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_b; // {"widget":"slider","default":0.500000,"min":0.0,"max":1.0,"step":0.01}

void main() {
    // Initialize local variables from uniforms
    float b = u_b;
    float g = u_g;
    float r = u_r;
    float decay = u_decay;
    float a = u_a;
    float wave_mystery = u_wave_mystery;
    float wave_a = u_wave_a;
    float rot = u_rot;
    float dy = u_dy;
    float zoomexp = u_zoomexp;
    float warp = u_warp;
    float wave_x = u_wave_x;
    float sy = u_sy;
    float cx = u_cx;
    float dx = u_dx;
    float cy = u_cy;
    float sx = u_sx;
    float wave_y = u_wave_y;
    float zoom = u_zoom;
    float wave_r = u_wave_r;
    float wave_g = u_wave_g;
    float wave_b = u_wave_b;

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
    float ib_size = 0.0;
    float le = 0.0;
    float mblock = 0.0;
    float mod_state = 0.0;
    float mpulse = 0.0;
    float mres = 0.0;
    float mth = 0.0;
    float ob_a = 0.0;
    float ob_b = 0.0;
    float ob_g = 0.0;
    float ob_r = 0.0;
    float ob_size = 0.0;
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
    // A full implementation would draw waves and borders, but this is a good approximation.
    vec4 border_color = vec4(ob_r, ob_g, ob_b, ob_a);
    FragColor = mix(feedback, border_color, border_color.a);
}
