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
// boilerplate
std::string slurp(const std::string &fileName);

class MyApp : public al::App {
public:
  al::ShaderProgram pointShader;

  al::Light light;
  // Light light;
  al::Material material;

  // al::Parameter width{"Width", 0.05, 0, 0.2};
  // Meshes and Effects

  al::VAOMesh meshBall;
  float ballSpeedScene4 = 0.5;

  Attractor referenceAttractor;
  al::VAOMesh cloudMeshScene4;
  al::VAOMesh cloudMesh2Scene4;

  // scene 4 params
  double scene3PointSize = 0.0; // sweet spot0.015;
  float scene4paramB = 0.11;

  // scene 4 sequencing
  double scene4event_decreaseB = 3.0;
  double scene4event_increaseB = 20.0;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    // boilerplate
    pointShader.compile(slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-vertex.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/scene3-point-fragment.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-geometry.glsl"));
    //
    // nav().pos(al::Vec3d(head.pos())); //

    // al::addSphere(test);

    // Initialize Mesh

    // referenceAttractor.makeNoiseCube(cloudMeshScene4, 5.0, 20000);
    al::addSphere(cloudMeshScene4, 3.5, 150, 150);

    std::cout << cloudMeshScene4.vertices().size() << std::endl;
    cloudMeshScene4.primitive(al::Mesh::POINTS);
    for (int i = 0; i < cloudMeshScene4.vertices().size(); ++i) {
      cloudMeshScene4.color(1.0, 0.3, 0.1, 1.0); // or any RGBA
      cloudMeshScene4.texCoord(1.0, 0.0);        // sets vertexSize.x
    }
    cloudMeshScene4.translate(-1, 1, -7.0);
    // cloudMeshScene4.update(); // reupload with new attribs
    cloudMeshScene4.update();

    // al::addIcosphere(meshBall, 0.02, 1);
    // meshBall.primitive(al::Mesh::LINE_LOOP);
    // meshBall.update();

    // scene4ShellMesh.primitive(al::Mesh::LINE_LOOP);
    // scene4ShellMesh.update();
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;
    std::cout << sceneTime << std::endl;

    // === camera drift ===
    for (auto &v : cloudMeshScene4.vertices()) {
      v += al::Vec3f(0, 0, -0.02);
    }

    // === ATTRACTOR TIMING SEQUENCE ===

    if (sceneTime <= 22.0) {
      if (scene3PointSize <= 0.015) {
        scene3PointSize += 0.000015;
      } else {
        scene3PointSize = 0.015;
      }
      // 0:00–0:22 — thomas (unraveling)
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.3f, scene4paramB);
    } else if (sceneTime <= 26.0) {
      // 0:22–0:26 — thomas slows
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, -0.2f,
                                       scene4paramB);
    } else if (sceneTime <= 47.0) {
      scene3PointSize = 0.015;
      // 0:27–0:47 — lorenz (different behavior)
      referenceAttractor.processLorenz(cloudMeshScene4, dt, 0.1f);
    } else if (sceneTime < 70.0) {
      scene3PointSize = 0.015;
      // 0:47–1:10 — thomas (new behavior)
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.3f, scene4paramB);
      // scene4paramB -= 0.001;
    } else if (sceneTime < 80.0) {
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.2f, scene4paramB);
    } else if (sceneTime < 93.0) {
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.4,
                                       scene4paramB * 2);
    } else if (sceneTime < 110.0) {
      referenceAttractor.processRabinovich(cloudMeshScene4, dt, 0.01f, 0.14f,
                                           0.10f);

      // referenceAttractor.processThomas(cloudMeshScene4, dt, 0.1f,
      // scene4paramB);
      scene3PointSize -= 0.00001;
    }
    //  else if (globalTime < 80.0) {
    //   // 1:10–1:20 — thomas slows
    //   // referenceAttractor.processThomas(cloudMeshScene4, dt, 0.05f,
    //   //
    // else if (sceneTime < 93.0) {
    //   // 1:20–1:33 — lorenz again
    //   referenceAttractor.processLorenz(cloudMeshScene4, dt, -0.2f);
    //   scene3PointSize -= 0.00001;
    // }

    // } else if (globalTime >= 110) {
    //   // scene3PointSize -= 0.00001;
    // }

    cloudMeshScene4.update();

    // === original B modulation logic ===
    // if (sceneTime >= scene4event_decreaseB &&
    //     sceneTime <= scene4event_increaseB) {
    //   scene4paramB -= 0.00001;
    // } else if (sceneTime >= scene4event_increaseB) {
    //   scene4paramB += 0.00001;
    // }
  }

  void onDraw(al::Graphics &g) override {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    g.depthTesting(true);

    g.clear(0, 0.1, 0.4);
    g.blending(true);
    g.blendAdd();

    g.shader(pointShader);
    pointShader.uniform("pointSize", scene3PointSize);
    pointShader.uniform("inputColor", al::Vec4f(1.0, 1.0, 1.0, 1.0));

    // === Center Attractor (Front) ===
    g.draw(cloudMeshScene4);

    g.pushMatrix();
    g.scale(1, 1, -1);
    g.draw(cloudMeshScene4);
    g.popMatrix();
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

// boilerplate
std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}