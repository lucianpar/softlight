#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

#define TWO_PI 6.283

// === FBM / Gyroid Noise ===
float gyroid(vec3 seed) {
    return dot(sin(seed), cos(seed.yzx));
}

float fbm(vec3 seed) {
    float result = 0.0, a = 0.5;
    for (int i = 0; i < 5; ++i) {
        seed.z += result * 0.5;
        result += abs(gyroid(seed / a)) * a;
        a *= 0.5;
    }
    return result;
}

// === Osmotic puff field ===
float smokePuff(vec2 uv, float t, int id) {
    float fi = float(id);
    vec2 center = vec2(
        sin(t * 0.25 + fi) * 0.7 + cos(t * 0.3 + fi * 1.3) * 0.5,
        cos(t * 0.22 + fi * 1.2) * 0.6 + sin(t * 0.28 + fi) * 0.4
    );
    float r = 0.15 + 0.05 * sin(t * 0.5 + fi);
    float d = length(uv - center);
    float falloff = smoothstep(r * 1.8, r * 0.6, d);

    // Add local turbulence
    float wobble = fbm(vec3(uv * 2.5 + fi * 0.7, t * 0.1));
    return falloff * (0.8 + 0.4 * wobble);
}

void main() {
    vec2 uv = vPos.xy / 10.0;
    float t = u_time + flux * 0.2;

    // Slight drift of entire field
    uv += 0.1 * vec2(sin(t * 0.2), cos(t * 0.25));

    float total = 0.0;

    // Accumulate multiple small smoke apparitions
    for (int i = 0; i < 6; ++i) {
        total += smokePuff(uv, t, i);
    }

    // Osmosis-style shifting
    vec2 drift = vec2(sin(total + t * 0.4), cos(total + t * 0.3));
    uv += 0.05 * total * drift;

    // Final detail noise
    float n = fbm(vec3(uv * 1.5, t * 0.07));
    total *= n;

    // Color and output
    vec3 coolSmoke = vec3(0.15, 0.2, 0.3);
    vec3 warmCore = vec3(0.5, 0.1, 0.2);
    vec3 color = mix(coolSmoke, warmCore, 0.5 + 0.5 * sin(t + total * 2.0));

    float alpha = 0.05 + 0.3 * total;

    fragColor = vec4(color * total, alpha);
}
