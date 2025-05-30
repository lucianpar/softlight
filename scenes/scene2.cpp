
#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
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
struct Common {};

class MyApp : public al::DistributedAppWithState<Common> {
public:
  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;
  // Light light;
  al::Material material;
  // GLOBAL TIME PARAMS//
  double globalTime = 0;
  double sceneTime = 0;

  al::Vec3f randomVec3f(float scale) {
    return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                     al::rnd::uniformS()) *
           scale;
  }

  //// START DECLARATIONS FOR SCENE 1 ////

  // SCENE 2 DECLARE /////
  //  MESHES//
  al::VAOMesh blobMesh;
  al::VAOMesh starCreatureMesh;
  std::vector<al::Nav> blobs;
  std::vector<al::Vec3f> velocity;
  std::vector<al::Vec3f> force;
  Creature creature;
  // PARAMS
  float scene2Boundary = 30.0f;
  bool inSphereScene2 = true;
  float blobSeperationThresh = 2.0f;
  int nAgentsScene2 = 30;
  float blobsSpeedScene2 = 0.1;
  float blobSizeScene2 = 1.8;
  float shellIncrementScene1;
  float particleIncrementScene1;
  float pointSize = 3.0;
  // Creature creature;
  std::vector<al::Vec3f> colorPallete = {
      {0.9f, 0.0f, 0.4}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};

  // MESH EFFECTS//
  VertexEffectChain blobsEffectChain;
  RippleEffect blobsRippleX;
  RippleEffect blobsRippleY;
  RippleEffect blobsRippleZ;

  VertexEffectChain starEffectChain;
  RippleEffect starRipple;

  //// SCENE 2 END DECLARATIONS ////

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    ////BIOLERPLATE////
    nav().pos(al::Vec3d(0, 0, 0)); // Set the camera to view the scene
    sequencer().playSequence();

    nav().pos(0, 0, 0);

    // SCENE 2 CREATE /////
    addSphere(blobMesh, blobSizeScene2, 40, 40);
    blobMesh.primitive(al::Mesh::TRIANGLES);
    for (int i = 0; i < blobMesh.vertices().size(); i++) {
    }
    blobMesh.generateNormals();
    creature.addStarfish(starCreatureMesh);
    starCreatureMesh.update();

    for (int b = 0; b < nAgentsScene2; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      blobs.push_back(p);
    }

    blobMesh.update();

    blobsRippleX.setParams(0.2, 0.1, 1.0, 'x');
    blobsRippleZ.setParams(0.4, 0.1, 1.0, 'z');
    blobsEffectChain.pushBack(&blobsRippleZ);
    blobsEffectChain.pushBack(&blobsRippleX);
    starRipple.setParams(1.0, 1.0, 1.0, 'z');
    starEffectChain.pushBack(&starRipple);
    scene2Boundary = 1.0;

    // SCENE 2 CREAT END /////
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

    // SCENE 2 ANIMATE /////
    // ISSUES WITH MOVEMENT THAT I NEED TO FIX //
    if (sceneTime >= 0) {
      // boundary is initially set super small so they are constrained
      scene2Boundary = 15.0;
    }
    if (sceneTime >= 0 && sceneTime <= 1.0) {
      blobsSpeedScene2 = 3.0;
    }
    if (sceneTime >= 1.0 && sceneTime <= 5.0) {
      blobsSpeedScene2 = 0.1;
    }

    if (sceneTime >= 5.0) {
      blobsSpeedScene2 = 0.2;
    }
    if (sceneTime >= 8.0) {
      blobsSpeedScene2 = 0.05;
    }
    if (sceneTime >= 11.0) {
      blobsSpeedScene2 = 0.3;
    }

    for (int i = 0; i < blobs.size(); ++i) {
      // inSphereScene2 = true;
      if (blobs[i].pos().mag() >= scene2Boundary) {
        blobs[i].faceToward(al::Vec3f(0), 0.02);
        blobs[i].moveF(blobSizeScene2);
      }

      blobs[i].moveF(blobsSpeedScene2 * 30.0);

      blobs[i].step(dt);

      // SHOULD PROBABLY UPDATE LIKE SO
      /*
      AND ADJUST TIMES ACCORDINGLY - SCALE WAY UP
      blobs[i].moveF(blobsSpeedScene2);

      blobs[i].step(dt);
      */
    }

    blobsEffectChain.process(blobMesh, sceneTime);

    //starEffectChain.process(starCreatureMesh, sceneTime);

    blobMesh.update();

    // SCENE 2 ANIMATE END /////
  }

  // END OF ANIMATE CALLBACK

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 START OF DRAW /////
    if (sceneIndex == 1) {

      // SCENE 2 DRAW ////
      glEnable(GL_BLEND);
      g.blendTrans();
      g.depthTesting(true);
      g.clear(0.0, 0.0, 0.09 + ((sceneTime / (118 - 334)) * 0.8));

      g.lighting(true);
      // lighting from karl's example
      light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
      light.ambient(al::RGB(0.5, (1.0), 1.0));
      light.diffuse(al::RGB(1, 1, 0.5));
      g.light(light);
      material.specular(light.diffuse() * 0.3);
      material.shininess(50);
      g.material(material);

      glDepthMask(GL_FALSE); // re enable later if needed

      g.pointSize(pointSize);

      for (int i = 0; i < blobs.size(); ++i) {
        al::Vec3f newColor = colorPallete[i % 3];

        g.pushMatrix();
        // g.color(1.0,sin(0.0+fearColorReact),0.5);
        g.translate(blobs[i].pos());
        g.rotate(blobs[i].quat());

        if (i % 2 == 1) {
          g.color(newColor.x, newColor.y, newColor.z,
                  0.4 + (sin(sceneTime * 2.0) * 0.1));
          g.draw(blobMesh);
        }

        else {
          g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4,
                  0.4 + (sin(sceneTime * 0.5) * 0.1));
          g.draw(starCreatureMesh);
        }
        g.popMatrix();
      }
      // SCENE 2 DRAW END ////

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
