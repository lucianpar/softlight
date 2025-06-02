#version 330 core

in vec3 vPos;
out vec4 fragColor;

uniform float u_time;

#define TWO_PI 6.283

// === Spruce1 ===
float D = 0.6;

float wave(vec2 p) {
    float v = sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));
    return v * mod(v, p.x + 0.0001) / (p.y + 0.0001);
}

const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);

float spiralFractal(vec2 p, float time) {
    float intensity = 0.0;
    float scale = 1.0;
    for (int i = 0; i < 4; ++i) {
        p = abs(p) / dot(p, p + 0.01) - 0.8;
        p *= rot;
        scale *= 1.2;
        intensity += dot(cos(p * scale + time), sin(p.yx * scale - time)) / scale;
    }
    return abs(intensity);
}

float map(vec2 p, float t) {
    float v = wave(p);
    p.x += t * 0.224;
    p *= rot;
    v += wave(p);
    p.x += t * 0.333 / (p.y + 0.0001);
    p *= rot;
    v += wave(p) / (p.x + 0.0001);
    v += spiralFractal(p * 0.6, t * 0.2) * 0.5;
    return abs(1.5 - v + t / 10000.0);
}

vec2 vortex(vec2 uv, float strength, float time) {
    float r = length(uv);
    float angle = atan(uv.y, uv.x);
    angle += strength * exp(-r * 1.5) * sin(time * 0.2 + r * 6.0);
    return vec2(cos(angle), sin(angle)) * r;
}

vec4 shaderSpruce1(vec2 uv, float t) {
    vec2 pos = vortex(vPos.xy / 20.0, 1.5, t);
    pos.y += t * 0.2;

    float v = map(pos, t);

    vec3 base = mix(vec3(0.85, 0.9, 1.0), vec3(0.3, 0.5, 0.7), v);
    vec3 accent = vec3(0.6, 0.9, 1.0);
    vec3 c = mix(base, accent, 0.1 * map(pos * 0.1, t));

    vec3 n = normalize(vec3(
        v - map(pos + vec2(D, 0.0), t),
        v - map(pos + vec2(0.0, D), t),
        -D
    ));

    vec3 l = normalize(vec3(0.2, 0.3, -0.6));
    float lighting = dot(l, n) + pow(dot(l, n), 20.0);
    c *= lighting * lighting;

    return vec4(c, 1.0);
}

// === Spruce2 ===
vec4 shaderSpruce2(vec2 uv, float t) {
    const float zoom = 0.010;
    const float speed = 0.1;
    const float formuparam = 0.53;
    const int iterations = 14;
    const int volsteps = 5;
    const float stepsize = 0.1;
    const float tile = 0.85;
    const float brightness = 0.002;
    const float songLength = 46.0;
    const float darkmatter = 2.0;
    const float distfading = 0.73;
    const float saturation = 0.85;

    vec2 centeredUV = uv * 2.0 - 1.0;
    vec3 dir = normalize(vec3(centeredUV * zoom, 1.0));
    float time = t * speed + 0.25;
    vec3 from = vec3(1.0, 0.5, 0.5) + vec3(time * 2.0, time, -2.0);

    float a1 = 0.1 * (t / 2.0);
    float a2 = vPos.y != 0.0 ? vPos.x / vPos.y : vPos.x;
    mat2 rot1 = mat2(cos(vPos.y), sin(a1), -sin(vPos.y), cos(a1));
    mat2 rot2 = mat2(cos(vPos.x), sin(a2), -sin(a2), abs(a2 - vPos.x));
    dir.xz *= rot1; dir.xy *= rot2;
    from.xz *= rot1; from.xy *= rot2;

    float s = 0.1, fade = 1.0;
    vec3 v = vec3(0.0);

    for (int r = 0; r < volsteps; r++) {
        vec3 p = from + s * dir * 0.5;
        p = smoothstep(0.0, tile, abs(mod(p, tile * 2.0) - tile));
        p *= 0.5;

        float pa = 0.0, a = 0.0;
        for (int i = 0; i < iterations; i++) {
            p = abs(p) / dot(p, p) - formuparam;
            a += abs(length(p) - pa / cos(2.0 * abs(vPos.y) / ((t / songLength + 0.1)) / 10.0));
            pa = length(p) + (abs(vPos.x) / cos(a + 0.001));
        }

        float dm = max(0.0, darkmatter - a * a * 0.001);
        a *= a * atan(a);
        if (r > 6) fade *= 1.0 - dm;

        v += fade;
        v += vec3(s, s * s, s * s * s * s) * a * brightness * fade;
        fade *= distfading;
        s += stepsize;
    }

    v = mix(vec3(length(v)), v, saturation);
    float intensity = length(v);
    vec3 blue = vec3(0.2, 0.6, 1.0);
    vec3 base = vec3(0.3);
    vec3 finalColor = mix(base, blue, smoothstep(0.1, 0.4, intensity));
    return vec4(finalColor * 0.01 * intensity, 1.0);
}

