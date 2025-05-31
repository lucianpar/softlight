#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

#define TWO_PI 6.283

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

// One-sided falloff mask (from left to right)
float engulfMask(vec2 uv, float t) {
    // dynamic threshold that moves inward over time
    float edge = 1.2 - 0.4 * sin(t * 0.2) - 0.5 * smoothstep(0.0, 15.0, t);
    float softness = 0.3 + 0.1 * sin(t * 0.5);
    return smoothstep(edge + softness, edge, uv.x);
}

void main() {
    vec2 uv = vPos.xy / 30.0;
    float t = u_time + flux * 0.3;

    // Drifted UV
    vec2 drift = vec2(
        sin(uv.y * 1.5 + t * 0.3),
        cos(uv.x * 1.3 - t * 0.25)
    );
    uv += 0.25 * drift * fbm(vec3(uv * 1.5, t * 0.1));

    // Smoke field
    float smoke = fbm(vec3(uv * 2.5, t * 0.08)) * 1.2;
    float mask = engulfMask(uv, t);

    // Enhance contrast and dynamics
    float intensity = pow(smoke, 2.0) * mask;

    // Color palette: smoky blue-gray to soft crimson
    vec3 blueSmoke = vec3(0.2, 0.3, 0.4);
    vec3 emberRed = vec3(0.4, 0.1, 0.1);
    vec3 color = mix(blueSmoke, emberRed, 0.5 + 0.5 * sin(t * 0.15 + smoke * 4.0));

    fragColor = vec4(color * intensity, 0.05 + 0.5 * intensity);
}
