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
  p.x += u_time * 0.224;  p *= rot;  v += wave(p);
  p.x += u_time * 0.333 / p.y;  p *= rot;  v += wave(p) / p.x;
  return abs(1.5 - v + u_time / 10000.0);
}

vec3 Mucous_Membrane(vec2 pos) {
  pos.y += u_time * 0.2;
  float v = map(pos);

  vec3 base = mix(vec3(0.8, 0.9, 1.0), vec3(0.2, 0.6, 0.9), v);
  vec3 accent = vec3(0.5, 0.9, 0.7);

  vec3 c = mix(base, accent, 0.1 * map(pos * 0.1));

  vec3 n = normalize(vec3(
    v - map(vec2(pos.x + D, pos.y)),
    v - map(vec2(pos.x, pos.y + D)),
    -D
  ));
  vec3 l = normalize(vec3(0.1, 0.2, -0.5));
  v = dot(l, n) + pow(dot(l, n), 40.0);

  c *= v * v;
  return c;
}

void main() {
 

  // Otherwise evaluate shader and fade in (quickly)
  float fade = smoothstep(0.0, 10.5, u_time);  // fade in over 0.5s
  vec3 shaderColor = Mucous_Membrane(vPos.xy) * 0.9;
  vec3 finalColor = mix(vec3(1.0), shaderColor, fade);
  fragColor = vec4(finalColor, 1.0);
}
