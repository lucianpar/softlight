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
#include "../eoys-mesh-fx/vfxMain.hpp"
#include "../eoys-mesh-fx/vfxUtility.hpp"
#include "../utility/attractors.hpp"
#include "../utility/creatures.hpp"
#include "../utility/imageColorToMesh.hpp"
#include "../utility/loadAudioScene.hpp"
#include "../utility/parseObj.hpp"
#include "../utility/soundObject.hpp"

std::string slurp(const std::string &fileName);

class MyApp : public al::App {

public:
  al::ShaderProgram pointShader;
  al::Nav head;
  al::Nav reflectedHead;
  al::Vec3f target;
  al::Vec3f reflectedTarget;

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
  al::Material material;

  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 5.0f;

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    nav().pos(al::Vec3d(head.pos()));

    ripple1.setParams(0.001, 0.01, 1.0, 'y');
    tree1EffectChain.pushBack(&ripple1);

    ripple2.setParams(0.0001, 0.01, 1.0, 'y');
    tree2EffectChain.pushBack(&ripple2);

    creature.addTree1(treeMesh);
    treeMesh.primitive(al::Mesh::POINTS);
    for (int i = 0; i < treeMesh.vertices().size(); ++i) {
      treeMesh.texCoord(1.0, 0.0);
    }

    creature.addTree2(treeMesh2);
    treeMesh2.primitive(al::Mesh::POINTS);
    for (int i = 0; i < treeMesh2.vertices().size(); ++i) {
      treeMesh2.vertices()[i] = {treeMesh.vertices()[i].x * 1,
                                 treeMesh.vertices()[i].y,
                                 treeMesh.vertices()[i].z * -1};
      treeMesh2.texCoord(1.0, 0.0);
    }

    treeMesh.translate(-5, -2.3, -3);
    treeMesh.scale(2.5);
    treeMesh2.translate(0, -2.6, 2);
    treeMesh2.scale(2.5);
    treeMesh.update();
    treeMesh2.update();

    pointShader.compile(slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-vertex.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-fragment.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-geometry.glsl"));
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    head.step(dt * updatedSpeed);

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
    g.clear(0, 0, 0.4);
    g.shader(pointShader);
    pointShader.uniform("pointSize", 0.001);
    g.blending(true);
    g.blendTrans();
    g.depthTesting(true);
    g.draw(treeMesh);
    g.draw(treeMesh2);
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

std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}