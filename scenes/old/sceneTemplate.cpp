
#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
// #include "al/graphics/al_Asset.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <string>
#include "al/sound/al_SoundFile.hpp"
#include "al/ui/al_FileSelector.hpp"

// MY CUSTOM INCLUDES:
#include "softlight-sphere/eoys-mesh-fx/ripple.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxUtility.hpp"
// #include "softlight-sphere/meshMorph.hpp"
#include "softlight-sphere/eoys-mesh-fx/scatter.hpp"
#include "softlight-sphere/eoys-mesh-fx/vfxMain.hpp"
#include "softlight-sphere/utility/loadAudioScene.hpp"
#include "softlight-sphere/utility/parseObj.hpp"
#include "softlight-sphere/utility/soundObject.hpp"
// #include "softlight-sphere/imageToMesh.hpp"
// #include "softlight-sphere/imageToSphere.hpp"
#include "softlight-sphere/utility/attractors.hpp"
#include "softlight-sphere/utility/creatures.hpp"
#include "softlight-sphere/utility/imageColorToMesh.hpp"

/* TO DO:

*figure out setting up bodyMesh as a uv for rotation
* set up newer sequence -
* make main shell transition to body - setting up heaeder
*ask karl about lighting
*scene 1 and 2 graphics

- START ACTUALLY SEQUENCING BASED ON TRACKS


- change how the sequencer works
- SET UP TRACK ANALYZER / STRING SEQUENCE GENERATOR
*have option for sphere configuration

* set up scene containers for everything?

*/

int sceneIndex = 1;

// using namespace al;

// al::Vec3f randomVec3f(float scale) {
//   return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
//                    al::rnd::uniformS()) *
//          scale;
// }

