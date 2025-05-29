#ifndef IMAGE_COLOR_TO_MESH_HPP
#define IMAGE_COLOR_TO_MESH_HPP

#include "al/graphics/al_Texture.hpp"
#include "al/graphics/al_Image.hpp"
#include "al/graphics/al_VAOMesh.hpp"
#include <iostream>

struct NewColorBuffer {
    // Loads an image and directly applies its colors to the target mesh
    void imageToNewMesh(const std::string& imagePath, al::VAOMesh& targetMesh) {
        al::Image image;
        if (!image.load(imagePath)) {
            std::cerr << "Failed to load image from " << imagePath << std::endl;
            return;
        }

        int meshVertexCount = targetMesh.vertices().size();
        int totalPixels = image.width() * image.height();
        int limit = std::min(meshVertexCount, totalPixels);

        // Clear and ensure target mesh has a color buffer of the correct size
        targetMesh.colors().clear();
        targetMesh.colors().reserve(meshVertexCount);

        // Map image pixels to the mesh color buffer
        for (int i = 0; i < limit; ++i) {
            int x = i % image.width();
            int y = i / image.width();
            auto pixel = image.at(x, y);

            float r = pixel.r / 255.0f;
            float g = pixel.g / 255.0f;
            float b = pixel.b / 255.0f;
            float a = pixel.a / 255.0f;

            targetMesh.color(al::Color(r, g, b, a)); // Directly add color
        }

        // If the image has fewer pixels than vertices, the remaining are set to black
        for (int i = limit; i < meshVertexCount; ++i) {
            targetMesh.color(al::Color(0.0f, 0.0f, 0.0f, 1.0f)); // Default black
        }

        // Update the mesh to apply the new colors
        targetMesh.update();
        std::cout << "Applied image colors to mesh. Total vertices colored: " 
                  << limit << " out of " << meshVertexCount << std::endl;
    }
};

#endif
