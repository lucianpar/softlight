#ifndef UTILITY_HPP
#define UTILITY_HPP

#include "al/graphics/al_VAOMesh.hpp"
#include "al/math/al_.hpp"
#include "al/math/al_Complex.hpp"
#include "al/math/al_Vec.hpp"

al::Vec3f randomVec3f(float scale) {
  return al::Vec3f(al::rnd::uniformS(), al::rnd::uniformS(),
                   al::rnd::uniformS()) *
         scale;
}

#endif