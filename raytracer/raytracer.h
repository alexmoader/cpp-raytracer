#pragma once

#include "options/camera_options.h"
#include "options/render_options.h"
#include "image.h"

#include "raygen.h"
#include "find_intersection.h"
#include "light_calculator.h"
#include "debuggers.h"

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <vector>

static int ToRgb255(double value) {
    value = std::clamp(value, 0.0, 1.0);
    return static_cast<int>(std::round(value * 255.0));
}
Image Render(const std::filesystem::path& path, const CameraOptions& camera_options,
             const RenderOptions& render_options) {
    Scene scene = ReadScene(path);

    if (render_options.mode == RenderMode::kDepth || render_options.mode == RenderMode::kNormal) {
        return RenderDebugModes(scene, camera_options, render_options.mode);
    }

    // kFull
    Image image(camera_options.screen_width, camera_options.screen_height);
    std::vector<Vector> colors;
    colors.reserve(static_cast<size_t>(camera_options.screen_width) *
                   static_cast<size_t>(camera_options.screen_height));

    double max_color = 0.0;
    for (int row = 0; row < camera_options.screen_height; ++row) {
        for (int col = 0; col < camera_options.screen_width; ++col) {
            Ray ray = GenerateRay(camera_options, row, col);
            auto hit = FindIntersectionWithMaterial(ray, scene);
            Vector color;
            if (!hit) {
                color = Vector(0, 0, 0);
            } else {
                color = RayTrace(ray, scene, render_options, render_options.depth);
            }
            max_color = std::max({max_color, color[0], color[1], color[2]});
            colors.push_back(color);
        }
    }

    size_t index = 0;
    for (int row = 0; row < camera_options.screen_height; ++row) {
        for (int col = 0; col < camera_options.screen_width; ++col, ++index) {
            Vector color = colors[index];
            if (max_color > 0.0) {
                for (int channel = 0; channel < 3; ++channel) {
                    color[channel] = color[channel] *
                                     (1.0 + color[channel] / (max_color * max_color)) /
                                     (1.0 + color[channel]);
                    color[channel] = std::pow(std::max(0.0, color[channel]), 1.0 / 2.2);
                }
            }
            image.SetPixel({ToRgb255(color[0]), ToRgb255(color[1]), ToRgb255(color[2])}, row, col);
        }
    }

    return image;
}
