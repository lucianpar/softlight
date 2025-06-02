#version 330 core

in vec3 vPos;
out vec4 fragColor;

uniform float u_time;

const float zoom = 0.010;
const float speed = 0.1;
const float formuparam = 0.53;
const int iterations = 14;
const int volsteps = 5;

const float stepsize = 0.1; // this is super important 
const float tile = 0.85;
const float brightness = 0.002;
const float songLength = 46.0; // define song length in seconds
const float darkmatter = 2.0;
const float distfading = 0.73;
const float saturation = 0.85;

// hash - doesn't really make sense to me
float hash(vec2 p) {
    return fract(sin(dot(p, vec2(41.3, 289.7))) * 10000.5) * tile;
}

void main() {
    vec2 uv = (vPos.xy + vec2(1.0)) * 0.1;
    vec2 centeredUV = uv * 2.0 - 1.0;

    vec3 dir = normalize(vec3(centeredUV * zoom, 1.0));
    float time = u_time * speed + 0.25;

    vec3 from = vec3(1.0, 0.5, 0.5) + vec3(time * 2.0, time, -2.0);

    float a1 = 0.1 * (u_time / 2.0);
    // Prevent division by zero
    float a2 = vPos.y != 0.0 ? vPos.x/vPos.y : vPos.x;
    // Use stable value for rotation instead of directly using vPos.y
    mat2 rot1 = mat2(cos(vPos.y), sin(a1), -sin(vPos.y), cos(a1*u_time));
    mat2 rot2 = mat2(cos(vPos.x), sin(a2/u_time), -sin(a2), abs(a2-vPos.x));
    dir.xz *= rot1; dir.xy *= rot2;
    from.xz *= rot1; from.xy *= rot2;

    float s = 0.1, fade = 1.0;
    vec3 v = vec3(0.0);

    for (int r = 0; r < volsteps; r++) {
       vec3 p = from + s * dir * 0.5;
p = smoothstep(0.0, tile, abs(mod(p, tile * 2.0) - tile));

// SCALE fractal space to make formations larger
p *= 0.5;

float pa = 0.0, a = 0.0;
for (int i = 0; i < iterations; i++) {
    p = abs(p) / dot(p, p) - formuparam;
    a += abs(length(p) - pa / tan(2.0 * abs(vPos.y) / ((u_time / songLength + 0.1)) / 10.0));
    pa = length(p) + (cos(vPos.x) / cos(a + 0.001));
    
}
        float dm = max(0.0, darkmatter - a * a * 0.001 * u_time);
        a *= a * atan(a);
        //a = pow(a, 2.5); 

        if (r > 6) fade *= 1.0 - dm;

        v += fade;
        v += vec3(s, s * s, s * s * s * s) * a * brightness * fade;
        fade *= distfading ;
        s += stepsize ;
    }

    v = mix(vec3(length(v)), v, saturation);

 
    float intensity = length(v);
vec3 blue = vec3(0.2, 0.6, 1.0);  // highlight blue
vec3 base = vec3(0.3);            // white ambient

// Color interpolation based on brightness
vec3 finalColor = mix(base, blue, smoothstep(0.1, 0.4, intensity));

// Scale brightness as before
fragColor = vec4(finalColor * 0.01 * intensity, 1.0);
}
