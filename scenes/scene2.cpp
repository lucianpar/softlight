
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

  // double sceneTime;
  //  int sceneIndex;
  //  bool running;

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
  double localTime = 0;
  // double sceneTime = 0;
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

  // Creature creature;
  std::vector<al::Vec3f> colorPallete = {
      {0.9f, 0.0f, 0.4}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};

  al::ParameterBool inSphereScene2{"inSphereScene2", "2", true};
  al::Parameter blobSeperationThresh{"blobSeperationThresh", "2", 2.0, 0.0,
                                     10.0};
  // al::Parameter scene2Boundary{"scene2Boundary", "2", 1.0, 0.0, 80.0};
  al::Parameter currentSpeedScene2{"currentSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter targetSpeedScene2{"targetSpeedScene2", "2", 0.1, 0.0, 50.0};
  al::Parameter interpRateScene2{"interpRateScene2", "2", 0.01, 0.0, 1.0};
  // al::Parameter blobSizeScene2{"blobSizeScene2", "2", 1.8, 0.0, 10.0};
  float blobSizeScene2 = 1.5;
  float scene2Boundary = 1.0;
  ;

  // MESH EFFECTS//
  VertexEffectChain blobsEffectChain;
  RippleEffect blobsRippleX;
  RippleEffect blobsRippleY;
  RippleEffect blobsRippleZ;

  VertexEffectChain starEffectChain;
  RippleEffect starRipple;

  // WIND PIECE SEQUENCING EVENTS (SCENE 2)
  float windSpeedFastStart = 0.0f;
  float windSpeedSlow1 = 3.0f;
  float windSpeedSuperSlow = 3.5f;
  float windSpeedMedFast1 = 11.0f;
  float windSpeedSlow2 = 23.0f;
  float windScaleLarge1 = 31.0f;
  float windSpeedMed2 = 34.0f;
  float windSpeedFast2 = 39.0f;
  float windSpeedSlow3 = 44.0f;
  float windScaleSmall1 = 49.0f;
  float windSpeedMed3 = 49.0f;
  float windSpeedSlow4 = 54.0f;
  float windScaleHuge1 = 67.0f;
  float windSpeedMed4 = 77.0f;
  float windSpeedSlower5 = 93.0f;
  float windScaleDownStart = 96.0f;
  float windSpeedFast3 = 99.0f;
  float windSpeedSlow5 = 101.0f;
  float windSpeedFast4 = 104.0f;
  float windSpeedSlow6 = 107.0f;
  float windSpeedMed5 = 117.0f;
  float windSizeFastTiny = 123.0f;
  float windSpeedSlow7 = 126.0f;
  float windScaleHuge2 = 133.0f;
  float windSpeedMed6 = 142.0f;
  float windScaleReturn = 146.0f;
  float windSpeedSlow8 = 150.0f;
  float windSpeedFast5 = 164.0f;
  float windSpeedSuperSlow2 = 175.0f;
  float windScaleFadeStart = 181.0f;
  float windSpeedMedSlow = 183.0f;
  float windFinalSlowdown = 199.0f;

  //// SCENE 2 END DECLARATIONS ////

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void createScene2() {
    parameterServer() << blobSeperationThresh;
    // parameterServer() << scene2Boundary;
    parameterServer() << currentSpeedScene2;
    parameterServer() << targetSpeedScene2;
    parameterServer() << interpRateScene2;
    parameterServer() << inSphereScene2;
    // parameterServer() << blobSizeScene2;
    //  if (isPrimary()) {
    addSphere(blobMesh, 1.8, 40, 40);
    blobMesh.primitive(
        al::Mesh::LINE_STRIP_ADJACENCY); // test if i like lines of triangles
                                         // more in the sphere
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
    blobMesh.update();
  }

  void onCreate() override {
    ////BIOLERPLATE////
    nav().pos(al::Vec3d(0, 0, 0)); // Set the camera to view the scene
    sequencer().playSequence();

    nav().pos(0, 0, 0);

    // SCENE 2 CREATE /////

    // navControl().disable(); // camera or nav movement -- comment out . this
    // is
    //  to deal with z fighting issues in scene 2.
    parameterServer() << sceneTime;
    parameterServer() << running;
    parameterServer() << blobSeperationThresh;
    // parameterServer() << scene2Boundary;
    parameterServer() << currentSpeedScene2;
    parameterServer() << targetSpeedScene2;
    parameterServer() << interpRateScene2;
    parameterServer() << inSphereScene2;
    // parameterServer() << blobSizeScene2;
    //  if (isPrimary()) {
    addSphere(blobMesh, 1.8, 40, 40);
    blobMesh.primitive(
        al::Mesh::LINE_STRIP_ADJACENCY); // test if i like lines of triangles
                                         // more in the sphere
    blobMesh.generateNormals();
    creature.addStarfish(starCreatureMesh);
    starCreatureMesh.update();

    for (int b = 0; b < nAgentsScene2; ++b) {
      al::Nav p;
      blobs.push_back(p);
    }

    std::cout << "blobs.size(): " << blobs.size() << std::endl;

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
    blobMesh.update();
    // scene2Boundary = 1.0;
    // }

    // SCENE 2 CREAT END /////
  }

  void animateScene2(double dt) {
    if (isPrimary()) {

      if (sceneTime < windSpeedSlow1) {
        targetSpeedScene2 = 3.0f;
        scene2Boundary = 12.0f; // Start open and fast
      } else if (sceneTime < windSpeedSuperSlow) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedMedFast1) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 3.0f; // Tight = appear large
      } else if (sceneTime < windSpeedSlow2) {
        targetSpeedScene2 = 5.0f; // Emphasize size release
        scene2Boundary = 18.0f;   // Open quickly = appear smaller
      } else if (sceneTime < windScaleLarge1) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 4.0f;
      } else if (sceneTime < windSpeedMed2) {
        targetSpeedScene2 = 5.5f; // Burst movement with scale jump
        scene2Boundary = 20.0f;
      } else if (sceneTime < windSpeedFast2) {
        targetSpeedScene2 = 4.0f;
        scene2Boundary = 25.0f;
      } else if (sceneTime < windSpeedMed3) {
        targetSpeedScene2 = 1.2f;
        scene2Boundary = 5.0f; // Dramatic zoom-in
      } else if (sceneTime < windSpeedSlow4) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 8.0f;
      } else if (sceneTime < windSpeedMed4) {
        targetSpeedScene2 = 4.5f;
        scene2Boundary = 22.0f; // Blowout moment
      } else if (sceneTime < windSpeedSlower5) {
        targetSpeedScene2 = 0.7f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedFast3) {
        targetSpeedScene2 = 3.5f;
        scene2Boundary = 15.0f;
      } else if (sceneTime < windSpeedSlow5) {
        targetSpeedScene2 = 0.4f;
        scene2Boundary = 3.5f;
      } else if (sceneTime < windSpeedFast4) {
        targetSpeedScene2 = 3.0f;
        scene2Boundary = 18.0f;
      } else if (sceneTime < windSpeedMed5) {
        targetSpeedScene2 = 0.6f;
        scene2Boundary = 10.0f;
      } else if (sceneTime < windSpeedSlow7) {
        targetSpeedScene2 = 6.0f; // Blast outward
        scene2Boundary = 30.0f;
      } else if (sceneTime < windSpeedMed6) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 5.0f;
      } else if (sceneTime < windScaleReturn) {
        targetSpeedScene2 = 2.2f;
        scene2Boundary = 14.0f;
      } else if (sceneTime < windSpeedSlow8) {
        targetSpeedScene2 = 0.3f;
        scene2Boundary = 6.0f;
      } else if (sceneTime < windSpeedFast5) {
        targetSpeedScene2 = 4.0f;
        scene2Boundary = 22.0f;
      } else if (sceneTime < windSpeedSuperSlow2) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 5.0f;
      } else if (sceneTime < windScaleFadeStart) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 3.0f;
      } else if (sceneTime < windSpeedMedSlow) {
        targetSpeedScene2 = 0.2f;
        scene2Boundary = 2.0f; // Ultra-tight presence
      } else if (sceneTime < windFinalSlowdown) {
        targetSpeedScene2 = 0.5f;
        scene2Boundary = 4.0f;
      } else {
        targetSpeedScene2 = 0.001f;
        scene2Boundary = 60.0f; // Full release drift
        // blobsEffectChain.clear(); // Optional: end FX
      }
    }
    // Animate all blobs
    if (isPrimary()) {
      for (int i = 0; i < blobs.size(); ++i) {
        al::Vec3f pos = blobs[i].pos();

        if (pos.mag() >= scene2Boundary) {
          blobs[i].faceToward(al::Vec3f(0),
                              0.01); // slow turn rate to avoid jitter
          blobs[i].moveF(blobSizeScene2);
        }
        // for setting state for renderers
        blobs[i].moveF(currentSpeedScene2 * 30.0f *
                       20.0f); // use smoothed speed
        blobs[i].step(dt);
        state().blobPosX[i] = blobs[i].pos().x;
        state().blobPosY[i] = blobs[i].pos().y;
        state().blobPosZ[i] = blobs[i].pos().z;
        state().blobQuatW[i] = blobs[i].quat().w;
        state().blobQuatX[i] = blobs[i].quat().x;
        state().blobQuatY[i] = blobs[i].quat().y;
        state().blobQuatZ[i] = blobs[i].quat().z;
      }
      blobsEffectChain.process(blobMesh, sceneTime);
      blobMesh.generateNormals();
      blobMesh.update();

      starEffectChain.process(starCreatureMesh, sceneTime);
      starCreatureMesh.generateNormals();
      starCreatureMesh.update();

      // THIS PROCESSING MIGHT NEED TO UPDATE OUTSIDE PRIMARY AS WELL?
    }
    if (!isPrimary()) {
      // updating pos and turning state
      for (int i = 0; i < blobs.size(); ++i) {
        blobs[i].pos().set(state().blobPosX[i], state().blobPosY[i],
                           state().blobPosZ[i]);

        // SETTING QUAT

        blobs[i].quat().set(state().blobQuatW[i], state().blobQuatX[i],
                            state().blobQuatY[i], state().blobQuatZ[i]);

        // blobs[i]
        //     .quat()
        //     .set(blobs[i].pos().x, blobs[i].pos().y,
        //     blobs[i].pos().z, 1.0f) .normalize(); // more efficient but
        //     less interesting
        // turning weird? come here
        blobMesh.update();
        starCreatureMesh.update();
        // dstd::cout << sceneTime << std::endl;
      }
    }
  }

  void onAnimate(double dt) override {

    if (running == true) {
      // Update global time
      if (isPrimary()) {
        globalTime += dt;
        // sceneTime = sceneTime + dt;
        localTime += dt;
        // localTime += dt;
        // sceneTime = localTime; // seems to be an alright fix

        // sequencer().update(globalTime);

        std::cout << "global time: " << globalTime << std::endl;
        fflush(stdout);
      }
      sceneTime = localTime;
      //  }

      // Set speed transitions via targetSpeedScene2

      // if (sceneIndex == 2) {

      if (isPrimary()) {

        if (sceneTime < windSpeedSlow1) {
          targetSpeedScene2 = 3.0f;
          scene2Boundary = 12.0f; // Start open and fast
        } else if (sceneTime < windSpeedSuperSlow) {
          targetSpeedScene2 = 0.5f;
          scene2Boundary = 6.0f;
        } else if (sceneTime < windSpeedMedFast1) {
          targetSpeedScene2 = 0.2f;
          scene2Boundary = 3.0f; // Tight = appear large
        } else if (sceneTime < windSpeedSlow2) {
          targetSpeedScene2 = 5.0f; // Emphasize size release
          scene2Boundary = 18.0f;   // Open quickly = appear smaller
        } else if (sceneTime < windScaleLarge1) {
          targetSpeedScene2 = 0.7f;
          scene2Boundary = 4.0f;
        } else if (sceneTime < windSpeedMed2) {
          targetSpeedScene2 = 5.5f; // Burst movement with scale jump
          scene2Boundary = 20.0f;
        } else if (sceneTime < windSpeedFast2) {
          targetSpeedScene2 = 4.0f;
          scene2Boundary = 25.0f;
        } else if (sceneTime < windSpeedMed3) {
          targetSpeedScene2 = 1.2f;
          scene2Boundary = 5.0f; // Dramatic zoom-in
        } else if (sceneTime < windSpeedSlow4) {
          targetSpeedScene2 = 0.7f;
          scene2Boundary = 8.0f;
        } else if (sceneTime < windSpeedMed4) {
          targetSpeedScene2 = 4.5f;
          scene2Boundary = 22.0f; // Blowout moment
        } else if (sceneTime < windSpeedSlower5) {
          targetSpeedScene2 = 0.7f;
          scene2Boundary = 6.0f;
        } else if (sceneTime < windSpeedFast3) {
          targetSpeedScene2 = 3.5f;
          scene2Boundary = 15.0f;
        } else if (sceneTime < windSpeedSlow5) {
          targetSpeedScene2 = 0.4f;
          scene2Boundary = 3.5f;
        } else if (sceneTime < windSpeedFast4) {
          targetSpeedScene2 = 3.0f;
          scene2Boundary = 18.0f;
        } else if (sceneTime < windSpeedMed5) {
          targetSpeedScene2 = 0.6f;
          scene2Boundary = 10.0f;
        } else if (sceneTime < windSpeedSlow7) {
          targetSpeedScene2 = 6.0f; // Blast outward
          scene2Boundary = 30.0f;
        } else if (sceneTime < windSpeedMed6) {
          targetSpeedScene2 = 0.5f;
          scene2Boundary = 5.0f;
        } else if (sceneTime < windScaleReturn) {
          targetSpeedScene2 = 2.2f;
          scene2Boundary = 14.0f;
        } else if (sceneTime < windSpeedSlow8) {
          targetSpeedScene2 = 0.3f;
          scene2Boundary = 6.0f;
        } else if (sceneTime < windSpeedFast5) {
          targetSpeedScene2 = 4.0f;
          scene2Boundary = 22.0f;
        } else if (sceneTime < windSpeedSuperSlow2) {
          targetSpeedScene2 = 0.2f;
          scene2Boundary = 5.0f;
        } else if (sceneTime < windScaleFadeStart) {
          targetSpeedScene2 = 0.2f;
          scene2Boundary = 3.0f;
        } else if (sceneTime < windSpeedMedSlow) {
          targetSpeedScene2 = 0.2f;
          scene2Boundary = 2.0f; // Ultra-tight presence
        } else if (sceneTime < windFinalSlowdown) {
          targetSpeedScene2 = 0.5f;
          scene2Boundary = 4.0f;
        } else {
          targetSpeedScene2 = 0.001f;
          scene2Boundary = 60.0f; // Full release drift
          // blobsEffectChain.clear(); // Optional: end FX
        }
      }

      // Animate all blobs
      if (isPrimary()) {
        for (int i = 0; i < blobs.size(); ++i) {
          al::Vec3f pos = blobs[i].pos();

          if (pos.mag() >= scene2Boundary) {
            blobs[i].faceToward(al::Vec3f(0),
                                0.01); // slow turn rate to avoid jitter
            blobs[i].moveF(blobSizeScene2);
          }
          // for setting state for renderers
          blobs[i].moveF(currentSpeedScene2 * 30.0f *
                         20.0f); // use smoothed speed
          blobs[i].step(dt);
          state().blobPosX[i] = blobs[i].pos().x;
          state().blobPosY[i] = blobs[i].pos().y;
          state().blobPosZ[i] = blobs[i].pos().z;
          state().blobQuatW[i] = blobs[i].quat().w;
          state().blobQuatX[i] = blobs[i].quat().x;
          state().blobQuatY[i] = blobs[i].quat().y;
          state().blobQuatZ[i] = blobs[i].quat().z;
        }
        blobsEffectChain.process(blobMesh, sceneTime);
        blobMesh.generateNormals();
        blobMesh.update();

        starEffectChain.process(starCreatureMesh, sceneTime);
        starCreatureMesh.generateNormals();
        starCreatureMesh.update();

        // THIS PROCESSING MIGHT NEED TO UPDATE OUTSIDE PRIMARY AS WELL?
      }
      if (!isPrimary()) {
        // updating pos and turning state
        for (int i = 0; i < blobs.size(); ++i) {
          blobs[i].pos().set(state().blobPosX[i], state().blobPosY[i],
                             state().blobPosZ[i]);

          // SETTING QUAT

          blobs[i].quat().set(state().blobQuatW[i], state().blobQuatX[i],
                              state().blobQuatY[i], state().blobQuatZ[i]);

          // blobs[i]
          //     .quat()
          //     .set(blobs[i].pos().x, blobs[i].pos().y,
          //     blobs[i].pos().z, 1.0f) .normalize(); // more efficient but
          //     less interesting
          // turning weird? come here
          blobMesh.update();
          starCreatureMesh.update();
        }
      }
    }
  }

  // END OF ANIMATE CALLBACK

  void drawScene2(al::Graphics &g) {
    g.clear(0.0, 0.0, 0.09 + ((sceneTime / (334.0 - 118.0)) * 0.8));
    g.light(light);

    g.blendTrans();
    g.depthTesting(true);

    g.lighting(true); // flashing / flickering -- comment this out if needed
    // lighting from karl's example
    light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
    light.ambient(al::RGB(0.5, (1.0), 1.0));
    light.diffuse(al::RGB(1, 1, 0.5));

    material.specular(light.diffuse());
    material.shininess(50);
    g.material(material);

    for (int i = 0; i < blobs.size(); ++i) {
      al::Vec3f newColor = colorPallete[i % 3];

      g.pushMatrix();
      g.translate(blobs[i].pos());
      g.rotate(blobs[i].quat());
      g.scale(1.5);
      if (i % 2 == 1) {
        g.color(newColor.x, newColor.y, newColor.z,
                0.3 + (sin(sceneTime * 2.0) * 0.1));
        g.draw(blobMesh);
      } else {
        g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4,
                0.4 + (sin(sceneTime * 0.6) * 0.1));
        g.draw(starCreatureMesh);
      }
      g.popMatrix();
    }
  }

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 START OF DRAW /////
    // if (sceneIndex == 2) {

    // SCENE 2 DRAW ////

    g.clear(0.0, 0.0, 0.09 + ((sceneTime / (334.0 - 118.0)) * 0.8));
    g.light(light);

    g.blendTrans();
    g.depthTesting(true);

    g.lighting(true); // flashing / flickering -- comment this out if needed
    // lighting from karl's example
    light.globalAmbient(al::RGB(0.5, (1.0), 1.0));
    light.ambient(al::RGB(0.5, (1.0), 1.0));
    light.diffuse(al::RGB(1, 1, 0.5));

    material.specular(light.diffuse());
    material.shininess(50);
    g.material(material);

    for (int i = 0; i < blobs.size(); ++i) {
      al::Vec3f newColor = colorPallete[i % 3];

      g.pushMatrix();
      g.translate(blobs[i].pos());
      g.rotate(blobs[i].quat());
      g.scale(1.5);
      if (i % 2 == 1) {
        g.color(newColor.x, newColor.y, newColor.z,
                0.3 + (sin(sceneTime * 2.0) * 0.1));
        g.draw(blobMesh);
      } else {
        g.color(newColor.x + 0.4, newColor.y + 0.4, newColor.z + 0.4,
                0.4 + (sin(sceneTime * 0.6) * 0.1));
        g.draw(starCreatureMesh);
      }
      g.popMatrix();
    }
    // SCENE 2 DRAW END ////

  } //// SCENE 1 END OF DRAW /////

  // mSequencer.render(g);
  //}

  void onSound(al::AudioIOData &io) override { mSequencer.render(io); }

  al::SynthSequencer &sequencer() { return mSequencer; }
  al::SynthSequencer mSequencer;
};

int main() {
  MyApp app;

  app.start();
  return 0;
}
