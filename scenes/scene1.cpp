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

struct Common {
  // float sceneTimeState;
};

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

  al::ParameterBool runningParam{"runningParam", "0", false};
  // al::Parameter sceneTime{"sceneTime", "0", 0.0, 0.0, 10000};
  // al::Parameter sceneIndex{"sceneTime", "0", 0.0, 0.0, 10000};

  al::Parameter sceneTimeParam{"sceneTime", "", 0.0f, 0.0f, 2000.0f};

  //

public:
  // Global Time
  int sceneIndex = 0;
  // int previousIndex = 0;
  double globalTime = 0;
  float localTime;
  float sceneTime = 0.0;
  bool running = false;

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

  ////SCENE 1 DECLARE  //////

  // SCENE 1 params

  float rippleAmplitudeScene1 = 0.0;
  float rippleSpeedXScene1 = 0.2;
  float rippleSpeedYScene1 = 0.2;
  float rippleSpeedZScene1 = 0.3;
  float attractorSpeedScene1;
  float startingBodyAlpha = 0.00001;
  float bodyAlphaIncScene1 = 0.0;
  float pointSize = 5.0f; // Particle size
  float shellIncrementScene1;
  float particleIncrementScene1;

  // SCENE 1 sequencing

  float shellTurnsWhiteEvent = 15.0f;

  float particlesAppearEvent = 15.0f;

  float particlesSlowRippleEvent = 10.0f;

  float rippleSpeedUpEvent = 30.0f;

  float stopSpeedUpEvent = 62.0f;

  float startTurnShellBlack = 62.0f;

  float stopTurnShellBlack = 64.0f;

  float bodyCloudAppear = 65.0f;

  float bodyCloudMoveOut = 70.0f;

  // float startAttractor = 70.0f;

  float moveInEvent = 75.0;

  // scene 1 Meshes and Effects
  al::VAOMesh attractorMesh;
  al::VAOMesh bodyMesh;
  objParser newObjParser;
  Attractor mainAttractor;
  VertexEffectChain mainEffectChain;
  RippleEffect mainRippleY;
  RippleEffect mainRippleX;
  RippleEffect mainRippleZ;

  VertexEffectChain bodyEffectChain;
  ;
  ScatterEffect bodyScatter;

  // SCENE 1 DECLARE END //////

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());

    // parameterServer() << running << sceneTime;

    parameterServer() << sceneTimeParam;

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

  void createScene1() {
    newObjParser.parse(objPath, bodyMesh);
    bodyMesh.translate(0, 3.5, -4);
    for (int i = 0; i < bodyMesh.vertices().size(); ++i) {
      bodyMesh.color(
          1.0, 0.6, 0.3,
          startingBodyAlpha); // Orange particles with alpha transparency
      bodyMesh.texCoord(1.0f, 0.0f);
    }
    bodyMesh.update();
    bodyMesh.primitive(al::Mesh::POINTS);
    bodyMesh.update();

    // Initialize attractor
    al::addSphere(attractorMesh, 10.0f, 100, 100);
    attractorMesh.primitive(al::Mesh::LINES); // switch back to lines
    for (int i = 0; i < attractorMesh.vertices().size(); ++i) {
      attractorMesh.color(1.0, 0.6, 0.2, 0.9);
    }
    std::cout << "made attractor sphere with vertices # : "
              << attractorMesh.vertices().size() << std::endl;

    // SET EFFEFCTS
    bodyScatter.setBaseMesh(bodyMesh.vertices());
    bodyScatter.setParams(0.5, 20.0);
    bodyScatter.setScatterVector(bodyMesh);
    mainRippleX.setParams(4, 0.2, 4.0, 'y');
    mainRippleX.setParams(4, 0.2, 6.0, 'x');
    mainRippleX.setParams(4, 0.2, 5.0, 'z');

    mainEffectChain.pushBack(&mainRippleX);
    mainEffectChain.pushBack(&mainRippleY);
    mainEffectChain.pushBack(&mainRippleZ);

    bodyEffectChain.pushBack(&bodyScatter);

    bodyScatter.triggerOut(true, bodyMesh);

    bodyMesh.update();

    attractorMesh.update();
  }

  void onCreate() override {
    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));

    // cuttleboneDomain = al::CuttleboneDomain<Common>::enableCuttlebone(this);
    // if (!cuttleboneDomain) {
    //   std::cerr << "ERRor: Cuttlebone not started" << std::endl;
    // }

    nav().pos(al::Vec3d(0, 0, 0));
    // sequencer().playSequence();

    // if (sceneIndex == 1) {
    //  std::string path =
    //  localAssetPath("../softlight-sphere-new/audio/Song2.wav");

    if (isPrimary()) {
      sequencer1().add<SoundObject>(0, 44000).set(

          Song1Path.c_str());
      sequencer2().add<SoundObject>(0, 44000).set(

          Song2Path.c_str());
      sequencer3().add<SoundObject>(0, 44000).set(

          Song3Path.c_str());
      sequencer4().add<SoundObject>(0, 44000).set(

          Song4Path.c_str());
      sequencer5().add<SoundObject>(0, 44000).set(

          Song5Path.c_str());
      sequencer6().add<SoundObject>(0, 44000).set(

          Song6Path.c_str());
    }

    // SCENE 1 CREATE ////

    newObjParser.parse(objPath, bodyMesh);
    bodyMesh.translate(0, 3.5, -4);
    for (int i = 0; i < bodyMesh.vertices().size(); ++i) {
      bodyMesh.color(
          1.0, 0.6, 0.3,
          startingBodyAlpha); // Orange particles with alpha transparency
      bodyMesh.texCoord(1.0f, 0.0f);
    }
    bodyMesh.update();
    bodyMesh.primitive(al::Mesh::POINTS);
    bodyMesh.update();

    // Initialize attractor
    al::addSphere(attractorMesh, 10.0f, 100, 100);
    attractorMesh.primitive(al::Mesh::LINES); // switch back to lines
    for (int i = 0; i < attractorMesh.vertices().size(); ++i) {
      attractorMesh.color(1.0, 0.6, 0.2, 0.9);
    }
    std::cout << "made attractor sphere with vertices # : "
              << attractorMesh.vertices().size() << std::endl;

    // SET EFFEFCTS
    bodyScatter.setBaseMesh(bodyMesh.vertices());
    bodyScatter.setParams(0.5, 20.0);
    bodyScatter.setScatterVector(bodyMesh);
    mainRippleX.setParams(4, 0.2, 4.0, 'y');
    mainRippleX.setParams(4, 0.2, 6.0, 'x');
    mainRippleX.setParams(4, 0.2, 5.0, 'z');

    mainEffectChain.pushBack(&mainRippleX);
    mainEffectChain.pushBack(&mainRippleY);
    mainEffectChain.pushBack(&mainRippleZ);

    bodyEffectChain.pushBack(&bodyScatter);

    bodyScatter.triggerOut(true, bodyMesh);

    bodyMesh.update();

    attractorMesh.update();

    // SCENE 1 CREATE END
  }
  bool onKeyDown(const al::Keyboard &k) override {

    if (isPrimary()) {

      if (k.key() == ' ' && running == false) {
        running = true;
        std::cout << "started running" << std::endl;
      } else if (k.key() == ' ' && running == true) {
        running = false;
        std::cout << "stopped running" << std::endl;
      } else {
        //   sceneTime = 0;
        //   std::cout << "reset scene time to 0" << std::endl;
      }
      if (k.key() == '1') {
        sceneIndex = 1;
        globalTime = 0.0;
        sceneTime = 0.0;
        running = true;
        std::cout << "scene index: " << 1 << "global time: " << globalTime
                  << std::endl;

        sequencer1().playSequence();

        return true;
      }
      if (k.key() == '2') {
        sceneIndex = 2;
        globalTime = 119.0;
        sceneTime = 0.0;
        std::cout << "scene index: " << 2 << "global time: " << globalTime
                  << std::endl;

        sequencer2().playSequence();

        return true;
      }
      if (k.key() == '3') {
        sceneIndex = 3;
        globalTime = 335.0;
        sceneTime = 0.0;
        std::cout << "scene index: " << 3 << "global time: " << globalTime
                  << std::endl;

        sequencer3().playSequence();
        return true;
      }
      if (k.key() == '4') {
        sceneIndex = 4;
        globalTime = 444.0;
        sceneTime = 0.0;
        std::cout << "scene index: " << 4 << "global time: " << globalTime
                  << std::endl;

        sequencer4().playSequence();
        return true;
      }
      if (k.key() == '5') {
        sceneIndex = 5;
        globalTime = 936.0;
        sceneTime = 0.0;
        std::cout << "scene index: " << 5 << "global time: " << globalTime
                  << std::endl;

        sequencer5().playSequence();
        return true;
      }
      if (k.key() == '6') {
        sceneIndex = 6;
        globalTime = 1105.0;
        sceneTime = 0.0;
        std::cout << "scene index: " << 6 << "global time: " << globalTime
                  << std::endl;

        sequencer6().playSequence();
        return true;
      }
      return false;
    }
  }

  void animateScene1(double dt) {

    if (sceneTime < particlesSlowRippleEvent) {
      mainAttractor.processThomas(attractorMesh, sceneTime, 0);
    }

    if (sceneTime >= particlesSlowRippleEvent &&
        sceneTime <= rippleSpeedUpEvent) {
      attractorSpeedScene1 = 0.00005;
      mainAttractor.processThomas(attractorMesh, sceneTime,
                                  attractorSpeedScene1);
      attractorMesh.translate(

          0, 5 * 0.00005, -5 * 0.00005);
    }

    if (sceneTime >= rippleSpeedUpEvent && sceneTime <= stopSpeedUpEvent) {
      attractorSpeedScene1 = 0.00015;
      mainAttractor.processThomas(attractorMesh, sceneTime,
                                  attractorSpeedScene1);
      attractorMesh.translate(

          0, 5 * 0.0001, -5 * 0.0001);
    }
    if (sceneTime >= stopSpeedUpEvent && sceneTime <= moveInEvent) {
      attractorSpeedScene1 = 0.00001;
      mainAttractor.processThomas(attractorMesh, sceneTime,
                                  attractorSpeedScene1);
      attractorMesh.translate(

          0, 0, -15 * 0.0002);
    }

    if (sceneTime >= stopSpeedUpEvent) {
      attractorSpeedScene1 = 0.00005;
      mainAttractor.processThomas(attractorMesh, sceneTime,
                                  attractorSpeedScene1);
      // attractorMesh.translate(

      //     0, 0, -10 * 0.0002);
      // mainEffectChain.process(attractorMesh, sceneTime); //ripple
      // commented out
    }

    attractorMesh.update();
    // body mesh effects
    if (sceneTime >= bodyCloudAppear) {
      if (bodyAlphaIncScene1 < 1.0) {
        bodyAlphaIncScene1 += 0.0000000000000001;
      }
    }
    if (sceneTime >= moveInEvent) {
      attractorSpeedScene1 = 0.0000001;
      mainAttractor.processThomas(attractorMesh, sceneTime,
                                  attractorSpeedScene1);
      bodyScatter.setParams(5.0, 20.0);
      bodyScatter.triggerIn(true);

      attractorMesh.translate(

          -50 * 0.0001, 100.5 * 0.0001, -80.0 * 0.0001);
      attractorMesh.scale(0.995);
    }

    bodyEffectChain.process(bodyMesh, sceneTime);
    bodyMesh.update();

    // SCENE 1 ANIMATE END
  }

  // }
  void onAnimate(double dt) override {
    // boiler plate for every scene / main template
    // if (!isPrimary()) {
    //   // //   running = state().running;
    //   running = pRunning.get();
    // // } else {
    // //   state().running = running;
    // //   pRunning.set(running);
    // // }

    // std::cout << "running : " << state().running << std::endl;
    // std::cout << "index : " << state().sceneIndex << std::endl;
    // std::cout << "time : " << state().sceneTime << std::endl;
    if (isPrimary()) {
      runningParam.set(running);
    }

    if (running == true && runningParam == true) {

      globalTime += dt;
      sceneTime += dt;
      // sceneTimeParam.set(sceneTime); // sync via parameter

      // time : " << globalTime << std::endl;
      // localTime += dt;
      // sceneTime += dt;
      // sceneTime.set(localTime); // GUI & scripting see this

      // state().sceneTimeState = localTime;
      // sceneTime.set(localTime);
      if (isPrimary()) {
        if (globalTime >= 0.0 && globalTime < 0.0 + dt) {
          sceneIndex = 1;
          sceneTime = 0.0;
          sequencer1().playSequence();
          std::cout << "started scene 1" << std::endl;
        } else if (globalTime >= 119.0 && globalTime < 119.0 + dt) {
          sceneIndex = 2;
          sceneTime = 0.0;
          sequencer2().playSequence();
          std::cout << "started scene 2" << std::endl;
        } else if (globalTime >= 335.0 && globalTime < 335.0 + dt) {
          sceneIndex = 3;
          sceneTime = 0.0;
          sequencer3().playSequence();
          std::cout << "started scene 3" << std::endl;
        } else if (globalTime >= 444.0 && globalTime < 444.0 + dt) {
          sceneIndex = 4;
          sceneTime = 0.0;
          sequencer4().playSequence();
          std::cout << "started scene 4" << std::endl;
        } else if (globalTime >= 936.0 && globalTime < 936.0 + dt) {
          sceneIndex = 5;
          sceneTime = 0.0;
          sequencer5().playSequence();
          std::cout << "started scene 5" << std::endl;
        } else if (globalTime >= 1105.0 && globalTime < 1105.0 + dt) {
          sceneIndex = 6;
          sceneTime = 0.0;
          sequencer6().playSequence();
          std::cout << "started scene 6" << std::endl;
        }

        // state().sceneIndex = sceneIndex;
        // state().sceneTime = sceneTime;
        // pTime.set(sceneTime);
        // pIndex.set(sceneIndex);
        // } else {
        //   sceneTime.set(state().sceneTimeState); // sync from primary
      }
      // sceneTime = sceneTimeParam.get(); // receive sync
      //  end of boilerplate

      // SCENE 1 ANIMATE

      if (sceneTime < particlesSlowRippleEvent) {
        mainAttractor.processThomas(attractorMesh, sceneTime, 0);
      }

      if (sceneTime >= particlesSlowRippleEvent &&
          sceneTime <= rippleSpeedUpEvent) {
        attractorSpeedScene1 = 0.00005;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 5 * 0.00005, -5 * 0.00005);
      }

      if (sceneTime >= rippleSpeedUpEvent && sceneTime <= stopSpeedUpEvent) {
        attractorSpeedScene1 = 0.00015;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 5 * 0.0001, -5 * 0.0001);
      }
      if (sceneTime >= stopSpeedUpEvent && sceneTime <= moveInEvent) {
        attractorSpeedScene1 = 0.00001;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        attractorMesh.translate(

            0, 0, -15 * 0.0002);
      }

      if (sceneTime >= stopSpeedUpEvent) {
        attractorSpeedScene1 = 0.00005;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        // attractorMesh.translate(

        //     0, 0, -10 * 0.0002);
        // mainEffectChain.process(attractorMesh, sceneTime); //ripple
        // commented out
      }

      attractorMesh.update();
      // body mesh effects
      if (sceneTime >= bodyCloudAppear) {
        if (bodyAlphaIncScene1 < 1.0) {
          bodyAlphaIncScene1 += 0.0000000000000001;
        }
      }
      if (sceneTime >= moveInEvent) {
        attractorSpeedScene1 = 0.0000001;
        mainAttractor.processThomas(attractorMesh, sceneTime,
                                    attractorSpeedScene1);
        bodyScatter.setParams(5.0, 20.0);
        bodyScatter.triggerIn(true);

        attractorMesh.translate(

            -50 * 0.0001, 100.5 * 0.0001, -80.0 * 0.0001);
        attractorMesh.scale(0.995);
      }

      bodyEffectChain.process(bodyMesh, sceneTime);
      bodyMesh.update();

      // SCENE 1 ANIMATE END
    }
  }

  void drawScene1(al::Graphics &g) {
    glEnable(GL_PROGRAM_POINT_SIZE);

    if (sceneTime < shellTurnsWhiteEvent) {
      shellIncrementScene1 = ((sceneTime) / (shellTurnsWhiteEvent));
      g.clear(1.0 - shellIncrementScene1);
    }
    if (sceneTime >= shellTurnsWhiteEvent) {
      g.clear(0.0);
    }
    g.depthTesting(true);
    g.blending(true);
    g.blendAdd();

    g.pointSize(pointSize);
    g.meshColor();
    g.draw(attractorMesh);

    if (sceneTime >= bodyCloudAppear) {
      g.shader(pointShader);
      pointShader.uniform("pointSize", 0.01);
      pointShader.uniform("inputColor",
                          al::Vec4f(1.0, 0.6, 0.3, bodyAlphaIncScene1));
      g.draw(bodyMesh);
      g.shader();
    }
    // } else {
    //   g.clear(0.0);
  }

  void onDraw(al::Graphics &g) override {

    // SCENE 1 DRAW /////
    if (running == true) {
      glEnable(GL_PROGRAM_POINT_SIZE);

      if (sceneTime < shellTurnsWhiteEvent) {
        shellIncrementScene1 = ((sceneTime) / (shellTurnsWhiteEvent));
        g.clear(1.0 - shellIncrementScene1);
      }
      if (sceneTime >= shellTurnsWhiteEvent) {
        g.clear(0.0);
      }
      g.depthTesting(true);
      g.blending(true);
      g.blendAdd();

      g.pointSize(pointSize);
      g.meshColor();
      g.draw(attractorMesh);

      if (sceneTime >= bodyCloudAppear) {
        g.shader(pointShader);
        pointShader.uniform("pointSize", 0.01);
        pointShader.uniform("inputColor",
                            al::Vec4f(1.0, 0.6, 0.3, bodyAlphaIncScene1));
        g.draw(bodyMesh);
        g.shader();
      }
    } else {
      g.clear(0.0);
    }

    // SCENE 1 END /////
  }

  void onSound(al::AudioIOData &io) override {
    if (isPrimary()) {
      spatializer->prepare(io);

      if (sceneIndex == 1) {
        mSequencer1.render(io);
      } else if (sceneIndex == 2) {
        mSequencer2.render(io);
      } else if (sceneIndex == 3) {
        mSequencer3.render(io);
      } else if (sceneIndex == 4) {
        mSequencer4.render(io);
      } else if (sceneIndex == 5) {
        mSequencer5.render(io);
      } else if (sceneIndex == 6) {
        mSequencer6.render(io);
      }

      spatializer->renderBuffer(io, {0, 0, 0}, io.outBuffer(0),
                                io.framesPerBuffer());
      spatializer->finalize(io);
    }
  }

  al::SynthSequencer mSequencer1;
  al::SynthSequencer mSequencer2;
  al::SynthSequencer mSequencer3;
  al::SynthSequencer mSequencer4;
  al::SynthSequencer mSequencer5;
  al::SynthSequencer mSequencer6;

  al::SynthSequencer &sequencer1() { return mSequencer1; }
  al::SynthSequencer &sequencer2() { return mSequencer2; }
  al::SynthSequencer &sequencer3() { return mSequencer3; }
  al::SynthSequencer &sequencer4() { return mSequencer4; }
  al::SynthSequencer &sequencer5() { return mSequencer5; }
  al::SynthSequencer &sequencer6() { return mSequencer6; }
};

int main() {
  MyApp app;

  if (al::sphere::isSphereMachine())
    app.configureAudio(44100, 512, 60, 0);
  else
    app.configureAudio(44100, 512, 2, 0);
  app.start();
  return 0;
}

std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}
