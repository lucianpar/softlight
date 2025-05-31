#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <ostream>
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
#include "softlight-sphere-new/softlight/softlight/eoys-shader/shaderToSphere.hpp"

/*
* sequence
* work on removing oldest mesh vertices
*update world color to flow into next scene
* add more mirrors
*figure out spacings

*/

struct Common {};
class MyApp : public al::DistributedAppWithState<Common> {
public:
  al::FileSelector selector;
  al::SearchPaths searchPaths;

  // scene 5
  std::vector<al::Nav> structures;
  ShadedSphere shadedSphere;
  std::string vertPathScene5;
  std::string fragPathScene5;
  bool initFlag = true;

  al::Parameter width{"Width", 0.05, 0, 0.2};
  int nAgentsScene4 = 400;

  // Meshes and Effects
  al::VAOMesh meshBall;
  float ballSpeedScene4 = 0.5;
  float ballSizeScene5 = 0.03;
  // al::VAOMesh smallMesh1;
  // al::VAOMesh smallMesh2;
  // al::VAOMesh smallMesh3;
  // al::VAOMesh smallMesh4;
  al::VAOMesh referenceMesh;
  Attractor referenceAttractor;

  al::Light light;
  // Light light;
  al::Material material;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 1.0f; // Particle size

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    // scene 5
    searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");

    al::FilePath vertPath5 = searchPaths.find("standard.vert");
    if (vertPath5.valid()) {
      vertPathScene5 = vertPath5.filepath();
      std::cout << "Found file at: " << vertPathScene5 << std::endl;
    } else {
      std::cout << "couldnt find basemesh in path" << std::endl;
    }
    al::FilePath fragPath5 = searchPaths.find("FoamMain.frag");
    if (fragPath5.valid()) {
      fragPathScene5 = fragPath5.filepath();
      std::cout << "Found file at: " << fragPathScene5 << std::endl;
    } else {
      std::cout << "couldnt find basemesh in path" << std::endl;
    }
  }

  void onCreate() override {
    // nav().pos(al::Vec3d(head.pos())); //
    shadedSphere.setSphere(15.0, 20);

    // shadedSphere.update();

    // shadedSphere.setShaders(
    //     "/Users/lucian/Desktop/201B/allolib_playground/"
    //     "softlight-sphere-new/softlight/softlight/eoys-shader/"
    //     "standard.vert",
    //     "/Users/lucian/Desktop/201B/allolib_playground/softlight-sphere-new/"
    //     "softlight/softlight/eoys-shader/Psych1.frag");

    shadedSphere.update(); // al::addSphere(test);

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
    al::addIcosphere(meshBall, ballSizeScene5, 1);
    meshBall.primitive(al::Mesh::LINE_LOOP);
    meshBall.update();

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

    // smallMesh1.translate(referenceMesh.vertices()[0]);
    // smallMesh1.update();
    // al::addIcosphere(smallMesh2);
    // smallMesh2.translate(referenceMesh.vertices()[1]);
    // smallMesh2.update();
    // al::addIcosphere(smallMesh3);
    // smallMesh3.update();
    // smallMesh3.translate(referenceMesh.vertices()[2]);
    // al::addIcosphere(smallMesh4);
    // smallMesh4.translate(referenceMesh.vertices()[3]);
    // smallMesh4.update();

    // al::addSphere(referenceMesh, 10.0, 10.0);
    referenceMesh.primitive(al::Mesh::POINTS);

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

    referenceAttractor.processChenLee(referenceMesh, dt, ballSpeedScene4);
    referenceMesh.update();

    for (int i = 0; i < structures.size(); ++i) {
      structures[i].faceToward(referenceMesh.vertices()[i]);
      structures[i].moveF(ballSpeedScene4);
      structures[i].step(dt);
    }
  }

  void onDraw(al::Graphics &g) override {
    g.clear(0.0);
    if (initFlag) {
      // Just like ShaderEngine does:
      shadedSphere.setShaders(vertPathScene5, fragPathScene5);
      initFlag = false;
    }

    g.shader(shadedSphere.shader());

    shadedSphere.setUniformFloat("u_time", sceneTime);
    shadedSphere.setUniformFloat("onset", 0.0f);
    shadedSphere.setUniformFloat("cent", 1000.0f);
    shadedSphere.setUniformFloat("flux", 0.5f);

    shadedSphere.draw(g);

    // // Draw all your Nav structures
    // g.pointSize(pointSize);
    // g.color(0.0, 0.0, 0.7, 0.5);
    // for (int i = 0; i < structures.size(); ++i) {
    //   g.pushMatrix();
    //   g.translate(structures[i].pos());
    //   g.rotate(structures[i].quat());
    //   g.draw(meshBall);
    //   g.popMatrix();
    // }
    // // g.draw(referenceMesh);
    // // g.draw(ribbon);
    // // g.draw(reflectedRibbon);
    // //   glowShader.end();
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
