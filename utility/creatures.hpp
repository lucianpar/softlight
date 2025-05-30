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

  void addStarfish(al::VAOMesh &m, int arms = 7, float baseArmLength = 1.0f,
                   float baseArmWidth = 0.1f) {
    m.primitive(al::Mesh::TRIANGLES);
    m.reset();

    for (int i = 0; i < arms; ++i) {
      float angle = i * (2 * M_PI / arms);

      // Randomize a bit for alien irregularity
      float angleOffset = al::rnd::uniform(-0.15f, 0.15f); // small bend
      float lenJitter = al::rnd::uniform(0.7f, 1.3f);
      float widthJitter = al::rnd::uniform(0.6f, 1.4f);
      float zJitter = al::rnd::uniform(-0.1f, 0.1f);

      float finalAngle = angle + angleOffset;
      float nextAngle = finalAngle + (2 * M_PI / arms) * 0.3f;

      // Compute slightly skewed base and tip
      al::Vec3f baseLeft(cos(finalAngle) * baseArmWidth * widthJitter,
                         sin(finalAngle) * baseArmWidth * widthJitter, zJitter);

      al::Vec3f baseRight(cos(nextAngle) * baseArmWidth * widthJitter,
                          sin(nextAngle) * baseArmWidth * widthJitter,
                          -zJitter);

      al::Vec3f tip =
          ((baseLeft + baseRight).normalize()) * baseArmLength * lenJitter;
      tip.z += al::rnd::uniform(-0.2f, 0.2f); // extra organic tip depth

      // Triangle: baseLeft -> tip -> baseRight
      m.vertex(baseLeft);
      m.vertex(tip);
      m.vertex(baseRight);

      // Alieny color gradient
      float hue = float(i) / arms;
      float sat = al::rnd::uniform(0.6f, 1.0f);
      float val = al::rnd::uniform(0.7f, 1.0f);
      al::Color c = al::HSV(hue, sat, val);

      m.color(c);
      m.color(c);
      m.color(c);
    }
  }
  // https://chatgpt.com/g/g-p-6821791853348191b683bea2cf85363d-softlight-sphere-project/c/68394302-a0f0-8011-ac5b-6552663a3c92
  // bending over tree
  void addTree1(al::VAOMesh &m, int targetVertexCount = 20000, int maxDepth = 8,
                float baseLength = 1.0f, float branchScale = 0.7f,
                float baseRadius = 0.3f, int circleDetail = 14,
                int stepsPerBranch = 16, int branchFactor = 3,
                bool addCanopy = true) {

    m.primitive(al::Mesh::POINTS);
    m.reset();

    struct Branch {
      al::Vec3f start;
      al::Vec3f dir;
      float radius;
      int depth;
    };

    std::vector<Branch> stack;
    stack.push_back(
        {al::Vec3f(0, 0, 0), al::Vec3f(0, baseLength, 0), baseRadius, 0});

    int vertexCount = 0;
    al::Vec3f finalTop;

    while (!stack.empty() && vertexCount < targetVertexCount) {
      Branch b = stack.back();
      stack.pop_back();

      al::Vec3f up = b.dir.normalize();
      al::Vec3f right = up.cross(al::Vec3f(0, 0, 1)).normalize();
      if (right.mag() < 0.001f)
        right = al::Vec3f(1, 0, 0);
      al::Vec3f forward = up.cross(right).normalize();

      float segmentLength = b.dir.mag() / stepsPerBranch;

      for (int s = 0; s <= stepsPerBranch; ++s) {
        float t = float(s) / stepsPerBranch;

        // Gentle S-curve to keep the trunk centered and elegant
        float trunkSway = 0.05f * sin((b.depth + t) * M_PI);
        al::Vec3f center = b.start + b.dir * t + right * trunkSway;

        // Exponential tapering, more dramatic at base
        float radius = b.radius * pow(0.6f, t);

        for (int i = 0; i < circleDetail; ++i) {
          float theta = M_2PI * i / circleDetail;
          al::Vec3f offset = cos(theta) * right + sin(theta) * forward;
          al::Vec3f p = center + offset * radius;

          m.vertex(p);
          al::Color c = al::HSV((float)b.depth / maxDepth, 0.5, 0.95);
          m.color(c);
          vertexCount++;
          if (vertexCount >= targetVertexCount)
            break;
        }

        if (b.depth == maxDepth && s == stepsPerBranch / 2)
          finalTop = center;

        if (vertexCount >= targetVertexCount)
          break;
      }

      if (b.depth < maxDepth && vertexCount < targetVertexCount) {
        for (int i = 0; i < branchFactor; ++i) {
          float theta = M_2PI * i / float(branchFactor);

          // Local frame branching
          al::Vec3f up = b.dir.normalize();
          al::Vec3f right = up.cross(al::Vec3f(0, 0, 1)).normalize();
          if (right.mag() < 0.001f)
            right = al::Vec3f(1, 0, 0);
          al::Vec3f forward = up.cross(right).normalize();

          al::Vec3f side = cos(theta) * right + sin(theta) * forward;
          al::Vec3f newDir =
              (up + side * 0.5f).normalize() * b.dir.mag() * branchScale;

          float newRadius = b.radius * 0.6f;
          stack.push_back({b.start + b.dir, newDir, newRadius, b.depth + 1});
        }
      }
    }

    if (addCanopy) {
      float canopyRadius = baseLength * pow(branchScale, maxDepth) * 1.4f;
      int canopyRings = 10;
      int canopyDetail = 18;

      for (int i = 0; i <= canopyRings; ++i) {
        float v = float(i) / canopyRings;
        float phi = M_PI * v * 0.5f; // upper hemisphere

        for (int j = 0; j < canopyDetail; ++j) {
          float u = float(j) / canopyDetail;
          float theta = M_2PI * u;

          float x = canopyRadius * sin(phi) * cos(theta);
          float y = canopyRadius * cos(phi);
          float z = canopyRadius * sin(phi) * sin(theta);

          al::Vec3f p = finalTop + al::Vec3f(x, y, z);
          m.vertex(p);
          m.color(al::Color(0.2f, 0.8f, 0.3f));
          vertexCount++;
          if (vertexCount >= targetVertexCount)
            return;
        }
      }
    }
  }
  void addTree2(al::VAOMesh &m, int targetVertexCount = 30000, int maxDepth = 8,
                float baseLength = 1.0f, float branchScale = 0.7f,
                float baseRadius = 0.3f, int circleDetail = 24,
                int stepsPerBranch = 32, int branchFactor = 3,
                bool addCanopy = true) {

    m.primitive(al::Mesh::POINTS);
    m.reset();

    struct Branch {
      al::Vec3f start;
      al::Vec3f dir;
      float radius;
      int depth;
    };

    std::vector<Branch> stack;
    stack.push_back(
        {al::Vec3f(0, 0, 0), al::Vec3f(0, baseLength, 0), baseRadius, 0});

    int vertexCount = 0;
    al::Vec3f finalTop;

    while (!stack.empty() && vertexCount < targetVertexCount) {
      Branch b = stack.back();
      stack.pop_back();

      al::Vec3f up = b.dir.normalize();
      al::Vec3f right = up.cross(al::Vec3f(0, 0, 1)).normalize();
      if (right.mag() < 0.001f)
        right = al::Vec3f(1, 0, 0);
      al::Vec3f forward = up.cross(right).normalize();

      float segmentLength = b.dir.mag() / stepsPerBranch;

      for (int s = 0; s <= stepsPerBranch; ++s) {
        float t = float(s) / stepsPerBranch;

        // S-curve along Y axis
        float trunkSway = 0.05f * sin((b.depth + t) * M_PI);
        al::Vec3f center = b.start + b.dir * t + right * trunkSway;

        // More gradual exponential tapering using cosine blend
        float taper = 0.5f * (1.0f + cos(t * M_PI));
        float radius =
            b.radius * (0.4f + 0.6f * taper); // range ~[0.4,1] * b.radius

        // Add twist to connect rings
        float twistOffset = t * M_2PI * 0.25f + b.depth;

        for (int i = 0; i < circleDetail; ++i) {
          float theta = (M_2PI * i / circleDetail) + twistOffset;
          al::Vec3f offset = cos(theta) * right + sin(theta) * forward;
          al::Vec3f p = center + offset * radius;

          m.vertex(p);
          al::Color c = al::HSV((float)b.depth / maxDepth, 0.6, 1.0);
          m.color(c);
          vertexCount++;
          if (vertexCount >= targetVertexCount)
            break;
        }

        if (b.depth == maxDepth && s == stepsPerBranch / 2)
          finalTop = center;

        if (vertexCount >= targetVertexCount)
          break;
      }

      if (b.depth < maxDepth && vertexCount < targetVertexCount) {
        for (int i = 0; i < branchFactor; ++i) {
          float theta = M_2PI * i / float(branchFactor);

          // Local frame branching
          al::Vec3f up = b.dir.normalize();
          al::Vec3f right = up.cross(al::Vec3f(0, 0, 1)).normalize();
          if (right.mag() < 0.001f)
            right = al::Vec3f(1, 0, 0);
          al::Vec3f forward = up.cross(right).normalize();

          al::Vec3f side = cos(theta) * right + sin(theta) * forward;
          al::Vec3f newDir =
              (up + side * 0.5f).normalize() * b.dir.mag() * branchScale;

          float newRadius = b.radius * 0.6f;
          stack.push_back({b.start + b.dir, newDir, newRadius, b.depth + 1});
        }
      }
    }

    if (addCanopy) {
      float canopyRadius = baseLength * pow(branchScale, maxDepth) * 1.4f;
      int canopyRings = 12;
      int canopyDetail = 24;

      for (int i = 0; i <= canopyRings; ++i) {
        float v = float(i) / canopyRings;
        float phi = M_PI * v * 0.5f;

        for (int j = 0; j < canopyDetail; ++j) {
          float u = float(j) / canopyDetail;
          float theta = M_2PI * u;

          float x = canopyRadius * sin(phi) * cos(theta);
          float y = canopyRadius * cos(phi);
          float z = canopyRadius * sin(phi) * sin(theta);

          al::Vec3f p = finalTop + al::Vec3f(x, y, z);
          m.vertex(p);
          m.color(al::Color(0.2f, 0.8f, 0.3f));
          vertexCount++;
          if (vertexCount >= targetVertexCount)
            return;
        }
      }
    }
  }
};

#endif