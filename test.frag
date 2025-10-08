#version 330 core

out vec4 FragColor;
in vec2 uv;

// Standard RaymarchVibe uniforms
uniform float iTime;
uniform vec2 iResolution;
uniform float iFps;
uniform float iFrame;
uniform float iProgress;
uniform vec4 iAudioBands;
uniform vec4 iAudioBandsAtt;

// Preset-specific uniforms with UI annotations
uniform float u_b; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_g; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_r; // {"widget":"slider","default":0.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_decay; // {"widget":"slider","default":0.98,"min":0.9,"max":1.0,"step":0.001}
uniform float u_a; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_mystery; // {"widget":"slider","default":0.0,"min":-1.0,"max":1.0,"step":0.01}
uniform float u_wave_a; // {"widget":"slider","default":1.0,"min":0.0,"max":1.0,"step":0.01}
uniform float u_rot; // {"widget":"slider","default":0.0,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_dy; // {"widget":"slider","default":0.0,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_zoomexp; // {"widget":"slider","default":1.0,"min":0.5,"max":2.0,"step":0.01}
uniform float u_warp; // {"widget":"slider","default":1.0,"min":0.0,"max":2.0,"step":0.01}
uniform float u_wave_x; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sy; // {"widget":"slider","default":1.0,"min":0.5,"max":1.5,"step":0.01}
uniform float u_cx; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_dx; // {"widget":"slider","default":0.0,"min":-0.1,"max":0.1,"step":0.001}
uniform float u_cy; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_sx; // {"widget":"slider","default":1.0,"min":0.5,"max":1.5,"step":0.01}
uniform float u_wave_y; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_zoom; // {"widget":"slider","default":1.0,"min":0.5,"max":1.5,"step":0.01}
uniform float u_wave_r; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_g; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}
uniform float u_wave_b; // {"widget":"slider","default":0.5,"min":0.0,"max":1.0,"step":0.01}

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
    float myvar = 0.0;

    // Per-frame logic
    wave_r = 0.5.0 + 0.5.0*sin(iTime);
    wave_g = 0.5.0 + 0.5.0*cos(iTime);
    wave_b = 0.5.0;

    // Per-pixel logic
    myvar = iAudioBands.uv.x*2.0;
    r = myvar * uv.x;
    g = myvar * uv.y;
    b = 0.0;

    FragColor = vec4(r, g, b, a);
}
