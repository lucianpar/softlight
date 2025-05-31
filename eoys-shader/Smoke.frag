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

// === Organism Field (Core Visual) ===
float organismPresence(vec2 uv, float t) {
    vec2 center = vec2(0.2 * sin(t * 0.3), 0.2 * cos(t * 0.2));
    float radius = 0.45 + 0.15 * sin(t * 0.7);
    float d = length(uv - center);
    return smoothstep(radius * 1.2, radius, d);
}

// === Internal Membrane Noise (for 2nd style) ===
float internalMembrane(vec2 uv, float t) {
    float pulse = sin(t * 0.6 + uv.x * 5.0) * cos(t * 0.4 + uv.y * 7.0);
    float band = smoothstep(0.2, 0.5, pulse);
    float fb = fbm(vec3(uv * 3.0, t * 0.1));
    return mix(0.0, fb, band);
}

void main() {
    vec2 uv = vPos.xy / 7.0;
    float t = u_time + flux * 0.4;

    float transitionTime = 10.0;
    float mixFactor = smoothstep(transitionTime, transitionTime + 2.0, u_time);  // Crossfade from 10s–12s

    // === Common drift for both styles ===
    vec2 drift = vec2(
        sin(uv.y * 1.5 + t * 0.3),
        cos(uv.x * 1.3 - t * 0.25)
    );
    uv += 0.15 * drift * fbm(vec3(uv * 1.5, t * 0.1));

    // === ORGANISM BASE SHADER ===
    float n1 = fbm(vec3(uv * 1.2, t * 0.07 + onset * 0.001));
    float core1 = organismPresence(uv, t);

    vec3 normal1 = normalize(vec3(
        n1 - fbm(vec3(uv + vec2(0.01, 0.0), t * 0.07)),
        n1 - fbm(vec3(uv + vec2(0.0, 0.01), t * 0.07)),
        0.3 + (u_time / 140.0)
    ));

    vec3 blue = vec3(0.1, 0.4, 0.9);
    vec3 blood = vec3(0.6, 0.1, 0.15);
    float blendFactor = 0.5 + 0.5 * sin(t * 0.9 + n1 * 2.5);
    vec3 baseColor = mix(blue, blood, blendFactor);

    vec3 light1 = normalize(vec3(0.4, 0.8, 1.0));
    vec3 light2 = normalize(vec3(-0.3, -0.5, 1.0));
    vec3 color1 = vec3(0.0);
    color1 += 0.3 * pow(dot(normal1, light1), 2.5);
    color1 += 0.2 * pow(dot(normal1, light2), 4.5);
    color1 += baseColor * n1 * core1;
    color1 *= n1 * core1;
    color1 += 0.02 * vec3(cos(t), 0.0, sin(t * 0.3));

    float alpha1 = 0.12 + 0.4 * core1 * n1;

    // === ORGANISM + INTERNAL MEMBRANE SHADER ===
    float n2 = n1;
    float core2 = core1;
    float inner = internalMembrane(uv, t) * core2;

    vec3 color2 = color1 + vec3(0.2, 0.05, 0.1) * inner * 0.4;
    float alpha2 = alpha1 + 0.2 * inner;

    // === FINAL CROSSFADE ===
    vec3 finalColor = mix(color1, color2, mixFactor);
    float finalAlpha = mix(alpha1, alpha2, mixFactor);

    fragColor = vec4(finalColor, finalAlpha);
}
