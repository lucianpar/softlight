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
#include "../eoys-shader/shaderToSphere.hpp"
#include "../utility/attractors.hpp"
#include "../utility/imageColorToMesh.hpp"

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
  al::Light light;
  // Light light;
  al::Material material;
  ShadedSphere shadedSphereScene3;

  al::Parameter sceneTime{"sceneTime", "0", 0.0, 0.0, 300.0};

  al::ParameterBool running{"running", "0", false};

  // scene 4

  std::string vertPathScene3;
  std::string fragPathScene3;

  // Meshes and Effects

  // Global Time
  double globalTime = 0;
  // double sceneTime = 0.0;
  int sceneIndex = 4;

  void onInit() override {

    parameterServer() << sceneTime;

    gam::sampleRate(audioIO().framesPerSecond());

    searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");

    // scene 3

    al::FilePath vertPath3 = searchPaths.find("standard.vert");
    if (vertPath3.valid()) {
      vertPathScene3 = vertPath3.filepath();
      std::cout << "Found file at: " << vertPathScene3 << std::endl;
    } else {
      std::cout << "couldnt find ver scene 4 in path" << std::endl;
    }
    al::FilePath fragPath3 =
        searchPaths.find("offer1.frag"); // switch to spruce main
    if (fragPath3.valid()) {
      fragPathScene3 = fragPath3.filepath();
      std::cout << "Found file at: " << fragPathScene3 << std::endl;
    } else {
      std::cout << "couldnt find frag scene 4 in path" << std::endl;
    }
  }
  bool onKeyDown(const al::Keyboard &k) override {

    if (isPrimary()) {

      if (k.key() == ' ' && running == false) {
        running = true;
        std::cout << "started running" << std::endl;
      } else if (k.key() == ' ' && running == true) {
        running = false;
        std::cout << "stopped running" << std::endl;
      }
    }
  }

  void onCreate() override {

    // scene 4

    // if (sceneIndex == 4) {
    shadedSphereScene3.setSphere(15.0, 20);
    shadedSphereScene3.setShaders(vertPathScene3, fragPathScene3);
    shadedSphereScene3.update();
    // }
  }

  void onAnimate(double dt) override {

    if (running == true) {

      globalTime += dt;
      sceneTime = sceneTime + dt;
      std::cout << globalTime << std::endl;
    }
  }

  void onDraw(al::Graphics &g) override {

    // scene 4
    g.clear(0.0);

    g.shader(shadedSphereScene3.shader());
    shadedSphereScene3.setUniformFloat("u_time", sceneTime);

    shadedSphereScene3.draw(g);
  }

  // scene 4
  void onSound(al::AudioIOData &io) override { mSequencer.render(io); }
  al::SynthSequencer &sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;
  app.start();
  return 0;
}
