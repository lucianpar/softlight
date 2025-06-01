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

*/
// boilerplate
std::string slurp(const std::string &fileName);

class MyApp : public al::DistributedApp {
public:
  al::ShaderProgram pointShader;
  std::string pointFragPath;
  std::string pointVertPath;
  std::string pointGeomPath;

  al::Light light;
  al::Material material;

  al::FileSelector selector;
  al::SearchPaths searchPaths;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;

  //////

  // scene 4 mesh stuff
  Attractor referenceAttractor;
  al::VAOMesh cloudMeshScene4;
  al::VAOMesh cloudMesh2Scene4;

  // scene 4 params
  double scene3PointSize = 0.0; // sweet spot0.015;
  float scene4paramB = 0.11;

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");
    al::FilePath frag = searchPaths.find("point-fragment.glsl");
    if (frag.valid()) {
      pointFragPath = frag.filepath();
      std::cout << "Found file at: " << pointFragPath << std::endl;
    } else {
      std::cout << "couldnt find point frag in path" << std::endl;
    }
    al::FilePath geom = searchPaths.find("point-geometry.glsl");
    if (frag.valid()) {
      pointGeomPath = geom.filepath();
      std::cout << "Found file at: " << pointGeomPath << std::endl;
    } else {
      std::cout << "couldnt find point geom in path" << std::endl;
    }
    al::FilePath vert = searchPaths.find("point-vertex.glsl");
    if (vert.valid()) {
      pointVertPath = vert.filepath();
      std::cout << "Found file at: " << pointVertPath << std::endl;
    } else {
      std::cout << "couldnt find point vert in path" << std::endl;
    }
  }

  void onCreate() override {
    // boilerplate

    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));

    // Initialize Mesh

    al::addSphere(cloudMeshScene4, 3.5, 150, 150);

    std::cout << cloudMeshScene4.vertices().size() << std::endl;
    cloudMeshScene4.primitive(al::Mesh::POINTS);
    for (int i = 0; i < cloudMeshScene4.vertices().size(); ++i) {
      cloudMeshScene4.color(1.0, 0.3, 0.1, 1.0); // or any RGBA
      cloudMeshScene4.texCoord(1.0, 0.0);        // sets vertexSize.x
    }
    cloudMeshScene4.translate(-1, 1, -7.0);
    cloudMeshScene4.update();
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;
    std::cout << sceneTime << std::endl;

    // SCENE 3 ANIMATE ///

    // === camera drift ===
    for (auto &v : cloudMeshScene4.vertices()) {
      v += al::Vec3f(0, 0, -0.02);
    }

    // === ATTRACTOR TIMING SEQUENCE ===

    // Time markers (could be constants or member variables)
    double s3marker0 = 0.0;
    double s3marker1 = 22.0;
    double s3marker2 = 26.0;
    double s3marker3 = 47.0;
    double s3marker4 = 70.0;
    double s3marker5 = 80.0;
    double s3marker6 = 93.0;
    double s3marker7 = 110.0;

    if (sceneTime >= s3marker0 && sceneTime <= s3marker1) {
      // 0:00–0:22 — thomas (unraveling)
      if (scene3PointSize <= 0.015) {
        scene3PointSize += 0.000015;
      } else {
        scene3PointSize = 0.015;
      }
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.2f, scene4paramB);
    }

    if (sceneTime > s3marker1 && sceneTime <= s3marker2) {
      // 0:22–0:26 — thomas slows
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, -0.15f,
                                       scene4paramB);
    }

    if (sceneTime > s3marker2 && sceneTime <= s3marker3) {
      // 0:27–0:47 — lorenz (different behavior)
      scene3PointSize = 0.015;
      referenceAttractor.processLorenz(cloudMeshScene4, dt, 0.1f);
    }

    if (sceneTime > s3marker3 && sceneTime <= s3marker4) {
      // 0:47–1:10 — thomas (new behavior)
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.3f, scene4paramB);
    }

    if (sceneTime > s3marker4 && sceneTime <= s3marker5) {
      // 1:10–1:20 — thomas slows
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.2f, scene4paramB);
    }

    if (sceneTime > s3marker5 && sceneTime <= s3marker6) {
      // 1:20–1:33 — thomas rapid
      scene3PointSize = 0.015;
      referenceAttractor.processThomas(cloudMeshScene4, dt, 0.4,
                                       scene4paramB * 2);
    }

    if (sceneTime > s3marker6 && sceneTime <= s3marker7) {
      // 1:33–1:49 — rabinovich fade
      referenceAttractor.processRabinovich(cloudMeshScene4, dt, 0.01f, 0.14f,
                                           0.10f);
      scene3PointSize -= 0.00001;
    }
    if (sceneTime >= 109) {
      scene3PointSize = 0.0;
    }
    cloudMeshScene4.update();

    // SCENE END 3 ANIMATE ///
  }

  void onDraw(al::Graphics &g) override {

    // SCENE 3 DRAW

    g.depthTesting(true);

    g.clear(0, 0.1, 0.4);
    g.blending(true);
    g.blendAdd();

    g.shader(pointShader);
    pointShader.uniform("pointSize", scene3PointSize);
    pointShader.uniform("inputColor", al::Vec4f(1.0, 1.0, 1.0, 1.0));

    g.draw(cloudMeshScene4);
    g.pushMatrix();
    g.scale(1, 1, -1);
    g.draw(cloudMeshScene4);
    g.popMatrix();

    // SCENE 4 DRAW
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