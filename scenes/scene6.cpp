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
#include "al/ui/al_FileSelector.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
// #include "al/graphics/al_Asset.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <cmath>
#include <fstream>
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

int sceneIndex = 6;

std::string slurp(const std::string &fileName);

#define MAX_JELLIES 6

struct Common {
  float flicker;
  float jellyX[MAX_JELLIES];
  float jellyY[MAX_JELLIES];
  float jellyZ[MAX_JELLIES];
  float jellyQuatW[MAX_JELLIES];
  float jellyQuatX[MAX_JELLIES];
  float jellyQuatY[MAX_JELLIES];
  float jellyQuatZ[MAX_JELLIES];
};

class MyApp : public al::DistributedAppWithState<Common> {
public:
  al::Light light;
  al::Material material;
  al::FileSelector selector;
  al::SearchPaths searchPaths;
  std::string pointFragPath;
  std::string pointVertPath;
  std::string pointGeomPath;
  al::ShaderProgram pointShader;

  al::VAOMesh jellyCreatureMesh;
  std::vector<al::Nav> jellies;

  Creature creature;

  // === Scene 6 PARAMETERS ===
  al::Parameter scene6Boundary{"scene6Boundary", "", 50.0f, 0.0f, 100.0f};
  al::ParameterBool inSphereScene6{"inSphereScene6", "", true};
  al::Parameter jellieseperationThresh{"jellieseperationThresh", "", 4.0f, 0.0f,
                                       20.0f};
  al::Parameter jelliesSpeedScene6{"jelliesSpeedScene6", "", 3.0f, 0.0f, 10.0f};
  al::Parameter jelliesizeScene2{"jelliesizeScene2", "", 5.0f, 0.0f, 20.0f};
  al::Parameter pointSizeScene6{"pointSizeScene6", "", 2.5f, 0.1f, 10.0f};
  al::Parameter scene6pulseSpeed{"scene6pulseSpeed", "", 0.4f, 0.0f, 5.0f};
  al::Parameter scene6pulseAmount{"scene6pulseAmount", "", 0.2f, 0.0f, 5.0f};

  // Scene 6 Effects
  AutoPulseEffect jellyPulse;
  RippleEffect jellyRippleY;
  RippleEffect jellyRippleX;
  VertexEffectChain jellyEffectChain;

  double globalTime = 0;
  double sceneTime = 0;

