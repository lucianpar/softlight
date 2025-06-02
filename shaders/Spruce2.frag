#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;

#define TWO_PI 6.283

float D = 0.6;

float wave(vec2 p) {
  float t = u_time * 0.1;

  float a = sin(p.x * 3.1 + sin(p.y * 4.3 + t * 0.8));
  float b = cos(p.y * 2.7 + cos(p.x * 5.1 + t * 1.3));
  float c = sin(p.x * 0.7 + cos(p.y * 1.9 + sin(t * 1.7)));
  float d = cos(length(p) * 2.0 + sin(dot(p, vec2(1.618, 3.1415)) + t * 0.5));

  // Combine and normalize to [0.0, 1.0]
  float v = (a * b + c * d) * 0.25 + 0.5;
  return clamp(v, 0.0, 1.0);
}

const mat2 rot = mat2(0.5, 0.86, -0.86, 0.5);

float map(vec2 p) {
  float v = wave(p);
  p.x += u_time * 0.224; p *= rot; v += wave(p);
  p.x += u_time * 0.333 / p.y; p *= rot; v += wave(p) / p.x;
  return abs(1.5 - v + u_time / 10000.0);
}

// Structural bands (layering with horizontal slices)
float bands(vec2 pos) {
  return smoothstep(0.4, 0.5, abs(sin(pos.y * 5.0 + u_time * 0.1)));
}

// Multi-layered map with varying scales
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
  vec3 accent = vec3(1.0); // white for streaks
  vec3 c = mix(base, accent, 0.15 * layeredMap(pos * 0.1));

  // Add horizontal layering
  float b = bands(pos);
  c = mix(c, vec3(0.9), b * 0.3); // faint glowing lamina

  // Surface lighting
  vec3 n = normalize(vec3(
    v - layeredMap(vec2(pos.x + D, pos.y)),
    v - layeredMap(vec2(pos.x, pos.y + D)),
    -D
  ));
  vec3 l = normalize(vec3(0.1, 0.2, -0.5));
  float lighting = dot(l, n) + pow(dot(l, n), 40.0);

  c *= lighting * lighting;
  return c;
}

void main() {
  float fade = smoothstep(0.0, 10.5, u_time);
  vec3 shaderColor = Mucous_Membrane(vPos.xy / 20.0) * 0.9;
  vec3 finalColor = mix(vec3(0.0), shaderColor, fade);
  fragColor = vec4(finalColor, 1.0);
}
