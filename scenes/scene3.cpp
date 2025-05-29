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
  float t{0};
  float a{0};
  al::Parameter width{"Width", 0.2, 0, 0.2};

  // Meshes and Effects

  al::VAOMesh ribbon{al::Mesh::TRIANGLES};
  al::VAOMesh reflectedRibbon{al::Mesh::TRIANGLES};
  al::VAOMesh referenceMesh;
  Attractor mainAttractor;
  float updatedSpeed = 1.0;

  VertexEffectChain ribbonEffectChain;
  RippleEffect rippleZ;

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

    rippleZ.setParams(0.01, 0.1, 2.0, 'z');
    ribbonEffectChain.pushBack(&rippleZ);

    // Initialize Mesh
    // mainAttractor.makeNoiseCube(referenceMesh, 5.0, 5);
    // std::cout << referenceMesh.vertices()[0] << referenceMesh.vertices()[1]
    //           << std::endl;
    // random generated values that produced good deterministic results
    // attempting to use the smallest number
    referenceMesh.vertex(0.532166, 3.68314, -2.96492);
    referenceMesh.vertex(-1.21797, -0.745106, 2.07905);

    // al::addSphere(referenceMesh, 10.0, 10.0);
    referenceMesh.primitive(al::Mesh::POINTS);

    head.moveF(1.0);
    head.spinF(0.3);
    // head
    // target.set(al::rnd::uniformS(), al::rnd::uniformS(),
    // al::rnd::uniformS());

    ribbon.vertex(width, 0, 0);
    ribbon.normal(width, 0, 0);
    ribbon.vertex(0, 0, 0);
    ribbon.normal(0, 1, 0);
    ribbon.vertex(-width, 0, 0);
    ribbon.normal(-width, 0, 0);

    reflectedRibbon.vertex(width, 0, 0);
    reflectedRibbon.normal(width, 0, 0);
    reflectedRibbon.vertex(0, 0, 0);
    reflectedRibbon.normal(0, 1, 0);
    reflectedRibbon.vertex(-width, 0, 0);
    reflectedRibbon.normal(-width, 0, 0);

    ribbon.update();
    reflectedRibbon.update();
    referenceMesh.update();
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    // Apply Attractor Effect
    mainAttractor.processArneodo(referenceMesh, dt, 1.0);
    target = referenceMesh.vertices()[0];
    reflectedTarget = target * al::Vec3f(-1.0, 1.0, 1.0);
    // mainAttractor.processChen(ribbon, dt, 1.0);

    head.faceToward(target, 1.0);
    reflectedHead = head;
    reflectedHead.faceToward(target, 1.0);
    reflectedHead.pos(head.pos() * al::Vec3f(-1, 1, 1)); // mirror across X

    // nav().pos(al::Vec3d(head.pos()));
    //  nav().faceToward(nav().pos() * -0.5);

    //
    ribbon.vertex(head.pos() + head.ur() * width);
    ribbon.vertex(head.pos());
    ribbon.vertex(head.pos() - head.ur() * width);
    ribbon.color(1.0, 1.0, 1.0, 0.4);
    ribbon.color(1.0, 1.0, 1.0, 0.05);
    ribbon.color(1.0, 1.0, 1.0, 0.4);

    reflectedRibbon.vertex(reflectedHead.pos() + reflectedHead.ur() * width);
    reflectedRibbon.vertex(reflectedHead.pos());
    reflectedRibbon.vertex(reflectedHead.pos() - reflectedHead.ur() * width);
    reflectedRibbon.color(1.0, 1.0, 1.0, 0.4);
    reflectedRibbon.color(1.0, 1.0, 1.0, 0.05);
    reflectedRibbon.color(1.0, 1.0, 1.0, 0.4);

    //     if ((int)(globalTime * 10) % 50 == 0) {
    //     addBranch(ribbon, head, width);
    // }

    if (true) {
      // sort of curved surface shading
      ribbon.normal(al::Vec3f(head.uu()).lerp(head.ur(), 0.1));
      ribbon.normal(head.uu());
      ribbon.normal(al::Vec3f(head.uu()).lerp(head.ur(), -0.1));
    } else {
      // flat shading....
      ribbon.normal(head.uu());
      ribbon.normal(head.uu());
      ribbon.normal(head.uu());
    }
    // MAKE ALL OF THESE INTO LOOPS
    const int n = ribbon.vertices().size();
    if (n > 3) {
      ribbon.index(n - 1);
      ribbon.index(n - 2);
      ribbon.index(n - 4);
      ribbon.index(n - 2);
      ribbon.index(n - 5);
      ribbon.index(n - 4);
      ribbon.index(n - 2);
      ribbon.index(n - 3);
      ribbon.index(n - 5);
      ribbon.index(n - 3);
      ribbon.index(n - 6);
      ribbon.index(n - 5);

      reflectedRibbon.index(n - 1);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 4);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 5);
      reflectedRibbon.index(n - 4);
      reflectedRibbon.index(n - 2);
      reflectedRibbon.index(n - 3);
      reflectedRibbon.index(n - 5);
      reflectedRibbon.index(n - 3);
      reflectedRibbon.index(n - 6);
      reflectedRibbon.index(n - 5);
    }

    head.step(dt * updatedSpeed);
    if (sceneTime < 2.0) {
      // ribbon.scale(1.1);
      ribbon.scale(1.001);
      reflectedRibbon.scale(1.001);
    }

    // ribbonEffectChain.process(ribbon, sceneTime);
    // ribbonEffectChain.process(reflectedRibbon, sceneTime);
    // ribbon.scale(1.0001);
    ribbon.update();

    // reflectedRibbon.scale(1.000001);
    // reflectedHead.step(dt * updatedSpeed);
    // reflectedRibbon.scale(1.001);
    reflectedRibbon.update();

    referenceMesh.update();
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
    g.pointSize(10.0);
    g.draw(referenceMesh);
    g.draw(ribbon);
    g.draw(reflectedRibbon);
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
