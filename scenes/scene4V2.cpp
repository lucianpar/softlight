#include "Gamma/Domain.h"
#include "Gamma/Envelope.h"
#include "Gamma/Oscillator.h"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Graphics.hpp"
#include "al/graphics/al_Light.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/io/al_ControlNav.hpp"
#include "al/math/al_Random.hpp"
#include "al/math/al_Vec.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/ui/al_ControlGUI.hpp"
#include "al/ui/al_Parameter.hpp"
#include "al/ui/al_PresetSequencer.hpp"
#include "al_ext/assets3d/al_Asset.hpp"
#include <iostream>
#include <ostream>
#include <string>

// MY CUSTOM INCLUDES:
#include "../eoys-mesh-fx/orbit.hpp"
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
#include "../utility/imageColorToMesh.hpp"

/*
* sequence
* work on removing oldest mesh vertices
*update world color to flow into next scene
* add more mirrors
*figure out spacings

*/
// boilerplate
std::string slurp(const std::string &fileName);

class MyApp : public al::App {
public:
  al::ShaderProgram pointShader;

  al::Light light;
  // Light light;
  al::Material material;

  // al::Parameter width{"Width", 0.05, 0, 0.2};
  // Meshes and Effects
  std::vector<al::Nav> structures;
  int nAgentsScene4 = 40;
  al::VAOMesh meshBall;
  float ballSpeedScene4 = 0.5;
  al::VAOMesh referenceMesh;
  Attractor referenceAttractor;
  al::VAOMesh cloudMesh;
  al::VAOMesh cloudMesh2;

  // Global Time
  double globalTime = 0;
  double sceneTime = 0;
  float pointSize = 5.0f; // Particle size

  void onInit() override { gam::sampleRate(audioIO().framesPerSecond()); }

  void onCreate() override {
    // boilerplate
    pointShader.compile(slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-vertex.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-fragment.glsl"),
                        slurp("/Users/lucian/Desktop/201B/allolib_playground/"
                              "softlight-sphere-new/softlight/softlight/"
                              "utility/point-geometry.glsl"));
    //
    // nav().pos(al::Vec3d(head.pos())); //

    // al::addSphere(test);

    // Initialize Mesh
    referenceAttractor.makeNoiseCube(referenceMesh, 5.0, nAgentsScene4);

    referenceMesh.update();

    referenceAttractor.makeNoiseCube(cloudMesh, 5.0, 20000);
    cloudMesh.primitive(al::Mesh::POINTS);
    for (int i = 0; i < cloudMesh.vertices().size(); ++i) {
      cloudMesh.color(1.0, 0.3, 0.1, 1.0); // or any RGBA
      cloudMesh.texCoord(1.0, 0.0);        // sets vertexSize.x
    }
    // cloudMesh.update(); // reupload with new attribs
    cloudMesh.update();

    al::addIcosphere(meshBall, 0.02, 1);
    meshBall.primitive(al::Mesh::LINE_LOOP);
    meshBall.update();

    // scene4ShellMesh.primitive(al::Mesh::LINE_LOOP);
    // scene4ShellMesh.update();

    std::cout << "totalVerts: " << meshBall.vertices().size() * nAgentsScene4
              << std::endl;

    for (int b = 0; b < nAgentsScene4; ++b) {
      al::Nav p;
      p.pos() = referenceMesh.vertices()[b];
      p.quat().set(1, 1, 1, 1).normalize();
      // p.set(randomVec3f(5), randomVec3f(1));
      structures.push_back(p);
      // velocity.push_back(al::Vec3f(0));
      // force.push_back(al::Vec3f(0));
    }
    // al::addSphere(referenceMesh, 10.0, 10.0);
    referenceMesh.primitive(al::Mesh::POINTS);

    // target.set(al::rnd::uniformS(), al::rnd::uniformS(),
    // al::rnd::uniformS());

    // makePointLine()
  }

  void onAnimate(double dt) override {
    globalTime += dt;
    sceneTime += dt;

    /// trying newer effect
    // referenceAttractor.processRossler(newAttractor, dt, 1.0);
    // referenceAttractor.processLorenz(newAttractor, dt, 1.0);

    referenceAttractor.processSprottLinzF(referenceMesh, dt, ballSpeedScene4,
                                          0.11);
    referenceMesh.update();

    referenceAttractor.processThomas(cloudMesh, dt, 1.0, 0.11);
    for (auto &v : cloudMesh.vertices()) {
      v += al::Vec3f(0, 0, -0.035); // or whatever offset you want
    }
    cloudMesh.update();

    for (int i = 0; i < structures.size(); ++i) {
      structures[i].faceToward(referenceMesh.vertices()[i]);
      structures[i].moveF(ballSpeedScene4);
      structures[i].step(dt);
    }
    // if (sceneTime <= 6.0) {
    //   cloudMesh.translate(0, 0, 0.1);
    // }

    // sunEffectChain.process(sunMesh, sceneTime); // toggle orbiting
  }

  void onDraw(al::Graphics &g) override {
    // glEnable(GL_BLEND);
    //  g.blendTrans();
    g.depthTesting(true);

    g.clear(1, 0.969, 0.906);

    // g.depthTesting(true);
    g.blending(true);
    // glEnable(GL_BLEND);
    // glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    g.blendTrans();
    // g.depthTesting(true);

    // g.lighting(false);
    // // lighting from karl's example
    // light.globalAmbient(al::RGB(1.0, 1.0, 1.0));
    // light.ambient(al::RGB(1.0, 1.0, 1.0));
    // light.diffuse(al::RGB(1, 1, 1.0));
    // g.light(light);
    // material.specular(1.0);
    // material.shininess(10);
    // g.material(material);

    // g.pointSize(pointSize);
    //  g.color(0.871, 0.467, 0.192, 0.1);
    // for (int i = 0; i < structures.size(); ++i) {
    //   g.pushMatrix();
    //   g.color(0.871, 0.467, 0.192, 1.0);
    //   g.translate(structures[i].pos());
    //   g.rotate(structures[i].quat());

    //   //g.draw(meshBall);
    //   // g.draw(cloudMesh);
    //   g.popMatrix();
    // }
    g.shader(pointShader);
    pointShader.uniform("pointSize", 0.009);
    pointShader.uniform("inputColor", al::Vec4f(0.055, 0.478, 0.44, 1.0));

    // g.color(0.0);
    g.draw(cloudMesh);
    // cloudMesh.update();
    // g.meshColor();
    //  g.draw(sunMesh);
    //  g.draw(scene4ShellMesh);
    //  g.draw(referenceMesh);
    //  g.draw(ribbon);
    //  g.draw(reflectedRibbon);
    //    glowShader.end();
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

// boilerplate
std::string slurp(const std::string &fileName) {
  std::ifstream file(fileName);
  std::string contents((std::istreambuf_iterator<char>(file)),
                       std::istreambuf_iterator<char>());
  return contents;
}