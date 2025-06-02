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
#include <fstream>
#include <iostream>
#include <ostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/orbit.hpp"
#include "../eoys-mesh-fx/ripple.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxMain.hpp"
#include "../eoys-mesh-fx/vfxUtility.hpp"
#include "../utility/attractors.hpp"
#include "../utility/imageColorToMesh.hpp"
#include "../utility/loadAudioScene.hpp"
#include "../utility/parseObj.hpp"
#include "../utility/soundObject.hpp"

std::string slurp(const std::string &fileName);

class MyApp : public al::DistributedApp {
public:
  al::ShaderProgram pointShader;
  std::string pointFragPath, pointVertPath, pointGeomPath;

  al::Light light;
  al::Material material;
  al::FileSelector selector;
  al::SearchPaths searchPaths;

  Attractor referenceAttractor;
  al::VAOMesh cloudMeshScene4;

  al::Parameter scene3PointSize{"scene3PointSize", "", 0.0, 0.0, 0.03};
  al::Parameter scene3ParamB{"scene3paramB", "", 0.11, 0.0, 1.0};
  al::Parameter sceneTime{"sceneTime", "", 0.0, 0.0, 300.0};

  al::ParameterBool running{"running", "0", false};

  double globalTime = 0;

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());
    searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");

    pointFragPath = searchPaths.find("point-fragment.glsl").filepath();
    pointGeomPath = searchPaths.find("point-geometry.glsl").filepath();
    pointVertPath = searchPaths.find("point-vertex.glsl").filepath();

    parameterServer() << scene3PointSize << scene3ParamB << sceneTime;
  }
  bool onKeyDown(const al::Keyboard &k) override {

    if (k.key() == ' ' && running == false) {
      running = true;
      std::cout << "started running" << std::endl;
    } else if (k.key() == ' ' && running == true) {
      running = false;
      std::cout << "stopped running" << std::endl;
    }
  }

  void onCreate() override {
    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));

    al::addSphere(cloudMeshScene4, 3.5, 200, 200); // make 150 150 if buggy
    cloudMeshScene4.primitive(al::Mesh::POINTS);
    for (auto &v : cloudMeshScene4.vertices()) {
      cloudMeshScene4.color(1.0, 0.3, 0.1, 1.0);
      cloudMeshScene4.texCoord(1.0, 0.0);
    }
    cloudMeshScene4.translate(-1, 1, -7.0);
    cloudMeshScene4.update();
  }

  void onAnimate(double dt) override {
    if (running == true) {
      globalTime += dt;
      sceneTime = sceneTime + dt;

      for (auto &v : cloudMeshScene4.vertices()) {
        v += al::Vec3f(0, 0, -0.02);
      }

      if (sceneTime <= 22.0) {
        scene3PointSize = scene3PointSize + 0.000015;
        referenceAttractor.processThomas(cloudMeshScene4, dt, 0.2f,
                                         scene3ParamB);
      } else if (sceneTime <= 26.0) {
        scene3PointSize = 0.015;
        referenceAttractor.processThomas(cloudMeshScene4, dt, -0.15f,
                                         scene3ParamB);
      } else if (sceneTime <= 47.0) {
        scene3PointSize = 0.015;
        referenceAttractor.processLorenz(cloudMeshScene4, dt, 0.1f);
      } else if (sceneTime <= 70.0) {
        scene3PointSize = 0.015;
        referenceAttractor.processThomas(cloudMeshScene4, dt, 0.3f,
                                         scene3ParamB);
      } else if (sceneTime <= 80.0) {
        scene3PointSize = 0.015;
        referenceAttractor.processThomas(cloudMeshScene4, dt, 0.2f,
                                         scene3ParamB);
      } else if (sceneTime <= 93.0) {
        scene3PointSize = 0.015;
        referenceAttractor.processThomas(cloudMeshScene4, dt, 0.4f,
                                         scene3ParamB * 2);
      } else if (sceneTime <= 110.0) {
        referenceAttractor.processRabinovich(cloudMeshScene4, dt, 0.01f, 0.14f,
                                             0.10f);
        scene3PointSize = std::max(scene3PointSize - 0.00001, 0.0);
      } else {
        scene3PointSize = 0.0;
      }

      cloudMeshScene4.update();
    }
  }

  void onDraw(al::Graphics &g) override {
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
  return std::string((std::istreambuf_iterator<char>(file)),
                     std::istreambuf_iterator<char>());
}
