#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <ostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/orbit.hpp"
#include "../eoys-mesh-fx/ripple.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxUtility.hpp"
// #include "../meshMorph.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxMain.hpp"
#include "../utility/loadAudioScene.hpp"
#include "../utility/parseObj.hpp"
#include "../utility/soundObject.hpp"
// #include "../imageToMesh.hpp"
// #include "../imageToSphere.hpp"
#include "../utility/attractors.hpp"
#include "../utility/imageColorToMesh.hpp"

/*
* sequence
* work on removing oldest mesh vertices
*update world color to flow into next scene
* add more mirrors
*figure out spacings

*/

class MyApp : public al::App {
public:
  al::Light light;
  // Light light;
  al::Material material;
  std::vector<al::Nav> structures;

  // al::Parameter width{"Width", 0.05, 0, 0.2};
  int nAgentsScene4 = 50;

  // Meshes and Effects
  al::VAOMesh meshBall;
  float ballSpeedScene4 = 0.5;
  al::VAOMesh referenceMesh;
  Attractor referenceAttractor;
  al::VAOMesh sunMesh;
  al::VAOMesh scene4ShellMesh;
  VertexEffectChain sunEffectChain;
  OrbitEffect sunOrbit1;
  OrbitEffect sunOrbit2;
  Attractor bigAttractor;
  // Global Time
  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 5.0f; // Particle size

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    // nav().pos(al::Vec3d(head.pos())); //

    // al::addSphere(test);

    // Initialize Mesh
    referenceAttractor.makeNoiseCube(referenceMesh, 5.0, nAgentsScene4);
    referenceMesh.update();
    // newAttractor.primitive(al::Mesh::LINE_LOOP);
    // newAttractor.update();
    // std::cout << referenceMesh.vertices()[0] << referenceMesh.vertices()[1]
    //           << std::endl;
    // random generated values that produced good deterministic results
    // attempting to use the smallest number
    // referenceMesh.vertex(0.532166, 3.68314, -2.96492);
    // referenceMesh.vertex(-1.21797, -0.745106, 2.07905);
    al::addIcosphere(meshBall, 0.05, 1);
    meshBall.primitive(al::Mesh::LINE_LOOP);
    meshBall.update();

    al::addSphere(sunMesh, 1.0, 30, 30);
    for (int i = 0; i < sunMesh.vertices().size(); i++) {
      sunMesh.color(0.012, 0.369, 0.31);
    }
    sunMesh.translate(0, 0, 5);
    sunMesh.primitive(al::Mesh::LINES);
    sunMesh.update();
    al::addSphere(scene4ShellMesh, 10.0, 100, 100);
    for (int i = 0; i < scene4ShellMesh.vertices().size(); i++) {
      scene4ShellMesh.color(0.012, 0.369, 0.31);
    }
    scene4ShellMesh.primitive(al::Mesh::LINE_LOOP);
    scene4ShellMesh.update();

    std::cout << "totalVerts: " << meshBall.vertices().size() * nAgentsScene4
              << std::endl;

    for (int b = 0; b < nAgentsScene4; ++b) {
      al::Nav p;
      p.pos() = referenceMesh.vertices()[b];
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      // p.set(randomVec3f(5), randomVec3f(1));
      structures.push_back(p);
      // velocity.push_back(al::Vec3f(0));
      // force.push_back(al::Vec3f(0));
    }
    // al::addSphere(referenceMesh, 10.0, 10.0);
    referenceMesh.primitive(al::Mesh::POINTS);

    sunOrbit1.setParams(0.3, 4.0, {0, 0, 0}, 1);
    sunOrbit2.setParams(0.1, 6.0, {0, 0, 0}, 2);
    sunEffectChain.pushBack(&sunOrbit1);
    sunEffectChain.pushBack(&sunOrbit2);

    // target.set(al::rnd::uniformS(), al::rnd::uniformS(),
    // al::rnd::uniformS());

    // makePointLine()
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    /// trying newer effect
    // referenceAttractor.processRossler(newAttractor, dt, 1.0);
    // referenceAttractor.processLorenz(newAttractor, dt, 1.0);

    referenceAttractor.processArneodo(referenceMesh, dt, ballSpeedScene4);
    referenceMesh.update();

    for (int i = 0; i < structures.size(); ++i) {
      structures[i].faceToward(referenceMesh.vertices()[i]);
      structures[i].moveF(ballSpeedScene4);
      structures[i].step(dt);
    }

    // sunEffectChain.process(sunMesh, sceneTime); // toggle orbiting
    bigAttractor.processBlackHoleSpiral(scene4ShellMesh, dt, 1, 3);
    scene4ShellMesh.update();
  }

  void onDraw(al::Graphics &g) override {
    glEnable(GL_BLEND);
    // g.blendTrans();
    g.depthTesting(true);

    g.clear(0.0);

    // g.depthTesting(true);
    g.blending(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // g.blendTrans();
    g.depthTesting(true);

    g.lighting(true);
    // lighting from karl's example
    light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
    light.ambient(al::RGB(1.0, 1.0, 1.0));
    light.diffuse(al::RGB(1, 1, 1.0));
    g.light(light);
    material.specular(1.0);
    material.shininess(10);
    g.material(material);

    g.pointSize(pointSize);
    // g.color(0.871, 0.467, 0.192, 0.1);
    for (int i = 0; i < structures.size(); ++i) {
      g.pushMatrix();
      g.color(0.871, 0.467, 0.192, 0.1);
      g.translate(structures[i].pos());
      g.rotate(structures[i].quat());

      g.draw(meshBall);
      g.popMatrix();
    }
    g.meshColor();
    // g.draw(sunMesh);
    g.draw(scene4ShellMesh);
    // g.draw(referenceMesh);
    // g.draw(ribbon);
    // g.draw(reflectedRibbon);
    //   glowShader.end();
  }

  void onSound(al::AudioIOData &io) override { mSequencer.render(io); }
  al::SynthSequencer &sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
