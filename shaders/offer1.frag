#version 330 core

in vec3 vPos;
in vec2 vUV;

out vec4 fragColor;

uniform float u_time;
uniform float onset;
uniform float cent;
uniform float flux;

#define TWO_PI 6.283
float D = 0.6;

// Gyroid noise
float gyroid(vec3 seed) {
  return dot(sin(seed), cos(seed.yzx));
}

// Fractal Brownian Motion
float fbm(vec3 seed) {
  float result = 0.0, a = 0.5;
  for (int i = 0; i < 6; ++i) {
    seed.z += result * 0.5;
    result += abs(gyroid(seed / a)) * a;
    a /= 2.0;
  }
  return result;
}

// Additional modulated fire field
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

// Final hybrid noise field
float map(vec2 p) {
  float v = wave(p);
  p.x += u_time * 0.224;
  p *= rot;
  v += wave(p);
  p.x += u_time * 0.333 / (p.y + 0.0001);
  p *= rot;
  v += wave(p) /abs(p.x + 0.0001);
  v += spiralFractal(p * 0.6, u_time * 0.2) * 0.5;
  return abs(1.5 - v + u_time / 10000.0);
}

float noise(vec2 p) {
  vec3 seed = vec3(p, length(p) - u_time * 0.1);
  return sin(fbm(seed) * 6.0 + u_time) * 0.5 + 0.5;
}

void main() {
  vec2 uv = vPos.xy * 2.0 - 1.0;
  float t = u_time * 0.1 ;

  // Subtle dynamic domain warp
  for (int i = 0; i < 30; ++i) {
    float k = cos(t);
    float l = sin(t);
    uv *= mat2(k * 1.001, l * 1.00001, l * 0.99999, -k * 0.99);
  }

  // Sample organic structure
  float shade = noise(uv);
  float v = map(uv);

  // Normals
   // Improved normals from the richer map field
  float center = map(uv);
  float dx = map(uv + vec2(0.01, 0.0)) - center;
  float dy = map(uv + vec2(0.0, 0.01)) - center;
  vec3 normal = normalize(vec3(dx, dy, 0.2));

  // Fire palette from original shader
  vec3 baseLow = vec3(0.9, 0.4, 0.1);  // deep orange
vec3 baseHigh = vec3(1.0, 0.85, 0.4); // golden yellow
vec3 color = mix(baseLow, baseHigh, shade); // use noise to shift warmth

  // Lighting from OrganicNoise.frag
  color += 0.4 * pow(dot(normal, vec3(0.0, 1.0, 1.0)) * 0.5, 10.0);
  vec3 tint = 0.5 + 0.5 * cos(vec3(1.0 * sin(t), 2.0, 3.0) * 5.0 + shade + normal.y * 2.0);
  color += tint * 0.3 * pow(dot(normal, vec3(0.0, 0.3, 1.0)) * 0.5 + 0.5, 10.0);
  color += 0.5 * vec3(1.0, 0.9, 0.8) * pow(dot(normal, vec3(0.0, -2.0, 1.0)) * 0.5 + 0.5, 2.0);

  // Final noise multiplied to dim base areas
    color = mix(vec3(0.2, 0.08, 0.01), color, shade /2.0);  // adds base orange in dark zones


  fragColor = vec4(color, 1.0);
}
