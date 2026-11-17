#include "raytracer.h"

#include <exception>
#include <iostream>
#include <string_view>

int main(int argc, char* argv[]) {
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: raytracer <scene.obj> <output.ppm> [full|depth|normals]\n";
        return 1;
    }

    RenderOptions options{.depth = 5};
    const std::string_view mode = argc == 4 ? argv[3] : "full";
    if (mode == "depth") {
        options.mode = RenderMode::kDepth;
    } else if (mode == "normals") {
        options.mode = RenderMode::kNormal;
    } else if (mode != "full") {
        std::cerr << "Unknown render mode: " << mode << '\n';
        return 1;
    }

    try {
        const CameraOptions camera{.screen_width = 480, .screen_height = 360};
        Render(argv[1], camera, options).Write(argv[2]);
    } catch (const std::exception& error) {
        std::cerr << error.what() << '\n';
        return 1;
    }
}
