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
#include "al/io/al_File.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/sound/al_SoundFile.hpp"
#include "al/sound/al_Spatializer.hpp"
#include "al/sphere/al_SphereUtils.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include "al_ext/statedistribution/al_CuttleboneDomain.hpp"
#include <iostream>
#include <string>

// spatial includes
#include "al/sound/al_Ambisonics.hpp"
#include "al/sound/al_Lbap.hpp"
#include "al/sound/al_Speaker.hpp"
#include "al/sound/al_Vbap.hpp"
#include "al/sphere/al_AlloSphereSpeakerLayout.hpp"

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/ripple.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxUtility.hpp"
#include "al_ext/soundfile/al_SoundfileBuffered.hpp"
// #include "../meshMorph.hpp"
#include "../eoys-mesh-fx/scatter.hpp"
#include "../eoys-mesh-fx/vfxMain.hpp"
#include "../utility/loadAudioScene.hpp"
#include "../utility/parseObj.hpp"
#include "../utility/staticSoundObject.hpp"
// #include "../imageToMesh.hpp"
// #include "../imageToSphere.hpp"
#include "../utility/attractors.hpp"
#include "../utility/imageColorToMesh.hpp"

std::string slurp(const std::string &fileName);

struct Common {};

class MyApp : public al::DistributedAppWithState<Common> {

  std::shared_ptr<al::CuttleboneDomain<Common>> cuttleboneDomain;

  al::FileSelector selector;
  al::SearchPaths searchPaths;

  ////spatial
  //
  al::Spatializer *spatializer{nullptr};
  al::Speakers speakerLayout;

  // al::ParameterBool pRunning{"running"};
  // al::Parameter pTime{"time", "", 0.0, 0.0, 10000}; 0ld stuff
  // al::ParameterInt pIndex{"index", "", 0, 0, 100};

  al::ParameterBool running{"running", "0", false};
  al::Parameter sceneTime{"sceneTime", "0", 0.0, 0.0, 10000};
  // al::Parameter sceneIndex{"sceneTime", "0", 0.0, 0.0, 10000};

  //
public:
  // Global Time
  int sceneIndex = 0;
  // int previousIndex = 0;
  double globalTime = 0;
  float localTime;
  // float sceneIndex = 0;
  // double sceneTime = 64;
  // bool running = false;
  std::string objPath;
  std::string Song1Path;
  std::string Song2Path;
  std::string Song3Path;
  std::string Song4Path;
  std::string Song5Path;
  std::string Song6Path;
  std::string pointFragPath;
  std::string pointVertPath;
  std::string pointGeomPath;
  al::ShaderProgram pointShader;

  // SCENE DECLERATIONS

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    parameterServer() << running << sceneTime;

    // SPATIAL STUFF
    audioIO().channelsBus(1);
    if (al::sphere::isSphereMachine())
      speakerLayout = al::AlloSphereSpeakerLayoutCompensated();
    else
      speakerLayout = al::StereoSpeakerLayout();

    spatializer = new al::AmbisonicsSpatializer(speakerLayout); //, 3, 1, 1);
    spatializer->compile();

    // FILE PATH STUFF
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

    al::FilePath objFP = searchPaths.find("floatBody.obj");
    if (objFP.valid()) {
      objPath = objFP.filepath();
      std::cout << "Found file at: " << objPath << std::endl;
    } else {
      std::cout << "couldnt find basemesh in path" << std::endl;
    }
    if (isPrimary()) {
      al::FilePath audio1Fp = searchPaths.find("Song1.wav");
      if (audio1Fp.valid()) {
        Song1Path = audio1Fp.filepath();
        std::cout << "Found file at: " << Song1Path << std::endl;
      } else {
        std::cout << "couldnt find song1 in path" << std::endl;
      }
      al::FilePath audio2Fp = searchPaths.find("Song2.wav");
      if (audio2Fp.valid()) {
        Song2Path = audio2Fp.filepath();
        std::cout << "Found file at: " << Song2Path << std::endl;
      } else {
        std::cout << "couldnt find song2 in path" << std::endl;
      }
      al::FilePath audio3Fp = searchPaths.find("Song3.wav");
      if (audio3Fp.valid()) {
        Song3Path = audio3Fp.filepath();
        std::cout << "Found file at: " << Song3Path << std::endl;
      } else {
        std::cout << "couldnt find song3 in path" << std::endl;
      }
      al::FilePath audio4Fp = searchPaths.find("Song4.wav");
      if (audio4Fp.valid()) {
        Song4Path = audio4Fp.filepath();
        std::cout << "Found file at: " << Song4Path << std::endl;
      } else {
        std::cout << "couldnt find song4 in path" << std::endl;
      }
      al::FilePath audio5Fp = searchPaths.find("Song5.wav");
      if (audio5Fp.valid()) {
        Song5Path = audio5Fp.filepath();
        std::cout << "Found file at: " << Song5Path << std::endl;
      } else {
        std::cout << "couldnt find song5 in path" << std::endl;
      }
      al::FilePath audio6Fp = searchPaths.find("Song6.wav");
      if (audio6Fp.valid()) {
        Song6Path = audio6Fp.filepath();
        std::cout << "Found file at: " << Song6Path << std::endl;
      } else {
        std::cout << "couldnt find song6 in path" << std::endl;
      }
    }
  }