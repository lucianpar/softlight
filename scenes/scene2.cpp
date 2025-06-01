
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
  bool onKeyDown(const al::Keyboard &k) override { return true; }
  //  float newSpeed = 0.0f;
  void onAnimate(double dt) override {

    // SET SCENES AND TIME TRANSITIONS ///
    globalTime += dt;
    sceneTime += dt;

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
    }

    blobsEffectChain.process(blobMesh, sceneTime);

    // starEffectChain.process(starCreatureMesh, sceneTime);

    blobMesh.update();

    // SCENE 2 ANIMATE END /////
  }

  // END OF ANIMATE CALLBACK

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 START OF DRAW /////
    if (sceneIndex == 1) {

      // SCENE 2 DRAW ////

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

      g.pointSize(pointSize);

      for (int i = 0; i < blobs.size(); ++i) {
        al::Vec3f newColor = colorPallete[i % 3];

        g.pushMatrix();
        g.translate(blobs[i].pos());
        g.rotate(blobs[i].quat());
        if (i % 2 == 1) {
          g.color(newColor.x, newColor.y, newColor.z,
                  0.4 + (sin(sceneTime * 2.0) * 0.1));
          g.draw(blobMesh);
        } else {
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

  app.start();
  return 0;
}
