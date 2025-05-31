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

// === Morphing metaball field ===
float metaballField(vec2 uv, float t) {
    float field = 0.0;
    for (int i = 0; i < 7; ++i) {
        float fi = float(i);
        vec2 center = 1.2 * vec2(
            sin(t * 0.27 + fi * 1.3) + cos(fi + t * 0.11),
            cos(t * 0.19 + fi * 1.6) + sin(fi * 0.4 + t * 0.29)
        );
        float radius = 0.35 + 0.25 * sin(t * 0.23 + fi * 0.8);
        float dist = length(uv - center) + 0.003;
        field += radius / dist;
    }
    return field;
}

void main() {
    vec2 uv = vPos.xy / 7.0;  // Normalize mesh coords

    float t = u_time + flux * 0.4;

    // === Metaball field & exaggerated warp ===
    float meta = metaballField(uv, t);
    float warp = smoothstep(0.6, 3.5, meta);
    float pulse = 1.0 + 0.2 * sin(t * 0.8 + meta * 2.0); // animate zone size

    vec2 drift = vec2(
        sin(meta * 1.5 + t * 0.6),
        cos(meta * 1.7 + t * 0.5)
    );
    vec2 warpedUV = uv + 0.33 * warp * pulse * drift;

    // === FBM ripple across UV ===
    warpedUV += 0.05 * vec2(
        sin(fbm(vec3(uv, t * 0.13)) * 8.0 + t),
        cos(fbm(vec3(uv.yx, t * 0.11)) * 6.0 - t)
    );

    // === FBM noise field ===
    float n = fbm(vec3(warpedUV * 0.95, t * 0.07));

    // === Surface normals (approximate) ===
    vec3 normal = normalize(vec3(
        n - fbm(vec3(warpedUV + vec2(0.015, 0.0), t * 0.07)),
        n - fbm(vec3(warpedUV + vec2(0.0, 0.015), t * 0.07)),
        0.3 + (u_time / 140.0)
    ));

    // === Colors: blue ↔ faint blood red ===
    vec3 blue = vec3(0.1, 0.4, 0.9);
    vec3 blood = vec3(0.6, 0.1, 0.15);
    float blendFactor = 0.5 + 0.5 * sin(t * 0.9 + meta * 2.5);
    vec3 baseColor = mix(blue, blood, blendFactor);

    // === Lighting ===
    vec3 light1 = normalize(vec3(0.4, 0.8, 1.0));
    vec3 light2 = normalize(vec3(-0.3, -0.5, 1.0));
    vec3 color = vec3(0.0);
    color += 0.3 * pow(dot(normal, light1), 2.5);
    color += 0.2 * pow(dot(normal, light2), 4.5);
    color += baseColor * n;

    // === Final modulation ===
    color *= n * warp;
    color += 0.02 * vec3(cos(t), 0.0, sin(t * 0.3));

    // === Output ===
    fragColor = vec4(color, 0.12 + 0.4 * warp);
}