class MyApp : public al::App {
public:
  al::Vec3f randomVec3f(float scale) {
    return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                     al::rnd::uniformS()) *
           scale;
  }
  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;

  //// START DECLARATIONS FOR SCENE 1 ////

  // MESHES//
  al::VAOMesh bodyMesh;
  al::VAOMesh bodyCloud;
  objParser newObjParser;
  al::Mesh boundarySphere;
  NewColorBuffer scene1ColorBuffer;

  // MESH EFFECTS//
  // for body
  VertexEffectChain bodyEffectChain;
  RippleEffect bodyRippleY;
  RippleEffect bodyRippleX;
  RippleEffect bodyRippleZ;
  ScatterEffect bodyScatter;
  Attractor bodyAttractor;

  //// END DECLARATIONS FOR SCENE 1 ////

  // GLOBAL TIME PARAMS//
  double globalTime = 0;
  double sceneTime = 0;
  // int sceneIndex; -- moved out of app to be global

  ////DECLARE VALUES FOR EVENT TIMES////

  // SCENE 1
  float rippleAmplitudeScene1 = 0.0;
  float rippleSpeedXScene1 = 0.2;
  float rippleSpeedYScene1 = 0.2;
  float rippleSpeedZScene1 = 0.3;

  float shellTurnsWhiteEvent = 7.0f;

  float particlesAppearEvent = 7.0f;

  float particlesSlowRippleEvent = 10.0f;

  float rippleSpeedUpEvent = 30.0f;

  float stopSpeedUpEvent = 62.0f;

  float startTurnShellBlack = 62.0f;

  float stopTurnShellBlack = 64.0f;

  float startAttractor = 70.0f;

  float moveInEvent = 85.0;

  // SCENE 2

  // float track2Start = 119;

  // i.e. -- moveOutEvent1, StartRipplingEvent1

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    ////BIOLERPLATE////
    nav().pos(al::Vec3d(0, 0, 0)); // Set the camera to view the scene
    sequencer().playSequence();

    // //INITIALIZE LIGHTING

    // end lighting test

    // BOUNDARY MESH
    addSphere(boundarySphere, 7.5);
    boundarySphere.primitive((al::Mesh::LINES));

    ///////

    // SCENE 1 STUFF INITS

    ////CREATE MY MESHES////

    // NEED TO RESET TO RELATIVE PATHS, NOT HARDCODED
    bodyMesh.primitive(al::Mesh::POINTS);

    newObjParser.parse("/Users/lucian/Desktop/201B/allolib_playground/"
                       "softlight-sphere/assets/BaseMesh.obj",
                       bodyMesh);
    bodyMesh.translate(0, 3.5, -4);

    scene1ColorBuffer.imageToNewMesh(
        "/Users/lucian/Desktop/201B/allolib_playground/softlight-sphere/assets/"
        "9400-white.png",
        bodyMesh);

    bodyMesh.update();

    // bodyCloud

    ////SET MESH EFFECTS////
    // for body
    bodyScatter.setBaseMesh(bodyMesh.vertices());
    bodyScatter.setParams(20.0, 40.0);
    bodyScatter.setScatterVector(bodyMesh);
    bodyRippleY.setParams(rippleSpeedYScene1, rippleAmplitudeScene1, 4.0, 'y');
    bodyRippleX.setParams(rippleSpeedXScene1, rippleAmplitudeScene1, 6.0, 'x');
    bodyRippleX.setParams(rippleSpeedXScene1, rippleAmplitudeScene1, 5.0, 'z');

    bodyEffectChain.pushBack(&bodyRippleY);
    bodyEffectChain.pushBack(&bodyRippleX);
    bodyEffectChain.pushBack(&bodyRippleZ);
    bodyEffectChain.pushBack(&bodyScatter);

    bodyScatter.triggerOut(true, bodyMesh);

    // SCENE 2 STUFF INITS
  }

  ////BASIC TRIGGERING////
  bool onKeyDown(const al::Keyboard &k) override {

    // if (k.key() == '1') {
    //  If the space key is pressed, we will trigger the sequencer
    // sequencer().playSequence();
    if (k.key() >= 49 && k.key() <= 54) {
      sceneIndex = k.key() - 48;
      std::cout << "pressed key: " << sceneIndex << std::endl;
    }

    return true;
  }
  float newSpeed = 0.0f;
  ;
  void onAnimate(double dt) override {

    // SET SCENES AND TIME TRANSITIONS ///
    globalTime += dt;
    sceneTime += dt;
    if (globalTime == 118) {
      sceneIndex = 2;
      sceneTime = 0;
    }
    if (globalTime == 334) {
      sceneIndex = 3;
      sceneTime = 0;
    }

    // Update the sequencer
    sequencer().update(globalTime); // XXX important to call this
    std::cout << "global time: " << globalTime << std::endl;
    fflush(stdout);
    // should we call in the audio callback instead?

    //// PROCESS MESH EFFECTS ////
    // bodyEffectChain.process(bodyMesh, globalTime);
    // openingSphereEffectChain.process(openingSphereMesh, globalTime);
    // openingSphereScatter.stop(true);
    // bodyScatter.triggerOut(true, bodyMesh);

    // bodyAttractor.processThomas(bodyMesh, globalTime, 0.001);

    // MESH EFFECT SEQUENCING//

    // SCENE 1 -- from

    // if (globalTime == 1){
    //   bodyScatter.triggerOut(true, bodyMesh);
    // }

    // bodyAttractor.processThomas(bodyMesh, globalTime, 0.0001);
    // bodyMesh.scale(1.01);

    float newAmplitude;
    // newSpeed;
    if (sceneTime < particlesSlowRippleEvent) {
      bodyAttractor.processThomas(bodyMesh, sceneTime, 0);
    }
    if (sceneTime >= particlesSlowRippleEvent &&
        sceneTime <= rippleSpeedUpEvent) {

      newSpeed = 0.00001;
      bodyAttractor.processThomas(bodyMesh, sceneTime, newSpeed);
      //   rippleAmplitudeScene1 =
      //   ((sceneTime-particlesSlowRippleEvent)/(rippleSpeedUpEvent-particlesSlowRippleEvent))*3;
      //   bodyRippleY.setParams(rippleSpeedYScene1, rippleAmplitudeScene1, 4.0,
      //   'y'); bodyRippleX.setParams(rippleSpeedXScene1,
      //   rippleAmplitudeScene1, 6.0, 'x');
      //   bodyRippleZ.setParams(rippleSpeedZScene1, rippleAmplitudeScene1, 5.0,
      //   'z');
    }
    if (sceneTime >= rippleSpeedUpEvent && sceneTime <= stopSpeedUpEvent) {
      newSpeed += 0.000001;
      bodyAttractor.processThomas(bodyMesh, sceneTime, newSpeed);
      //   rippleSpeedXScene1 = 0.2 + (((sceneTime-rippleSpeedUpEvent) /
      //   (stopSpeedUpEvent - rippleSpeedUpEvent))); rippleSpeedYScene1 = 0.2 +
      //   (((sceneTime-rippleSpeedUpEvent) / (stopSpeedUpEvent -
      //   rippleSpeedUpEvent))); rippleSpeedZScene1 = 0.3 +
      //   (((sceneTime-rippleSpeedUpEvent) / (stopSpeedUpEvent -
      //   rippleSpeedUpEvent))); //.0015;
      //   bodyRippleY.setParams(rippleSpeedYScene1, rippleAmplitudeScene1, 4.0,
      //   'y'); bodyRippleX.setParams(rippleSpeedXScene1,
      //   rippleAmplitudeScene1, 6.0, 'x');
      //   bodyRippleZ.setParams(rippleSpeedZScene1, rippleAmplitudeScene1, 5.0,
      //   'z');
    }
    if (sceneTime >= stopSpeedUpEvent && sceneTime <= moveInEvent) {
      if (newSpeed >= 0) {
        newSpeed -= 0.0001;
      }
      bodyAttractor.processThomas(bodyMesh, sceneTime, newSpeed);
    }

    if (sceneTime >= moveInEvent) {

      if (rippleAmplitudeScene1 > 0.0f) {
        rippleAmplitudeScene1 =
            1.0 - ((sceneTime - moveInEvent) / moveInEvent + 10.0);
      }
      bodyScatter.setParams(3.0, 10.0);
      bodyRippleY.setParams(rippleSpeedYScene1, rippleAmplitudeScene1, 4.0,
                            'y');
      bodyRippleX.setParams(rippleSpeedXScene1, rippleAmplitudeScene1, 6.0,
                            'x');
      bodyRippleZ.setParams(rippleSpeedZScene1, rippleAmplitudeScene1, 5.0,
                            'z');
      bodyScatter.triggerIn(true);
    }

    bodyEffectChain.process(bodyMesh, sceneTime);

    // SCENE 2 -- from
  }
  // END OF ANIMATE CALLBACK

  //// INCREMENT VALUES FOR DRAW ////
  float shellIncrementScene1;
  float particleIncrementScene1;
  float pointSize = 3.0;
  void onDraw(al::Graphics &g) override {

    //// SCENE 1 START OF DRAW /////
    if (sceneIndex == 1) {

      // THIS SEQUENCE MAKES THE SHELL APPEAR
      if (sceneTime < shellTurnsWhiteEvent) {
        shellIncrementScene1 = ((sceneTime) / (shellTurnsWhiteEvent));
        g.clear(1.0 - shellIncrementScene1);
      }
      if (sceneTime >= shellTurnsWhiteEvent) {
        g.clear(0.0);
      }
      // if (sceneTime >= startTurnShellBlack && sceneTime<=
      // (startTurnShellBlack + 3)){
      //   shellIncrementScene1 -=  ((sceneTime-startTurnShellBlack)/
      //   (startTurnShellBlack + 3)); g.clear(1.0-shellIncrementScene1);
      // }
      // if (sceneTime >= (startTurnShellBlack+3)){
      //   //shellIncrementScene1 =
      //   g.clear(0.0);
      // }

      // PARTICLES SEQUENCE 1

      g.pointSize(pointSize);
      // g.color(1.0);
      // g.meshColor();

      // if (sceneTime >= particlesAppearEvent) {
      g.meshColor();
      g.draw(bodyMesh);
      // }

      // MAIN COLOR SEQUENCE //
      // color sequence
      // if(sceneTime<=particlesAppearEvent){
      // g.color(0+(sceneTime/particlesAppearEvent));}
      // else{
      //   //g.color(1.0);
      // }

      // only draw once particles have dispersed

      // g.draw(boundarySphere);

    } //// SCENE 1 END OF DRAW /////

    mSequencer.render(g);
  }

  void onSound(al::AudioIOData &io) override { mSequencer.render(io); }

  al::SynthSequencer &sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;
  // int sceneIndex;

  // can this section move to onCreate or initial app declarations?? or is it
  // fine here//

  bool soundObjectVisual = false;
  AudioLoader audioLoader;
  std::vector<std::vector<std::string>> songFiles(
      5); // vector for each song scene . //NOTE -- VEC IS 0 INDEX, BUT SONG
          // FOLDERS ARE 1 INDEX FOR CLARITY
  audioLoader.loadSceneAudio(songFiles, 1);
  audioLoader.loadSceneAudio(songFiles, 2);
  // audioLoader.loadSceneAudio(songFiles, 2);
  // audioLoader.loadSceneAudio(songFiles, 3);
  // audioLoader.loadSceneAudio(songFiles, 4);
  // audioLoader.loadSceneAudio(songFiles, 5);
  // audioLoader.loadSceneAudio(songFiles, 6);

  // ^^^

  // constants for testing
  double g = 0.7;
  float a = 1.4;
  float b = 1.6;
  float c = 1.0;
  float d = 0.7;

  // group audio by every scene. make multiple sequencers to trigger when every
  // scene index is switched

  // assign trajectories in the sequencer!!

  // UNCOMMENT AUDIO !!!!!!
  // this logic doesn't work, will  have to change
  // if (sceneIndex ==1){
  // app.sequencer().add<SoundObject>(0, 44000).set( 0, 0, 0,
  // 0.5,soundObjectVisual ,(songFiles[0][0]).c_str(), [&](double t, const
  // al::Vec3f& p) -> al::Vec3f {
  //   return al::Vec3f(
  //   //body of lambda logic. will replace this will header calls
  //          (sin(a * p.y) + c * cos(a * p.x)),
  //         (sin(b * p.x) + d * cos(b * p.y)),
  //           p.z
  //       );
  // });
  // }
  // if (sceneIndex == 2) {
  // app.sequencer().add<SoundObject>(0, 44000).set( 0, 0, 0,
  // 0.5,soundObjectVisual ,(songFiles[1][0]).c_str(), [&](double t, const
  // al::Vec3f& p) -> al::Vec3f {
  //   return al::Vec3f(
  //   //body of lambda logic. will replace this will header calls
  //          (cos(a * p.y) + c * cos(a * p.x)),
  //         (sin(b * p.x) + d * sin(b * p.y)),
  //           p.z
  //       );
  // });
  // }

  app.start();
  return 0;
}
