#ifndef SOUND_OBJECT_HPP
#define SOUND_OBJECT_HPP

#include <functional>
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/scene/al_SynthVoice.hpp"







class SoundObject : public al::SynthVoice {
  gam::SamplePlayer<> mSource;
  al::Mesh mesh;//{al::Mesh()};
  al::Vec3f position;//{0.0f, 0.0f, 0.0f};
  float mSize{1.0f};
  double time = 0.0;
  bool showingObject;

public:
  typedef std::function<al::Vec3f(double, const al::Vec3f&)> FuncType;
  FuncType trajectory;

  inline SoundObject() {
    al::addSphere(mesh, 5.0);
    mesh.primitive(al::Mesh::POINTS);
    //std::cout << "SoundObject Mesh Vertices: " << mesh.vertices().size() << std::endl;
  }

  void onProcess(al::AudioIOData& io) override {
    // this->position(1,dt%100,)
    while (io()) {
      float sampleValue = mSource(); //temporary fix
      io.out(0) += sampleValue;
      io.out(1) += sampleValue;
    }
    if (mSource.done()) {
      free();
    }
  }

  void update(double dt) override {

    time = dt;
    // if the trajectory function is real, then call it
    if (trajectory) {
    //  printf("time: %f\n", time);
    //   fflush(stdout);
      // update our position based on this custom function
      position = trajectory(time, position);
    }
    
  }

  void onProcess(al::Graphics& g) override {
    //mesh.update();
    g.pushMatrix();
    g.translate(position);
    g.color(1.0,0.0,0.0);
    g.scale(mSize * mSource.pos() / mSource.max());
    if (showingObject){
    g.draw(mesh);
    }
    g.popMatrix();
  }

  void set(float x, float y, float z, float size, bool showObject, const char* filename, FuncType func) {
    trajectory = func;       // Set the trajectory function
    mSource.load(filename);  // Load a sound file
    position.set(x, y, z);   // Set the position of the voice
    mSize = size;
    showingObject = showObject;
  }

  void onTriggerOn() override {
    time = 0;
    mSource.reset();
  }
  void onTriggerOff() override { mSource.finish(); }
};

#endif  // SOUND_OBJECT_HPP