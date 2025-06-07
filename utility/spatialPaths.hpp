#ifndef SPATIAL_PATHS_HPP
#define SPATIAL_PATHS_HPP

#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Vec.hpp"
#include <cmath>

struct spatialPaths {
  // basic thomas attractor

public:
  // float b = 0.208;
  // float speed = 0.1;

  al::Vec3f ThomasAttractor(al::Vec3f input, float time, float speed,
                            float b = 0.208) {
    al::Vec3f output;
    output.x = input.x + (-b * input.x + sin(input.y)) * time * speed;
    output.y = input.y + (-b * input.y + sin(input.z)) * time * speed;
    output.z = input.z + (-b * input.z + sin(input.x)) * time * speed;

    return output;
  }

  al::Vec3f ChenAttractor(al::Vec3f input, float time, float speed,
                          float a = 35.0f, float c = 28.0f, float d = 3.0f) {
    al::Vec3f output;
    output.x = input.x + (a * (input.y - input.x)) * time * speed;
    output.y = input.y +
               (c * input.x - input.x * input.z + c * input.y) * time * speed;
    output.z = input.z + (input.x * input.y - d * input.z) * time * speed;
    return output;
  }

  al::Vec3f LorenzAttractor(al::Vec3f input, float time, float speed,
                            float sigma = 10.0f, float rho = 28.0f,
                            float beta = 8.0f / 3.0f) {
    al::Vec3f output;
    output.x = input.x + sigma * (input.y - input.x) * time * speed;
    output.y = input.y + (input.x * (rho - input.z) - input.y) * time * speed;
    output.z = input.z + (input.x * input.y - beta * input.z) * time * speed;
    return output;
  }

  //
  al::Vec3f SuperformulaAttractor(al::Vec3f input, float time, float speed,
                                  float a = 1.0f, float b = 1.0f,
                                  float m = 6.0f, float n1 = 0.3f,
                                  float n2 = 1.0f, float n3 = 1.0f) {
    float r = input.mag();
    float theta = std::atan2(input.y, input.x);     // azimuth
    float phi = std::acos(input.z / (r + 0.0001f)); // polar angle

    float t = theta;
    float s = phi;

    float part1 = pow(fabs(cos(m * t / 4.0f) / a), n2);
    float part2 = pow(fabs(sin(m * t / 4.0f) / b), n3);
    float superR = pow(part1 + part2, -1.0f / n1);

    float x = superR * sin(s) * cos(t);
    float y = superR * sin(s) * sin(t);
    float z = superR * cos(s);

    al::Vec3f output;
    output.x = input.x + (x - input.x) * time * speed;
    output.y = input.y + (y - input.y) * time * speed;
    output.z = input.z + (z - input.z) * time * speed;
    return output;
  }

  al::Vec3f BacterialGrowth(al::Vec3f input, float time, float speed,
                            float rMax = 2.0f, float growthRate = 1.0f,
                            float noiseScale = 0.1f) {
    al::Vec3f output;

    float r = input.mag(); // Distance from origin (colony center)

    // Logistic growth model
    float dr =
        growthRate * r * (1.0f - r / rMax); // classic dN/dt = rN(1 - N/K)

    // Optional undulating variation for natural look
    float oscillation = sin(time + r) * noiseScale;

    // Apply outward growth
    al::Vec3f direction = input.normalized();
    float delta = (dr + oscillation) * time * speed;

    output = input + direction * delta;
    return output;
  }

  al::Vec3f FlameAttractor(al::Vec3f input, float time, float speed,
                           float swirlFreq = 4.0f, float swirlAmp = 0.2f,
                           float riseSpeed = 1.0f, float decay = 0.1f) {
    al::Vec3f output;

    float x = input.x;
    float y = input.y;
    float z = input.z;

    // Swirling lateral motion using sine waves
    float swirlX = sin(y * swirlFreq + time) * swirlAmp;
    float swirlZ = cos(y * swirlFreq + time) * swirlAmp;

    // Vertical rise decays with height
    float lift = riseSpeed * exp(-decay * y);

    output.x = x + swirlX * time * speed;
    output.y = y + lift * time * speed;
    output.z = z + swirlZ * time * speed;

    return output;
  }
};

#endif