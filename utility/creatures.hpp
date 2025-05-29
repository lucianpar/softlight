#ifndef CREATURES_HPP
#define CREATURES_HPP

#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Constants.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_StdRandom.hpp"
#include "al/math/al_Vec.hpp"
class Creature {

public:
  // extreme llm use for this - will find the conversation and link it (started
  // with my own rough geometry and gave it reference images)
  void makeJellyfish(al::VAOMesh &mesh, float headRadius = 0.5f,
                     int radialDivs = 64, int verticalDivs = 32,
                     int ruffleRings = 6, int numTendrils = 90,
                     int segmentsPerTendril = 100, float legRatio = 4.0f,
                     float amplitude = 0.2f, float frequency = 6.0f,
                     float twistAmount = 0.8f) {
    using namespace al;
    mesh.reset();
    mesh.primitive(Mesh::POINTS);

    // head geometry
    for (int y = 0; y < verticalDivs; ++y) {
      float v = (float)y / (verticalDivs - 1);
      float ringRadius = sin(v * M_PI * 0.5f);
      float yPos = cos(v * M_PI * 0.5f);

      float lobeMod = 1.0f + 0.05f * sin(radialDivs * v * M_PI * 2.0f);

      for (int x = 0; x < radialDivs; ++x) {
        float u = (float)x / radialDivs;
        float theta = u * M_2PI;
        float r = ringRadius * headRadius * lobeMod;

        float px = r * cos(theta);
        float py = yPos * headRadius * (0.6 + 0.4 * v);
        float pz = r * sin(theta);

        mesh.vertex(Vec3f(px, py, pz));
        mesh.color(RGB(1.0, 0.6, 0.7));
      }
    }

    // ruffled skirt geom
    for (int ring = 0; ring < ruffleRings; ++ring) {
      float yOffset = -0.005f * ring + 0.02f;             // raised by +0.02f
      float radius = headRadius * (0.35f + 0.05f * ring); // tighter taper

      for (int x = 0; x < radialDivs; ++x) {
        float u = (float)x / radialDivs;
        float theta = u * M_2PI;
        float wobble = 0.02f * sin(8 * theta + ring * 0.5f);

        float px = (radius + wobble) * cos(theta);
        float py = yOffset;
        float pz = (radius + wobble) * sin(theta);

        mesh.vertex(Vec3f(px, py, pz));
        mesh.color(RGB(1.0, 0.8, 0.9));
      }
    }

    // tendrils
    float legLength = headRadius * legRatio;

    for (int i = 0; i < numTendrils; ++i) {
      float angle = al::rnd::uniform(M_2PI);
      float radialOffset = al::rnd::uniform(0.2f, 0.5f) * headRadius;

      float baseX = cos(angle) * radialOffset;
      float baseZ = sin(angle) * radialOffset;

      float thisAmp = amplitude * al::rnd::uniform(0.5f, 1.2f);
      float thisFreq = frequency * al::rnd::uniform(0.5f, 1.2f);
      float thisTwist = twistAmount * al::rnd::uniformS();

      for (int j = 0; j < segmentsPerTendril; ++j) {
        float t = (float)j / segmentsPerTendril;
        float y = -t * legLength;

        float offsetX = sin(j * thisFreq * 0.1f) * thisAmp * (1.0f - t);
        float offsetZ = cos(j * thisFreq * 0.1f) * thisAmp * (1.0f - t);
        float spiral = thisTwist * j;

        float x = baseX + offsetX + cos(spiral) * 0.01f;
        float z = baseZ + offsetZ + sin(spiral) * 0.01f;

        mesh.vertex(Vec3f(x, y, z));
        mesh.color(RGB(0.9, 0.5 + 0.3 * t, 0.8));
      }
    }

    mesh.update();

    std::cout << "[Jellyfish] Organic mesh created with "
              << mesh.vertices().size() << " vertices." << std::endl;
  }
  // end of jellfish

  void addStarfish(al::Mesh &m, int arms = 5, float armLength = 1.0f,
                   float armWidth = 0.1f) {
    m.primitive(al::Mesh::TRIANGLES_ADJACENCY);
    m.reset();

    al::Vec3f center(0, 0, 0);

    for (int i = 0; i < arms; ++i) {
      float angle = i * (2 * M_PI / arms);
      float nextAngle = angle + M_PI / arms; // for width offset

      // Tip direction
      al::Vec3f dir(cos(angle), sin(angle), 0);
      al::Vec3f perp(-dir.y, dir.x, 0); // perpendicular for width

      // Arm tip vertex
      al::Vec3f tip = dir * armLength;
      // Base left and right for width
      al::Vec3f baseLeft = center + perp * armWidth * 0.5f;
      al::Vec3f baseRight = center - perp * armWidth * 0.5f;

      // Triangle: baseLeft -> tip -> baseRight
      m.vertex(baseLeft);
      m.vertex(tip);
      m.vertex(baseRight);

      // Color per arm (optional gradient)
      float hue = float(i) / arms;
      al::Color c = al::HSV(hue, 0.8, 0.9);
      m.color(c);
      m.color(c);
      m.color(c);
    }
  }
};

#endif