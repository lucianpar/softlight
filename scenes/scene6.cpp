
#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
// #include "al/app/al_App.hpp"
#include "al/app/al_DistributedApp.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_StdRandom.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
// #include "al/graphics/al_Asset.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <cmath>
#include <iostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/autoPulse.hpp"
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
#include "../utility/creatures.hpp"
#include "../utility/imageColorToMesh.hpp"

/* TO DO:

* fix jittering - need karls input. going for fluid movement
* sequence based on rms - mostly just update movement
* update world color to flow into next scene

* fix audio players

*

*/

int sceneIndex = 1;

// using namespace al;

struct Common {};

class MyApp : public al::DistributedAppWithState<Common> {
public:
  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;
  al::Material material;

  //// SCENE 6 DECLARE START

  // scene 6 MESHES
  al::VAOMesh jellyCreatureMesh;
  std::vector<al::Nav> jellies;
  std::vector<al::Vec3f> velocity;
  std::vector<al::Vec3f> force;
  Creature creature;

  // SCENE 6 PARAMS
  float scene2Boundary = 20.0f;
  bool inSphereScene2 = true;
  float jellieseperationThresh = 2.0f;
  int nAgentsScene6 = 3;
  float jelliesSpeedScene2 = 4.0;
  float jelliesizeScene2 = 2.5;
  float pointSize = 2.5;
  std::vector<al::Vec3f> colorPallete = {
      {1.0f, 0.0f, 0.5}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};

  // SCENE 6  MESH EFFECTS//
  AutoPulseEffect jellyPulse;
  RippleEffect jellyRippleY;
  RippleEffect jellyRippleX;
  VertexEffectChain jellyEffectChain;

  // SCENE 6 DECLARE END

  // GLOBAL TIME PARAMS//
  double globalTime = 0;
  double sceneTime = 0;

