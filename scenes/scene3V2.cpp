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
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <string>

// MY CUSTOM INCLUDES:
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
#include "../utility/creatures.hpp"
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
  al::Nav head; // for main ribbon
  al::Nav reflectedHead;
  al::Vec3f target;
  al::Vec3f reflectedTarget;

  al::Parameter width{"Width", 0.01, 0, 0.2};

  // Meshes and Effects

  al::VAOMesh treeMesh{al::Mesh::POINTS};
  al::VAOMesh treeMesh2{al::Mesh::POINTS};
  Creature creature;
  Attractor mainAttractor;
  float updatedSpeed = 1.0;

  VertexEffectChain tree1EffectChain;
  RippleEffect ripple1;
  VertexEffectChain tree2EffectChain;
  RippleEffect ripple2;

  al::Light light;
  // Light light;
  al::Material material;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 5.0f; // Particle size

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    nav().pos(al::Vec3d(head.pos())); //

    ripple1.setParams(0.001, 0.01, 1.0, 'y');
    tree1EffectChain.pushBack(&ripple1);

    ripple2.setParams(0.0001, 0.01, 1.0, 'y');
    tree2EffectChain.pushBack(&ripple2);

    // Initialize Mesh

    creature.addTree1(treeMesh);
    treeMesh.primitive(al::Mesh::POINTS);
    // treeMesh.translate(0, -1.5, 4);
    creature.addTree2(treeMesh2);
    for (int i = 0; i < treeMesh2.vertices().size(); i++) {

      treeMesh2.vertices()[i] = {treeMesh.vertices()[i].x * 1,
                                 treeMesh.vertices()[i].y,
                                 treeMesh.vertices()[i].z * -1};
    }
    treeMesh2.primitive(al::Mesh::POINTS);
    treeMesh.translate(-5, -2.3, -3);
    treeMesh.scale(2.5);
    treeMesh2.translate(0, -2.6, 2);
    treeMesh2.scale(2.5);
    //  treeMesh.scale(5.5);
    treeMesh.update();
    treeMesh2.update();
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    head.step(dt * updatedSpeed);
    if (sceneTime < 2.0) {
      ;
    }

    if (sceneTime >= 5) {
      tree1EffectChain.process(treeMesh, sceneTime);
      tree2EffectChain.process(treeMesh2, sceneTime);
    }
    if (sceneTime >= 10) {
      mainAttractor.processDadras(treeMesh, dt, 0.1);
      mainAttractor.processDadras(treeMesh2, dt, 0.1);
    }

    treeMesh.update();
  }

  void onDraw(al::Graphics &g) override {
    glEnable(GL_BLEND);
    g.blendTrans();
    g.depthTesting(true);
    g.clear(0.0, 0.0, 0.5, 1.0);
    // g.depthTesting(true);
    g.blending(true);
    g.blendAdd(); // Additive blending for glowing effect
    g.lighting(true);
    // lighting from karl's example
    light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
    light.ambient(al::RGB(0.5, (1.0), 1.0));
    light.diffuse(al::RGB(1, 1, 0.5));
    g.light(light);
    material.specular(light.diffuse());
    material.shininess(50);
    g.material(material);

    // Use Custom Glow Shader
    // glowShader.begin();
    // glowShader.uniform("u_time", (float)globalTime);
    // glowShader.uniform("u_resolution", al::Vec2f(width(), height()));
    g.pointSize(2.5);
    g.meshColor();
    // g.color (1.0, 1.0, 1.0, 0.5);
    g.pointSize(2.0);

    g.color(1.0);
    g.draw(treeMesh);
    g.draw(treeMesh2);
    //  glowShader.end();
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