// === Spruce3 ===
vec4 shaderSpruce3(vec2 uv, float t) {
    const float zoom = 0.010;
    const float speed = 0.1;
    const float formuparam = 0.53;
    const int iterations = 14;
    const int volsteps = 5;
    const float stepsize = 0.1;
    const float tile = 0.85;
    const float brightness = 0.002;
    const float songLength = 46.0;
    const float darkmatter = 2.0;
    const float distfading = 0.73;
    const float saturation = 0.85;

    vec2 centeredUV = uv * 2.0 - 1.0;
    vec3 dir = normalize(vec3(centeredUV * zoom, 1.0));
    float time = t * speed + 0.25;
    vec3 from = vec3(1.0, 0.5, 0.5) + vec3(time * 2.0, time, -2.0);

    float a1 = 0.1 * (t / 2.0);
    float a2 = vPos.y != 0.0 ? vPos.x / vPos.y : vPos.x;
    mat2 rot1 = mat2(cos(vPos.y), sin(a1), -sin(vPos.y), cos(a1));
    mat2 rot2 = mat2(cos(vPos.x), sin(a2), -sin(a2), abs(a2 - vPos.x));
    dir.xz *= rot1; dir.xy *= rot2;
    from.xz *= rot1; from.xy *= rot2;

    float s = 0.1, fade = 1.0;
    vec3 v = vec3(0.0);

    for (int r = 0; r < volsteps; r++) {
        vec3 p = from + s * dir * 0.5;
        p = smoothstep(0.0, tile, abs(mod(p, tile * 2.0) - tile));
        p *= 0.5;

        float pa = 0.0, a = 0.0;
        for (int i = 0; i < iterations; i++) {
            p = abs(p) / dot(p, p) - formuparam;
            a += abs(length(p) - pa / cos(2.0 * abs(vPos.y) / ((t / songLength + 0.1)) / 10.0));
            pa = length(p) + (abs(vPos.x) / sin(a + 0.001));
        }

        float dm = max(0.0, darkmatter - a * a * 0.001);
        a *= a * a;
        if (r > 6) fade *= 1.0 - dm;

        v += fade;
        v += vec3(s, s * s, s * s * s * s) * a * brightness * fade;
        fade *= distfading;
        s += stepsize;
    }

    v = mix(vec3(length(v)), v, saturation);
    float intensity = length(v);
    vec3 blue = vec3(0.2, 0.6, 1.0);
    vec3 base = vec3(0.3);
    vec3 finalColor = mix(base, blue, smoothstep(0.1, 0.4, intensity));
    return vec4(finalColor * 0.01 * intensity, 1.0);
}

