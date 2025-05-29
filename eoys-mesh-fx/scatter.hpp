#ifndef SCATTER_EFFECT_HPP
#define SCATTER_EFFECT_HPP

#include "al/math/al_Vec.hpp"
#include "vfxUtility.hpp"
#include <vector>
#include <random>
#include <iostream>

/**
 * @brief ScatterEffect scales entire mesh inward and outward at pulse rate.
 * It must use .setBaseMesh in onCreate to create a reference point for scaling.
 * Can be updated in onAnimate. Must be passed into a VFX Chain instance via effectChain.push_back in onCreate.
 */
class ScatterEffect : public VertexEffect {
public:
    float rate = 1.0f;     // Seconds to reach displacement target
    float amount = 0.2f;   // max displacement amount in GL units
    bool outward = false;
    bool inward = false;

    std::vector<al::Vec3f> displacementVector;
    std::vector<al::Vec3f> targetVertices;

    // easiest random num for now
    std::mt19937 rng{std::random_device{}()};
    std::uniform_real_distribution<float> dist{-1.0f, 1.0f};

    /**
     * @brief Set effect parameters
     * 
     * @param speed  Seconds to reach displacement target
     * @param Amount Max displacement amount in GL units
     */
    void setParams(float speed, float Amount) {
        rate = speed;
        amount = Amount;
    }

    /**
     * @brief Initialize the scatter vector for displacement
     * 
     * @param mesh The target VAOMesh to apply the effect to
     */
    void setScatterVector(al::VAOMesh& mesh) {
        displacementVector.clear();
        displacementVector.resize(mesh.vertices().size());

        for (size_t i = 0; i < mesh.vertices().size(); ++i) {
            displacementVector[i] = al::Vec3f(
                dist(rng) * amount, 
                dist(rng) * amount, 
                dist(rng) * amount
            );
        }

        std::cout << "ScatterEffect: Displacement vector set with " << displacementVector.size() << " elements.\n";
    }

    /**
     * @brief Trigger scatter effect outward
     * 
     * @param mesh The target VAOMesh to apply the effect to
     */
    void triggerOut(bool, al::VAOMesh& mesh) {
        if (baseVerts.empty()) {
            std::cerr << "ScatterEffect Error: baseVerts not initialized.\n";
            return;
        }

        inward = false;
        outward = true;
        targetVertices.resize(mesh.vertices().size());

        for (size_t i = 0; i < mesh.vertices().size(); ++i) {
            targetVertices[i] = baseVerts[i] + displacementVector[i];
        }

        //std::cout << "Scatter Triggered out" << std::endl;
    }

    /**
     * @brief Trigger scatter effect inward (reset to base)
     */
    void triggerIn(bool) {
        if (baseVerts.empty()) {
            std::cerr << "ScatterEffect Error: baseVerts not initialized.\n";
            return;
        }

        outward = false;
        inward = true;
        targetVertices = baseVerts;
        //std::cout << "Scatter Triggered in (reset)" << std::endl;
    }

    /**
     * @brief Stop the scatter effect
     */
    void stop(bool) {
        outward = false;
        inward = false;
        std::cout << "Scatter stopped.\n";
    }

    /**
     * @brief Process the scatter effect on the mesh
     * 
     * @param mesh The target VAOMesh to apply the effect to
     * @param t    Time parameter (for timing-based effects, currently unused)
     */
    void process(al::VAOMesh& mesh, float t) override {
        auto& verts = mesh.vertices();
        if (verts.size() != baseVerts.size()) {
            std::cerr << "ScatterEffect Error: Vertex count mismatch.\n";
            return;
        }

        if (outward) {
            for (size_t i = 0; i < verts.size(); ++i) {
                verts[i] += (targetVertices[i] - verts[i]) / (rate * 60.0f);
            }
        }

        if (inward) {
            for (size_t i = 0; i < verts.size(); ++i) {
                verts[i] += (baseVerts[i] - verts[i]) / (rate * 60.0f);
            }
        }

        mesh.update();
    }
};

#endif // SCATTER_EFFECT_HPP