  al::Vec3f randomVec3f(float scale) {
    return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                     al::rnd::uniformS()) *
           scale;
  }

  void onInit() override {
    // cuddlebone stuff
    gam::sampleRate(audioIO().framesPerSecond());
  }

  void onCreate() override {
    ////BIOLERPLATE////
    // nav().pos(al::Vec3d(jellies[0].pos())); // Set the camera to view the
    // scene
    sequencer().playSequence();

    // //INITIALIZE LIGHTING

    nav().pos(0, 0, 0);

    // creature.makeJellyfish(jellyCreatureMesh);
    creature.makeJellyfish(
        jellyCreatureMesh, 0.6f, 72, 48, 8, 40, 40, 4.0f, 0.25f, 5.5f,
        1.0f); // less resolution than default by turning tendrils down

    jellyCreatureMesh.scale(jelliesizeScene2);
    jellyCreatureMesh.primitive(al::Mesh::POINTS);
    jellyCreatureMesh.generateNormals();
    jellyPulse.setBaseMesh(jellyCreatureMesh.vertices());
    jellyRippleY.setParams(0.2, 0.005, 2.0, 'y');
    // jellyRippleX.setParams(0.1, 0.08, 2.0, 'x');
    jellyEffectChain.pushBack(&jellyRippleY);
    // jellyEffectChain.pushBack(&jellyRippleX);

    // jellyPulse.setParams(0.3, 0., 1);

    jellyCreatureMesh.update();

    // blobMesh.update();

    for (int b = 0; b < nAgentsScene6; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      // p.set(randomVec3f(5), randomVec3f(1));
      jellies.push_back(p);
      // velocity.push_back(al::Vec3f(0));
      // force.push_back(al::Vec3f(0));
    }

    // nav().pos(al::Vec3d(jellies[0].pos())); // Set the camera to view the
    // scene

    // jellyEffectChain.pushBack(&jellyPulse);
  }

  ////BASIC TRIGGERING////
  bool onKeyDown(const al::Keyboard &k) override {

    if (k.key() >= 49 && k.key() <= 54) {
      sceneIndex = k.key() - 48;
      std::cout << "pressed key: " << sceneIndex << std::endl;
    }

    return true;
  }
  //  float newSpeed = 0.0f;
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

    ///// SCENE 6  ANIMATE ->>>>>

    // SCENE 6 CAMERA
    // nav().pos(al::Vec3d(jellies[0].pos()));
    // nav().spin(0.2);
    // camernav().turnF(0.6);

    // SCENE 6 MAIN LOGIC

    for (int i = 0; i < jellies.size(); ++i) {
      // llm helped with movement
      float t = globalTime + i * 10.0f; // moving at slightly diff rates

      // drifting sort of
      float wobbleAmount = 0.02f * std::sin(t * 0.7f);
      jellies[i].turnF(0.007f + wobbleAmount); //

      //
      if (jellies[i].pos().mag() > scene2Boundary) {
        jellies[i].faceToward(al::Vec3f(0), 0.02f);
      }

      // bobbing
      float bob = 0.003f * std::sin(t * 0.5f);
      jellies[i].pos().y += bob;

      // standard move forward
      jellies[i].moveF(jelliesSpeedScene2);
      jellies[i].step(dt);
    }

    // SCENE 6 UPDATE AND PROCESS
    jellyEffectChain.process(jellyCreatureMesh, sceneTime);

    jellyCreatureMesh.update();
  }
  // END OF ANIMATE CALLBACK

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 jellyT OF DRAW /////
    if (sceneIndex == 1) {

      // SCENE 6 WORLD LIGHTING
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      // g.blendTrans();
      g.depthTesting(true);
      g.clear(0.1, 0.0, 0.3);

      g.lighting(true);
      // lighting from karl's example
      light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
      light.ambient(al::RGB(1.0, 1.0, 1.0));
      light.diffuse(al::RGB(1, 1, 1.0));
      g.light(light);
      material.specular(1.0);
      material.shininess(128);
      g.material(material);
      // end of lighting for karl's example

      glDepthMask(GL_FALSE); // re enable later if needed

      // SCENE 6 SEQUENCING

      g.pointSize(pointSize);

      for (int i = 0; i < jellies.size(); ++i) {
        al::Vec3f newColor = colorPallete[0];

        g.pushMatrix();
        g.translate(jellies[i].pos());
        g.rotate(jellies[i].quat());
        // g.color(newColor.x, newColor.y, newColor.z, 0.2);
        float flicker = 0.1f + 0.05f * std::sin(globalTime * 2.0);
        g.color(1.0f, 0.4f, 0.7f, flicker);
        g.draw(jellyCreatureMesh);
        g.popMatrix();
      }
    }

    ///// END SCENE 6  ANIMATE ->>>>>

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
  // fine here// -- commented out cause audio isnt working

  // bool soundObjectVisual = false;
  // AudioLoader audioLoader;
  // std::vector<std::vector<std::string>> songFiles(
  //     5); // vector for each song scene . //NOTE -- VEC IS 0 INDEX, BUT SONG
  //         // FOLDERS ARE 1 INDEX FOR CLARITY
  // audioLoader.loadSceneAudio(songFiles, 1);
  // audioLoader.loadSceneAudio(songFiles, 2);
  // // audioLoader.loadSceneAudio(songFiles, 2);
  // // audioLoader.loadSceneAudio(songFiles, 3);
  // // audioLoader.loadSceneAudio(songFiles, 4);
  // // audioLoader.loadSceneAudio(songFiles, 5);
  // // audioLoader.loadSceneAudio(songFiles, 6);

  // // ^^^

  // // constants for testing
  // double g = 0.7;
  // float a = 1.4;
  // float b = 1.6;
  // float c = 1.0;
  // float d = 0.7;

  // // group audio by every scene. make multiple sequencers to trigger when
  // every
  // // scene index is switched

  // // assign trajectories in the sequencer!!

  // // UNCOMMENT AUDIO !!!!!!
  // // this logic doesn't work, will  have to change
  // if (sceneIndex == 1) {
  //   app.sequencer().add<SoundObject>(0, 44000).set(
  //       0, 0, 0, 0.5, soundObjectVisual, (songFiles[0][0]).c_str(),
  //       [&](double t, const al::Vec3f &p) -> al::Vec3f {
  //         return al::Vec3f(
  //             // body of lambda logic. will replace this will header calls
  //             (sin(a * p.y) + c * cos(a * p.x)),
  //             (sin(b * p.x) + d * cos(b * p.y)), p.z);
  //       });
  // }
  // if (sceneIndex == 2) {
  //   app.sequencer().add<SoundObject>(0, 44000).set(
  //       0, 0, 0, 0.5, soundObjectVisual, (songFiles[1][0]).c_str(),
  //       [&](double t, const al::Vec3f &p) -> al::Vec3f {
  //         return al::Vec3f(
  //             // body of lambda logic. will replace this will header calls
  //             (cos(a * p.y) + c * cos(a * p.x)),
  //             (sin(b * p.x) + d * sin(b * p.y)), p.z);
  //       });
  // }

  app.start();
  return 0;
}
