
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

*

*/

int sceneIndex = 6;

std::string slurp(const std::string &fileName);

// using namespace al;

struct Common {};

class MyApp : public al::DistributedAppWithState<Common> {
public:
  ////INITIAL OBJECTS AND DECLARATIONS////
  // ->
  al::Light light;
  al::Material material;
  al::FileSelector selector;
  al::SearchPaths searchPaths;
  std::string pointFragPath;
  std::string pointVertPath;
  std::string pointGeomPath;
  al::ShaderProgram pointShader;

  //// SCENE 6 DECLARE START

  // scene 6 MESHES
  al::VAOMesh jellyCreatureMesh;
  std::vector<al::Nav> jellies;

  Creature creature;

  // SCENE 6 PARAMS
  float scene2Boundary = 50.0f;
  bool inSphereScene2 = true;
  float jellieseperationThresh = 4.0f;
  int nAgentsScene6 = 6;
  float jelliesSpeedScene2 = 3.0;
  float jelliesizeScene2 = 5.0;
  float pointSize = 2.5;
  std::vector<al::Vec3f> colorPallete = {
      {1.0f, 0.0f, 0.5}, {0.11, 0.2, 0.46}, {0.11, 0.44, 0.46}};
  float flicker;
  float scene6pulseSpeed = 0.4;
  float scene6pulseAmount = 0.2;

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
  }

  void onCreate() override {
    nav().pos(0, 0, 0);
    pointShader.compile(slurp(pointVertPath), slurp(pointFragPath),
                        slurp(pointGeomPath));
    ////BIOLERPLATE////
    // nav().pos(al::Vec3d(jellies[0].pos())); // Set the camera to view the
    // scene
    sequencer().playSequence();

    // //INITIALIZE LIGHTING

    creature.makeJellyfish(
        jellyCreatureMesh, 0.6f, 72, 48, 8, 40, 40, 4.0f, 0.25f, 5.5f,
        1.0f); // less resolution than default by turning tendrils down
    for (int i = 0; i < jellyCreatureMesh.vertices().size(); ++i) {
      jellyCreatureMesh.color(1.0, 1, 1,
                              1); // Orange particles with alpha transparency
      jellyCreatureMesh.texCoord(1.0f, 0.0f);
    }
    jellyCreatureMesh.scale(jelliesizeScene2);
    jellyCreatureMesh.primitive(al::Mesh::POINTS);
    jellyCreatureMesh.generateNormals();
    jellyPulse.setBaseMesh(jellyCreatureMesh.vertices());
    // jellyRippleY.setParams(0.1, 0.1, 2.0, 'y');
    jellyPulse.setParams(scene6pulseSpeed, scene6pulseAmount, 1);
    // jellyEffectChain.pushBack(&jellyRippleY);
    jellyEffectChain.pushBack(&jellyPulse);

    jellyCreatureMesh.update();

    for (int b = 0; b < nAgentsScene6; ++b) {
      al::Nav p;
      p.pos() = randomVec3f(5);
      p.quat()
          .set(al::rnd::uniformS(), al::rnd::uniformS(), al::rnd::uniformS(),
               al::rnd::uniformS())
          .normalize();
      jellies.push_back(p);
    }
  }

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

    for (int i = 0; i < jellies.size(); ++i) {
      float t = globalTime + i * 10.0f; // moving at slightly diff rates

      // drifting sort of
      float wobbleAmount = 0.01f * std::sin(t * 0.7f);
      jellies[i].turnF(0.004f + wobbleAmount); //

      if (jellies[i].pos().mag() > scene2Boundary) {
        jellies[i].faceToward(al::Vec3f(0), 0.005f);
      }

      // bobbing
      // float bob = 0.001f * std::sin(t * 0.3f);
      // jellies[i].pos().y += bob;

      // standard move forward
      jellies[i].moveF(jelliesSpeedScene2 * 8.0);
      jellies[i].step(dt);
    }

    // SCENE 6 UPDATE AND PROCESS
    jellyPulse.setParams(scene6pulseSpeed, scene6pulseAmount, 1);
    jellyEffectChain.process(jellyCreatureMesh, sceneTime);

    jellyCreatureMesh.update();
    flicker = 0.25f + 0.05f * std::sin(sceneTime * 2.0);
    // end scene 6 animate
  }
  // END OF ANIMATE CALLBACK

  void onDraw(al::Graphics &g) override {

    //// SCENE 1 jellyT OF DRAW /////
    if (sceneIndex == 6) {

      // SCENE 6 WORLD LIGHTING
      glEnable(GL_BLEND);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);

      // glEnable(GL_PROGRAM_POINT_SIZE); <------COMMENTED OUT POINT SHADER
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

      // SCENE 6 SEQUENCING

      g.pointSize(pointSize);

      for (int i = 0; i < jellies.size(); ++i) {

        g.pushMatrix();
        g.translate(jellies[i].pos());
        g.rotate(jellies[i].quat());
        g.pointSize(2.0); //<----- REMOVE IF USING POINT SHADER
        g.color(1.0f, 0.4f, 0.7f,
                flicker); //<----- REMOVE IF USING POINT SHADER
        // g.shader(pointShader);<------COMMENTED OUT POINT
        //  pointShader.uniform("pointSize", 0.02); <------COMMENTED OUT POINT
        //  SHADER
        // pointShader.uniform("inputColor", al::Vec4f(1.0f, 0.4f, 0.7f,
        // flicker)); <------COMMENTED OUT POINT SHADER
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

  app.start();
  return 0;
}

std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}
