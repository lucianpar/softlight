
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
#define nAgentsScene2 30

int sceneIndex = 2;
struct Common {

  // scene 2
  float blobPosX[nAgentsScene2];
  float blobPosY[nAgentsScene2];
  float blobPosZ[nAgentsScene2];
  float blobQuatW[nAgentsScene2];
  float blobQuatX[nAgentsScene2];
  float blobQuatY[nAgentsScene2];
  float blobQuatZ[nAgentsScene2];
};

class MyApp : public al::DistributedAppWithState<Common> {
public:
  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;
  al::Material material;
  // GLOBAL TIME PARAMS//
  double globalTime = 0;
  float localTime = 0;
  al::Parameter sceneTime{"SceneTime", 0.0, 0.0, 600.0};
  al::ParameterBool running{"running", "0", true};

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
  // int nAgentsScene2 = 30;
  float blobsSpeedScene2 = 0.1;
  float blobSizeScene2 = 1.8;
  float shellIncrementScene1;
  float particleIncrementScene1;
  float pointSize = 3.0;
  // Creature creature;
  std::vector<al::Vec3f> colorPallete = {
      {0.9f, 0.0f, 0.4}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};

  al::ParameterBool inSphereScene2{"inSphereScene2", "2", true};
  al::Parameter blobSeperationThresh{"blobSeperationThresh", "2", 2.0, 0.0,
                                     10.0};
  al::Parameter scene2Boundary{"scene2Boundary", "2", 1.0, 0.0, 80.0};
  al::Parameter currentSpeedScene2{"currentSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter targetSpeedScene2{"targetSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter interpRateScene2{"interpRateScene2", "2", 0.01, 0.0, 1.0};

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

    parameterServer() << sceneTime << running;
    parameterServer() << blobSeperationThresh << scene2Boundary;
    parameterServer() << currentSpeedScene2 << targetSpeedScene2
                      << interpRateScene2;
    parameterServer() << inSphereScene2;
    // if (isPrimary()) {
    addSphere(blobMesh, blobSizeScene2, 40, 40);
    blobMesh.primitive(al::Mesh::TRIANGLES);
    blobMesh.generateNormals();
    creature.addStarfish(starCreatureMesh);
    starCreatureMesh.update();

    for (int b = 0; b < nAgentsScene2; ++b) {
      al::Nav p;
      blobs.push_back(p);
    }

    // Only assign random positions and orientations on primary node
    if (isPrimary()) {
      for (int b = 0; b < nAgentsScene2; ++b) {
        blobs[b].pos() = randomVec3f(5.0f);
        blobs[b]
            .quat()
            .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
                 al::rnd::uniformS())
            .normalize();
      }
    }

    blobMesh.update();

    blobsRippleX.setParams(0.2, 0.1, 1.0, 'x');
    blobsRippleZ.setParams(0.4, 0.1, 1.0, 'z');
    blobsEffectChain.pushBack(&blobsRippleZ);
    blobsEffectChain.pushBack(&blobsRippleX);
    starRipple.setParams(1.0, 1.0, 1.0, 'z');
    starEffectChain.pushBack(&starRipple);
    // scene2Boundary = 1.0;
    // }

    // SCENE 2 CREAT END /////
  }

  void onAnimate(double dt) override {

    if (running == true) {
      // Update global time
      if (isPrimary()) {
        globalTime += dt;
        // localTime += dt;
        sceneTime = sceneTime + dt;

        sequencer().update(globalTime);

        std::cout << "global time: " << globalTime << std::endl;
        fflush(stdout);
      }

      // Set speed transitions via targetSpeedScene2

      if (sceneIndex == 2) {
        if (sceneTime < 1.0) {
          scene2Boundary = 30.0;
          targetSpeedScene2 = 5.0;
        } else if (sceneTime < 5.0) {

          targetSpeedScene2 = 0.1;
        } else if (sceneTime < 8.0) {
          targetSpeedScene2 = 0.2;
        } else if (sceneTime < 11.0) {
          targetSpeedScene2 = 0.05;
        } else if (sceneTime >= 20.0) {
          targetSpeedScene2 = 2.0;
          // scene2Boundary = 5.0;
        }

        // Smooth the speed
        currentSpeedScene2 =
            currentSpeedScene2 +
            (targetSpeedScene2 - currentSpeedScene2) * interpRateScene2;

        // Expand boundary once early
        if (sceneTime >= 0.2) {
          scene2Boundary = 4.0;
        }

        // Animate all blobs
        if (isPrimary()) {
          for (int i = 0; i < blobs.size(); ++i) {
            al::Vec3f pos = blobs[i].pos();

            if (pos.mag() >= scene2Boundary) {
              blobs[i].faceToward(al::Vec3f(0),
                                  0.005); // slow turn rate to avoid jitter
              blobs[i].moveF(blobSizeScene2);
            }
            // for setting state for renderers
            blobs[i].moveF(currentSpeedScene2 * 30.0f); // use smoothed speed
            blobs[i].step(dt);
            state().blobPosX[i] = blobs[i].pos().x;
            state().blobPosY[i] = blobs[i].pos().y;
            state().blobPosZ[i] = blobs[i].pos().z;
            state().blobQuatW[i] = blobs[i].quat().w;
            state().blobQuatX[i] = blobs[i].quat().x;
            state().blobQuatY[i] = blobs[i].quat().y;
            state().blobQuatZ[i] = blobs[i].quat().z;
          }
        }
        if (!isPrimary()) {
          // updating pos and turning state
          for (int i = 0; i < blobs.size(); ++i) {
            blobs[i].pos().set(state().blobPosX[i], state().blobPosY[i],
                               state().blobPosZ[i]);

            // SETTING QUAT

            blobs[i].quat().set(state().blobQuatW[i], state().blobQuatX[i],
                                state().blobQuatY[i], state().blobQuatZ[i]);
            // more accurate but less efficient

            // blobs[i]
            //     .quat()
            //     .set(blobs[i].pos().x, blobs[i].pos().y,
            //     blobs[i].pos().z, 1.0f) .normalize(); // more efficient but
            //     less interesting
            // turning weird? come here
          }
        }

        // Apply mesh effects + regenerate normals
        blobsEffectChain.process(blobMesh, sceneTime);
        blobMesh.generateNormals();
        blobMesh.update();

        starEffectChain.process(starCreatureMesh, sceneTime);
        starCreatureMesh.generateNormals();
        starCreatureMesh.update();
      }
    }
  }

  // END OF ANIMATE CALLBACK

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 START OF DRAW /////
    if (sceneIndex == 2) {

      // SCENE 2 DRAW ////

      g.blendTrans();
      g.depthTesting(true);
      g.clear(0.0, 0.0, 0.09 + ((sceneTime / (334 - 118)) * 0.8));

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
