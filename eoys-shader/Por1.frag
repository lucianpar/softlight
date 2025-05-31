#version 330 core

in vec3 vPos;
in vec2 vUV;
out vec4 fragColor;

uniform float u_time;

#define PI 3.14159265359

// --- Hash-based Noise ---
vec2 hash(vec2 x) {
    const vec2 k = vec2(0.3183099, 0.3678794);
    x = x * k + k.yx;
    return -1.0 + 2.0 * fract(16.0 * k * fract(x.x * x.y * (x.x + x.y)));
}

float noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    vec2 u = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(dot(hash(i + vec2(0.0, 0.0)), f),
            dot(hash(i + vec2(1.0, 0.0)), f - vec2(1.0, 0.0)), u.x),
        mix(dot(hash(i + vec2(0.0, 1.0)), f - vec2(0.0, 1.0)),
            dot(hash(i + vec2(1.0, 1.0)), f - vec2(1.0, 1.0)), u.x),
        u.y);
}

// --- Fractal Brownian Motion ---
float fbm(vec2 p) {
    float total = 0.0;
    float amp = 0.5;
    for (int i = 0; i < 4; i++) {
        total += noise(p) * amp;
        p *= 2.0;
        amp *= 0.5;
    }
    return total;
}

// --- Tendril Shape ---
float tendril(vec2 uv, float time, float phase) {
    uv.x += sin(uv.y * 0.3 + time * 0.4 + phase) * 0.3;
    uv.x += fbm(uv * 0.5 + phase + time * 0.1) * 0.5;

    float stripe = exp(-14.0 * uv.x * uv.x);
    return stripe;
}

void main() {
    vec2 uv = vPos.xy * 4.0; // normalized domain scaling
    float t = u_time;
    float glow = 0.0;

    const int tendrilCount = 4;
    const int smearSteps = 30;

    // Layer multiple tendrils with varying curl/noise
    for (int j = 0; j < tendrilCount; j++) {
        float phase = float(j) * 3.14 * 0.3 + sin(t * 0.2 + j);
        float xOffset = float(j - tendrilCount / 2) * 0.2;

        for (int i = 0; i < smearSteps; i++) {
            float offset = float(i) * 0.0025;
            vec2 p = vec2(uv.x + xOffset, uv.y - offset);
            glow += tendril(p, t, phase) / float(i + 1);
        }
    }

    // Ghostly magenta-cyan tint
    vec3 baseColor = vec3(0.3, 0.6, 0.9);
    vec3 highlight = vec3(0.9, 0.5, 0.8);
    vec3 col = glow * mix(baseColor, highlight, glow);

    fragColor = vec4(col, 1.0);
}