  al::Vec3f randomVec3f(float scale) {
    return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                     al::rnd::uniformS()) *
           scale;
  }

  void onInit() override {
    gam::sampleRate(audioIO().framesPerSecond());
    searchPaths.addSearchPath(al::File::currentPath() + "/../../../..");

    al::FilePath frag = searchPaths.find("point-fragment.glsl");
    if (frag.valid())
      pointFragPath = frag.filepath();

    al::FilePath geom = searchPaths.find("point-geometry.glsl");
    if (geom.valid())
      pointGeomPath = geom.filepath();

    al::FilePath vert = searchPaths.find("point-vertex.glsl");
    if (vert.valid())
      pointVertPath = vert.filepath();

    parameterServer() << scene6Boundary << inSphereScene6
                      << jellieseperationThresh << jelliesSpeedScene6
                      << jelliesizeScene2 << pointSizeScene6 << scene6pulseSpeed
                      << scene6pulseAmount;
  }

  void onCreate() override {
    nav().pos(0, 0, 0);
    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));
    sequencer().playSequence();

    creature.makeJellyfish(jellyCreatureMesh, 0.6f, 72, 48, 8, 40, 40, 4.0f,
                           0.25f, 5.5f, 1.0f);
    for (int i = 0; i < jellyCreatureMesh.vertices().size(); ++i) {
      jellyCreatureMesh.color(1.0, 1, 1, 1);
      jellyCreatureMesh.texCoord(1.0f, 0.0f);
    }
    jellyCreatureMesh.scale(jelliesizeScene2);
    jellyCreatureMesh.primitive(al::Mesh::POINTS);
    jellyCreatureMesh.generateNormals();
    jellyPulse.setBaseMesh(jellyCreatureMesh.vertices());
    jellyPulse.setParams(scene6pulseSpeed, scene6pulseAmount, 1);
    jellyEffectChain.pushBack(&jellyPulse);
    jellyCreatureMesh.update();

    for (int b = 0; b < MAX_JELLIES; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      jellies.push_back(p);
      state().jellyX[b] = p.pos().x;
      state().jellyY[b] = p.pos().y;
      state().jellyZ[b] = p.pos().z;
      state().jellyQuatW[b] = p.quat().w;
      state().jellyQuatX[b] = p.quat().x;
      state().jellyQuatY[b] = p.quat().y;
      state().jellyQuatZ[b] = p.quat().z;
    }
  }
  double localTime;
  void onAnimate(double dt) override {
    if (isPrimary()) {
      globalTime += dt;
      localTime += dt;
    }
    sceneTime = localTime; // parameter - getting sent to everyone

    // sequencer().update(globalTime);
    std::cout << "global time: " << globalTime << std::endl;
    fflush(stdout);
    if (isPrimary()) {
      for (int i = 0; i < jellies.size(); ++i) {
        float t = globalTime + i * 10.0f;
        float wobbleAmount = 0.01f * std::sin(t * 0.7f);
        jellies[i].turnF(0.004f + wobbleAmount);
        if (jellies[i].pos().mag() > scene6Boundary.get())
          jellies[i].faceToward(al::Vec3f(0), 0.005f);
        jellies[i].moveF(jelliesSpeedScene6.get() * 8.0);
        jellies[i].step(dt);

        state().jellyX[i] = jellies[i].pos().x;
        state().jellyY[i] = jellies[i].pos().y;
        state().jellyZ[i] = jellies[i].pos().z;
        state().jellyQuatW[i] = jellies[i].quat().w;
        state().jellyQuatX[i] = jellies[i].quat().x;
        state().jellyQuatY[i] = jellies[i].quat().y;
        state().jellyQuatZ[i] = jellies[i].quat().z;

        state().flicker =
            0.25f +
            0.05f * std::sin(sceneTime * 2.0); // move back outside is primary

        // move back outside is primary?
        jellyPulse.setParams(scene6pulseSpeed, scene6pulseAmount, 1);
        jellyEffectChain.process(jellyCreatureMesh, sceneTime);
        jellyCreatureMesh.update();
      }
    }
    if (!isPrimary()) {
      for (int i = 0; i < jellies.size(); ++i) {
        jellies[i].pos().set(state().jellyX[i], state().jellyY[i],
                             state().jellyZ[i]);
        jellies[i].quat().set(state().jellyQuatW[i], state().jellyQuatX[i],
                              state().jellyQuatY[i], state().jellyQuatZ[i]);
      }
    }

    // state().flicker = 0.25f + 0.05f * std::sin(sceneTime * 2.0);
  }

  void onDraw(al::Graphics &g) override {
    if (sceneIndex == 6) {
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
      g.depthTesting(true);
      g.clear(0.1, 0.0, 0.3);
      g.lighting(true);
      light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
      light.ambient(al::RGB(1.0, 1.0, 1.0));
      light.diffuse(al::RGB(1, 1, 1.0));
      g.light(light);
      material.specular(1.0);
      material.shininess(200);
      g.material(material);
      g.pointSize(pointSizeScene6.get());

      for (int i = 0; i < jellies.size(); ++i) {
        g.pushMatrix();
        g.translate(state().jellyX[i], state().jellyY[i], state().jellyZ[i]);
        g.rotate(al::Quatf(state().jellyQuatW[i], state().jellyQuatX[i],
                           state().jellyQuatY[i], state().jellyQuatZ[i]));
        g.pointSize(2.0);
        g.color(1.0f, 0.4f, 0.7f, state().flicker);
        g.draw(jellyCreatureMesh);
        g.popMatrix();
      }
    }
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

std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}