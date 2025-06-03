#ifndef STATIC_SOUND_OBJECT_HPP
#define STATIC_SOUND_OBJECT_HPP

#include "Gamma/SamplePlayer.h"
#include "al/app/al_App.hpp"
#include "al/graphics/al_Mesh.hpp"
#include "al/graphics/al_Shapes.hpp"
#include "al/graphics/al_VAO.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include "al/scene/al_SynthSequencer.hpp"
#include "al/scene/al_SynthVoice.hpp"
#include <functional>

class SoundObject : public al::SynthVoice {
  gam::SamplePlayer<> mSource;
  al::Vec3f position; //{0.0f, 0.0f, 0.0f};
  double time = 0.0;

public:
  typedef std::function<al::Vec3f(double, const al::Vec3f &)> FuncType;
  FuncType trajectory;

  void onProcess(al::AudioIOData &io) override {
    // this->position(1,dt%100,)
    while (io()) {
      float sampleValue = mSource() * 0.5; // temporary fix
      io.out(0) += sampleValue;
      io.out(1) += sampleValue;

      // io.bus(0) += sampleValue;

      // io.out(47) += sampleValue;
    }
    if (mSource.done()) {
      free();
    }
  }

  void set(const char *filename, float x = 0, float y = 0, float z = 0) {
    // Set the trajectory function
    mSource.load(filename); // Load a sound file
    position.set(x, y, z);  // Set the position of the voice
  }

  void onTriggerOn() override {
    time = 0;
    mSource.reset();
  }
  void onTriggerOff() override { mSource.finish(); }
};

#endif // SOUND_OBJECT_HPP