vec4 shaderSpruce4(vec2 uv, float t) {
    const float zoom = 10.0;
    const float speed = 0.1;
    const float formuparam = 0.53;
    const int iterations = 15;
    const int volsteps = 5;

    const float stepsize = 0.1;
    const float tile = 0.85;
    const float brightness = 0.002;
    const float songLength = 46.0;
    const float darkmatter = 2.0;
    const float distfading = 0.73;
    const float saturation = 0.85;

    vec2 centeredUV = uv * 2.0 - 1.0;
    vec3 dir = normalize(vec3(centeredUV * zoom, 1.0));
    float time = t * speed + 0.25;

    vec3 from = vec3(1.0, 0.5, 0.5) + vec3(time * 2.0, time, -2.0);

    // --- Chaotic Matrix Rotation ---
    float chaosA = sin(t * 0.1 + vPos.y * 2.1) * 1.2;
    float chaosB = cos(t * 0.2 + vPos.x * 1.7) * 1.5;
    float chaosC = sin(vPos.x * 0.5 + vPos.y * 0.3 + t * 0.15) * 2.0;

    mat2 baseRot = mat2(cos(chaosA), sin(chaosB), -sin(chaosB), cos(chaosC));
    mat2 swirl1 = mat2(cos(chaosC), sin(chaosA), -sin(chaosA), cos(chaosB));
    mat2 swirl2 = mat2(cos(chaosB + chaosA), sin(chaosB - chaosC),
                       -sin(chaosA + chaosC), cos(chaosA - chaosB));

    dir.xy *= baseRot;
    dir.xz *= swirl1;
    from.xy *= swirl2;
    from.yz *= baseRot;

    float zoomMod = 1.0 + 0.1 * sin(t * 0.8 + length(vPos.xy));
    dir *= zoomMod;

    float s = 0.1, fade = 1.0;
    vec3 v = vec3(0.0);

    for (int r = 0; r < volsteps; r++) {
        vec3 p = from + s * dir * 0.5;
        p = smoothstep(0.0, tile, abs(mod(p, tile * 2.0) - tile));

        float pa = 0.0, a = 0.0;
        for (int i = 0; i < iterations; i++) {
            p = abs(p) / dot(p, p) - formuparam;
            a += abs(length(p) - pa / (2.0 * abs(vPos.y) / ((t / songLength + 0.1)) / 10.0));
            pa = length(p) + (abs(vPos.x) / (a + 0.001));
        }

        float dm = max(0.0, darkmatter - a * a * 0.001);
        a *= a * a;

        if (r > 6) fade *= 1.0 - dm;

        v += fade;
        v += vec3(s, s * s, s * s * s * s) * a * brightness * fade;
        fade *= distfading;
        s += stepsize;
    }

    v = mix(vec3(length(v)), v, saturation);

    float intensity = length(v);
    vec3 blue = vec3(0.2, 0.6, 1.0);
    vec3 base = vec3(1.0);

    vec3 finalColor = mix(base, blue, smoothstep(0.1, 2.0, intensity));
    return vec4(finalColor * 0.01 * intensity, 1.0);
}


float ease(float x) {
    return x * x * (3.0 - 2.0 * x); // smooth cubic easing
}
// === Main ===
void main() {
    vec2 uv = (vPos.xy + vec2(1.0)) * 0.5;
    float t = u_time;

    vec4 c1, c2;
    float blend = 0.0;

    if (t < 25.0) {
        fragColor = shaderSpruce1(uv, t);
    }
    else if (t < 28.0) {
        float b = ease(smoothstep(25.0, 28.0, t));
        c1 = shaderSpruce1(uv, 25.0);
        c2 = shaderSpruce2(uv, t - 25.0);
        fragColor = mix(c1, c2, b);
    }
    else if (t < 45.0) {
        fragColor = shaderSpruce2(uv, t - 25.0);
    }
    else if (t < 48.0) {
        float b = ease(smoothstep(45.0, 48.0, t));
        c1 = shaderSpruce2(uv, 20.0); // fixed t
        vec4 rampIn = shaderSpruce3(uv, t - 45.0) * b; // ramp brightness
        fragColor = mix(c1, rampIn, b);
    }
    else if (t < 77.0) {
        float ramp = smoothstep(48.0, 50.0, t);
        fragColor = shaderSpruce3(uv, t - 45.0) * ramp;
    }
    else if (t < 81.0) {
        float b = ease(smoothstep(77.0, 81.0, t));
        c1 = shaderSpruce3(uv, 32.0);
        c2 = shaderSpruce4(uv, t - 77.0) * b;
        fragColor = mix(c1, c2, b);
    }
    else if (t < 93.0) {
        float ramp = smoothstep(81.0, 85.0, t);
        fragColor = shaderSpruce4(uv, t - 77.0) * ramp;
    }
    else if (t < 108.0) {
        float fade = smoothstep(90.0, 108.0, t);
        vec4 c = shaderSpruce4(uv, (t - 77.0) * 1.8); // speed-up
        vec3 darkened = mix(c.rgb, vec3(0.05, 0.08, 0.1), fade);
        fragColor = vec4(darkened, 1.0);
    }
    else {
        fragColor = vec4(0.0, 0.0, 0.0, 1.0);
    }
}
