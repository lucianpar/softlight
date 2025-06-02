#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;

#define TWO_PI 6.283

float D = 0.6;

float wave(vec2 p) {
  float v = sin(p.x + sin(p.y * 2.0) + sin(p.y * 0.43));
  return v * mod(v, p.x) / p.y;
}

const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);

float map(vec2 p) {
  float v = wave(p);
  p.x += u_time * 0.224; p *= rot; v += wave(p);
  p.x += u_time * 0.333 / p.y; p *= rot; v += wave(p) / p.x;
  return abs(1.5 - v + u_time / 10000.0);
}

// Structural bands (horizontal slices)
float bands(vec2 pos) {
  return smoothstep(0.49, 0.5, abs(sin(pos.y * 6.0 + u_time * 0.15)));
}

// Pointillistic breakup (speckle layer)
float speckle(vec2 p) {
  vec2 grid = floor(p * 30.0);
  vec2 f = fract(p * 30.0);
  float d = length(f - 0.5);
  float jitter = fract(sin(dot(grid, vec2(127.1, 311.7))) * 43758.5453);
  return smoothstep(0.05, 0.02, d + jitter * 0.02);
}

float layeredMap(vec2 pos) {
  float base = map(pos);
  float detail1 = map(pos * 2.5 + vec2(10.0, 5.0));
  float detail2 = map(pos * 5.5 + vec2(20.0, -10.0));
  return mix(base, detail1, 0.3) + detail2 * 0.1;
}

// Polar vortex swirl distortion centered around origin
vec2 vortex(vec2 uv, float strength, float time) {
  float r = length(uv);
  float angle = atan(uv.y, uv.x);
  angle += strength * exp(-r * 1.5) * sin(time * 0.2 + r * 6.0);
  return vec2(cos(angle), sin(angle)) * r;
}

vec3 Mucous_Membrane(vec2 pos) {
  // Vortex distortion
  vec2 centerUV = pos;
  pos = vortex(centerUV, 1.5, u_time);

  // Animate vertically for texture drift
  pos.y += u_time * 0.2;

  float v = layeredMap(pos);

  // DARK BLUE BASE with WHITE ACCENTS
  vec3 base = mix(vec3(0.05, 0.1, 0.3), vec3(0.0, 0.0, 0.1), v); // deep blue
  vec3 accent = vec3(1.0); // white
  vec3 c = mix(base, accent, 0.15 * layeredMap(pos * 0.1));

  // Add speckles to simulate a sparse structure
  float s = speckle(pos * 1.2);
  c *= s;

  // Banding
  float b = bands(pos);
  c = mix(c, vec3(0.9), b * 0.3); // thinner layered streaks

  // Surface lighting (thinner)
  vec3 n = normalize(vec3(
    v - layeredMap(vec2(pos.x + D, pos.y)),
    v - layeredMap(vec2(pos.x, pos.y + D)),
    -D
  ));
  vec3 l = normalize(vec3(0.1, 0.2, -0.5));
  float lighting = pow(dot(l, n), 2.0); // reduce smoothness, sharpen glints

  c *= lighting * 1.5;
  return c;
}

void main() {
  float fade = smoothstep(0.0, 10.5, u_time);
  vec3 shaderColor = Mucous_Membrane(vPos.xy / 20.0) * 0.9;
  vec3 finalColor = mix(vec3(0.0), shaderColor, fade);
  fragColor = vec4(finalColor, 1.0);
}
