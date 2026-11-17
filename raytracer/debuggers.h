#pragma once

// тут я реализую режимы kDepth и kNormal

#include "ray.h"
#include "raygen.h"
#include "find_intersection.h"
#include "intersection.h"
#include "scene.h"
#include "image.h"
#include "options/camera_options.h"
#include "options/render_options.h"

#include <vector>
#include <algorithm>
#include <optional>

Image RenderDebugModes(const Scene& scene, const CameraOptions& camera_opts,
                       const RenderMode& render_mode) {
    int w = camera_opts.screen_width;
    int h = camera_opts.screen_height;
    Image image(w, h);

    if (render_mode == RenderMode::kDepth) {
        std::vector<std::vector<double>> depth_map(h, std::vector<double>(w, -1.0));
        double max_depth = 0.0;

        // собираем расстояния, чтобы найти максимум
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                Ray ray = GenerateRay(camera_opts, row, col);
                std::optional<Intersection> intersection = FindIntersection(ray, scene);
                if (intersection) {
                    double distance = intersection->GetDistance();
                    depth_map[row][col] = distance;
                    max_depth = std::max(max_depth, distance);
                }
            }
        }

        // записываем цвета
        // void SetPixel(const RGB& pixel, int y, int x)
        // struct RGB {
        //     int r, g, b;
        // };
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                double d = depth_map[row][col];
                if (d >= 0.0) {
                    double normalized_depth = d / max_depth;
                    int c = static_cast<int>(std::round(normalized_depth * 255.0));
                    image.SetPixel({c, c, c}, row, col);
                } else {
                    image.SetPixel({255, 255, 255}, row, col);
                }
            }
        }

    } else if (render_mode == RenderMode::kNormal) {
        for (int row = 0; row < h; ++row) {
            for (int col = 0; col < w; ++col) {
                Ray ray = GenerateRay(camera_opts, row, col);
                std::optional<Intersection> intersection = FindIntersection(ray, scene);
                if (intersection) {
                    const Vector& normal = intersection->GetNormal();
                    int r = static_cast<int>(std::round((normal[0] * 0.5 + 0.5) * 255.0));
                    int g = static_cast<int>(std::round((normal[1] * 0.5 + 0.5) * 255.0));
                    int b = static_cast<int>(std::round((normal[2] * 0.5 + 0.5) * 255.0));
                    image.SetPixel({r, g, b}, row, col);
                } else {
                    image.SetPixel({0, 0, 0}, row, col);
                }
            }
        }
    }

    return image;
}
