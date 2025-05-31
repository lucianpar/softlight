#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

#define TWO_PI 6.283

// Utility Functions
float gyroid(vec3 seed) {
    return dot(sin(seed), cos(seed.yzx));
}

float fbm(vec3 seed) {
    float result = 0.0;
    float a = 0.5;
    for (int i = 0; i < 5; ++i) {
        seed.z += result * 0.5;
        result += abs(gyroid(seed / a)) * a;
        a *= 0.5;
    }
    return result;
}

float speedEnvelope(float t) {
    if (t < 300.0) {
        return mix(0.2, 2.5, t / 300.0);
    } else {
        return mix(2.5, 0.01, (t - 300.0) / 191.0);
    }
}

// Shader 1: Smoke Organism
float organismPresence(vec2 uv, float t) {
    vec2 center = vec2(0.2 * sin(t * 0.3), 0.2 * cos(t * 0.2));
    float radius = 0.45 + 0.15 * sin(t * 0.7);
    float d = length(uv - center);
    return smoothstep(radius * 1.2, radius, d);
}

float internalMembrane(vec2 uv, float t) {
    float pulse = sin(t * 0.6 + uv.x * 5.0) * cos(t * 0.4 + uv.y * 7.0);
    float band = smoothstep(0.2, 0.5, pulse);
    float fb = fbm(vec3(uv * 3.0, t * 0.1));
    return mix(0.0, fb, band);
}

vec4 shaderSmoke1(vec2 uv, float t) {
    uv = vPos.xy / 15.0;
    float rate = speedEnvelope(t);
    float time = t * rate;

    uv += 0.15 * vec2(sin(uv.y * 1.5 + time * 0.3), cos(uv.x * 1.3 - time * 0.25)) * fbm(vec3(uv * 1.5, time * 0.1));
    float n1 = fbm(vec3(uv * 1.2, time * 0.07 + onset * 0.001));
    float core1 = organismPresence(uv, time);

    vec3 normal1 = normalize(vec3(
        n1 - fbm(vec3(uv + vec2(0.01, 0.0), time * 0.07)),
        n1 - fbm(vec3(uv + vec2(0.0, 0.01), time * 0.07)),
        0.3 + (time / 140.0)
    ));

    vec3 baseColor = mix(vec3(0.1, 0.4, 0.9), vec3(0.6, 0.1, 0.15), 0.5 + 0.5 * sin(time * 0.9 + n1 * 2.5));

    vec3 light1 = normalize(vec3(0.4, 0.8, 1.0));
    vec3 light2 = normalize(vec3(-0.3, -0.5, 1.0));

    vec3 color1 = vec3(0.0);
    color1 += 0.3 * pow(dot(normal1, light1), 2.5);
    color1 += 0.2 * pow(dot(normal1, light2), 4.5);
    color1 += baseColor * n1 * core1;
    color1 *= n1 * core1;
    color1 += 0.02 * vec3(cos(time), 0.0, sin(time * 0.3));

    float alpha1 = 0.12 + 0.4 * core1 * n1;

    float inner = internalMembrane(uv, time) * core1;
    vec3 color2 = color1 + vec3(0.2, 0.05, 0.1) * inner * 0.4;
    float alpha2 = alpha1 + 0.2 * inner;

    return vec4(mix(color1, color2, 0.5), mix(alpha1, alpha2, 0.5));
}

// Shader 2: Small Free-Floating Smoke Puffs
float smokePuff(vec2 uv, float t, int id) {
    float fi = float(id);
    vec2 center = vec2(sin(fi * 1.3 + t * 0.5), cos(fi * 1.7 - t * 0.4));
    float radius = 0.2 + 0.1 * sin(t * 0.3 + fi);
    float d = length(uv - center) + 0.01;
    return radius / (d * d);
}

vec4 shaderSmoke2(vec2 uv, float t) {
    uv = vPos.xy / 10.0;
    float rate = speedEnvelope(t);
    float time = t * rate;

    uv += 0.1 * vec2(sin(time * 0.2), cos(time * 0.25));

    float total = 0.0;
    for (int i = 0; i < 6; ++i) {
        total += smokePuff(uv, time, i);
    }

    vec2 drift = vec2(sin(total + time * 0.4), cos(total + time * 0.3));
    uv += 0.05 * total * drift;

    float n = fbm(vec3(uv * 1.5, time * 0.07));
    total *= n;

    vec3 color = mix(vec3(0.15, 0.2, 0.3), vec3(0.5, 0.1, 0.2), 0.5 + 0.5 * sin(time + total * 2.0));
    float alpha = 0.05 + 0.3 * total;

    return vec4(color * total, alpha);
}

// Shader 3: Engulfing Smoke
float engulfMask(vec2 uv, float t) {
    return smoothstep(1.2, 0.1, length(uv - vec2(-0.6, 0.0)) + 0.1 * sin(t * 0.5));
}

