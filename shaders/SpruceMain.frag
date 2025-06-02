#version 330 core

in vec3 vPos;
out vec4 fragColor;

uniform float u_time;

#define TWO_PI 6.283
#define D 0.6

// === SP1: wave + spiral fractal ===
float wave(vec2 p) {
  float v = sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));
  return v * mod(v, p.x + 0.0001) / (p.y + 0.0001);
}

const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);

float spiralFractal(vec2 p) {
  float intensity = 0.0;
  float scale = 1.0;
  for (int i = 0; i < 4; ++i) {
    p = abs(p) / dot(p, p + 0.01) - 0.8;
    p *= rot;
    scale *= 1.2;
    intensity += dot(cos(p * scale + u_time), sin(p.yx * scale - u_time)) / scale;
  }
  return abs(intensity);
}

float map(vec2 p) {
  float v = wave(p);
  p.x += u_time * 0.224;
  p *= rot;
  v += wave(p);
  p.x += u_time * 0.333 / (p.y + 0.0001);
  p *= rot;
  v += wave(p) / (p.x + 0.0001);
  v += spiralFractal(p * 0.6) * 0.5;
  return abs(1.5 - v + u_time / 10000.0);
}

vec2 vortex(vec2 uv, float strength) {
  float r = length(uv);
  float angle = atan(uv.y, uv.x);
  angle += strength * exp(-r * 1.5) * sin(u_time * 0.2 + r * 6.0);
  return vec2(cos(angle), sin(angle)) * r;
}

vec4 shaderSpruce1(vec2 uv) {
  vec2 pos = vortex(uv, 1.5);
  pos.y += u_time * 0.2;

  float v = map(pos);

  vec3 base = mix(vec3(0.85, 0.9, 1.0), vec3(0.3, 0.5, 0.7), v);
  vec3 accent = vec3(0.6, 0.9, 1.0);
  vec3 c = mix(base, accent, 0.1 * map(pos * 0.1));

  vec3 n = normalize(vec3(
    v - map(pos + vec2(D, 0.0)),
    v - map(pos + vec2(0.0, D)),
    -D
  ));
  vec3 l = normalize(vec3(0.2, 0.3, -0.6));
  float lighting = dot(l, n) + pow(dot(l, n), 20.0);

  c *= lighting * lighting;
  return vec4(c, 1.0);
}

// === SP2 ===
float hash(vec2 p) {
  return fract(sin(dot(p, vec2(41.3, 289.7))) * 10000.5) * 0.85;
}

vec4 shaderSpruce2(vec2 uv, float t) {
  const float zoom = 0.010;
  const float speed = 0.1;
  const float formuparam = 0.53;
  const int iterations = 14;
  const int volsteps = 5;
  const float stepsize = 0.1;
  const float brightness = 0.002;
  const float darkmatter = 2.0;
  const float distfading = 0.73;
  const float saturation = 0.85;

  vec2 centeredUV = uv * 2.0 - 1.0;
  vec3 dir = normalize(vec3(centeredUV * zoom, 1.0));
  float time = t * speed + 0.25;
  vec3 from = vec3(1.0, 0.5, 0.5) + vec3(time * 2.0, time, -2.0);

  float a1 = 0.1 * (t / 2.0);
  float a2 = vPos.y != 0.0 ? vPos.x / vPos.y : vPos.x;

  mat2 rot1 = mat2(cos(vPos.y), sin(a1), -sin(vPos.y), cos(a1 * t));
  mat2 rot2 = mat2(cos(vPos.x), sin(a2 / t), -sin(a2), abs(a2 - vPos.x));
  dir.xz *= rot1; dir.xy *= rot2;
  from.xz *= rot1; from.xy *= rot2;

  float s = 0.1, fade = 1.0;
  vec3 v = vec3(0.0);

  for (int r = 0; r < volsteps; r++) {
    vec3 p = from + s * dir * 0.5;
    p = smoothstep(0.0, 0.85, abs(mod(p, 0.85 * 2.0) - 0.85));
    p *= 0.5;

    float pa = 0.0, a = 0.0;
    for (int i = 0; i < iterations; i++) {
      p = abs(p) / dot(p, p) - formuparam;
      a += abs(length(p) - pa / cos(2.0 * abs(vPos.y) / ((t / 46.0 + 0.1)) / 10.0));
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

// === MAIN ===
void main() {
  vec2 uv = (vPos.xy + vec2(1.0)) * 0.5;

  float blend = smoothstep(4.0, 5.0, u_time);
  vec4 c1 = shaderSpruce1(uv);
  vec4 c2 = shaderSpruce2(uv, u_time);

  fragColor = mix(c1, c2, blend);
}