vec4 shaderSmoke3(vec2 uv, float t) {
    uv = vPos.xy / 15.0;
    float rate = speedEnvelope(t);
    float time = t * rate;

    vec2 drift = vec2(
        sin(uv.y * 1.5 + time * 0.3),
        cos(uv.x * 1.3 - time * 0.25)
    );
    uv += 0.25 * drift * fbm(vec3(uv * 1.5, time * 0.1));

    float smoke = fbm(vec3(uv * 2.5, time * 0.08)) * 1.2;
    float mask = engulfMask(uv, time);
    float intensity = pow(smoke, 2.0) * mask;

    vec3 color = mix(vec3(0.2, 0.3, 0.4), vec3(0.4, 0.1, 0.1), 0.5 + 0.5 * sin(time * 0.15 + smoke * 4.0));
    return vec4(color * intensity, 0.05 + 0.5 * intensity);
}

// Shader 4: Foam Bubble
float metaballField(vec2 uv, float t) {
    float field = 0.0;
    for (int i = 0; i < 7; ++i) {
        float fi = float(i);
        vec2 center = 1.2 * vec2(
            sin(t * 0.27 + fi * 1.3) + cos(fi + t * 0.11),
            cos(t * 0.19 + fi * 1.6) + sin(fi * 0.4 + t * 0.29)
        );
        float radius = 0.3 + 0.2 * sin(t * 0.23 + fi * 0.8);
        float dist = length(uv - center) + 0.005;
        field += radius / dist;
    }
    return field;
}

vec4 shaderFoamBubble(vec2 uv, float t) {
    uv = vPos.xy / 7.0;
    float rate = speedEnvelope(t);
    float time = t * rate;

    float meta = metaballField(uv, time);
    float warp = smoothstep(0.6, 3.5, meta);
    float pulse = 1.0 + 0.2 * sin(time * 0.8 + meta * 2.0);

    vec2 drift = vec2(
        sin(meta * 1.5 + time * 0.6),
        cos(meta * 1.7 + time * 0.5)
    );
    vec2 warpedUV = uv + 0.33 * warp * pulse * drift;

    warpedUV += 0.05 * vec2(
        sin(fbm(vec3(uv, time * 0.13)) * 8.0 + time),
        cos(fbm(vec3(uv.yx, time * 0.11)) * 6.0 - time)
    );

    float n = fbm(vec3(warpedUV * 0.95, time * 0.07));

    vec3 normal = normalize(vec3(
        n - fbm(vec3(warpedUV + vec2(0.015, 0.0), time * 0.07)),
        n - fbm(vec3(warpedUV + vec2(0.0, 0.015), time * 0.07)),
        0.3 + (time / 140.0)
    ));

    vec3 baseColor = mix(vec3(0.1, 0.4, 0.9), vec3(0.25, 0.05, 0.1), 0.5 + 0.5 * sin(time * 0.9 + meta * 2.5));

    vec3 light1 = normalize(vec3(0.4, 0.8, 1.0));
    vec3 light2 = normalize(vec3(-0.3, -0.5, 1.0));
    vec3 color = vec3(0.0);
    color += 0.3 * pow(dot(normal, light1), 2.5);
    color += 0.2 * pow(dot(normal, light2), 4.5);
    color += baseColor * n;

    color *= n * warp;
    color += 0.02 * vec3(cos(time), 0.0, sin(time * 0.3));

    return vec4(color, 0.12 + 0.4 * warp);
}

void main() {
    vec2 uv = vPos.xy / 10.0;
    float t = u_time;

    float seg = mod(t, 240.0);
    float blend = smoothstep(0.0, 1.0, fract(t));

    vec4 c1, c2;

    if (seg < 60.0) {
        c1 = shaderSmoke1(uv, t);
        c2 = shaderSmoke2(uv, t);
        blend = smoothstep(59.0, 60.0, seg);
    } else if (seg < 120.0) {
        c1 = shaderSmoke2(uv, t);
        c2 = shaderSmoke3(uv, t);
        blend = smoothstep(119.0, 120.0, seg);
    } else if (seg < 180.0) {
        c1 = shaderSmoke3(uv, t);
        c2 = shaderFoamBubble(uv, t);
        blend = smoothstep(179.0, 180.0, seg);
    } else {
        c1 = shaderFoamBubble(uv, t);
        c2 = shaderSmoke1(uv, t);
        blend = smoothstep(239.0, 240.0, seg);
    }

    vec4 finalColor = mix(c1, c2, blend);

    // === Fade to white in last 10 seconds ===
    float fadeWhite = smoothstep(481.0, 491.0, t);
    finalColor.rgb = mix(finalColor.rgb, vec3(1.0), fadeWhite);
    finalColor.a = mix(finalColor.a, 1.0, fadeWhite);

    fragColor = finalColor;
}
