#pragma once

// тут я реализую функцию трассировки луча - возвращает освещенность в направлении луча

#include "vector.h"
#include "ray.h"
#include "geometry.h"
#include "scene.h"
#include "material.h"
#include "light.h"
#include "find_intersection.h"
#include "options/render_options.h"

#include <algorithm>
#include <cmath>

constexpr double kEps = 1e-6;

inline bool IsShadowed(const Vector& point, const Vector& light_pos, const Scene& scene) {
    Vector dir(light_pos[0] - point[0], light_pos[1] - point[1], light_pos[2] - point[2]);
    double dist_to_light = Length(dir);
    dir.Normalize();

    Vector origin(point[0] + dir[0] * kEps, point[1] + dir[1] * kEps, point[2] + dir[2] * kEps);

    Ray shadow_ray(origin, dir);

    auto hit = FindIntersectionWithMaterial(shadow_ray, scene);
    if (hit && hit->distance < dist_to_light - kEps) {
        return true;
    }
    return false;
}

// трассировка луча - рекурсивный расчет освещенности по модели Фонга
inline Vector RayTrace(const Ray& ray, const Scene& scene, const RenderOptions& opts, int depth) {
    if (depth <= 0) {
        return Vector(0, 0, 0);
    }

    auto hit = FindIntersectionWithMaterial(ray, scene);
    if (!hit) {
        return Vector(0, 0, 0);
    }

    if (hit->material == nullptr) {
        return Vector(0, 0, 0);
    }

    const Material& mat = *hit->material;
    const Vector& point = hit->position;
    const Vector& normal = hit->normal;

    Vector view_dir(-ray.GetDirection()[0], -ray.GetDirection()[1], -ray.GetDirection()[2]);

    Vector diffuse_sum(0, 0, 0);
    Vector specular_sum(0, 0, 0);

    for (const auto& light : scene.GetLights()) {
        if (IsShadowed(point, light.position, scene)) {
            continue;
        }

        Vector to_light(light.position[0] - point[0], light.position[1] - point[1],
                        light.position[2] - point[2]);
        to_light.Normalize();

        double diff = std::max(0.0, DotProduct(to_light, normal));
        diffuse_sum[0] += light.intensity[0] * diff;
        diffuse_sum[1] += light.intensity[1] * diff;
        diffuse_sum[2] += light.intensity[2] * diff;

        Vector from_light(-to_light[0], -to_light[1], -to_light[2]);
        Vector reflected = Reflect(from_light, normal);

        double spec = std::max(0.0, DotProduct(reflected, view_dir));  // max(0, dot(v_e, v_lr))
        spec = std::pow(spec, mat.specular_exponent);                  // ^Ns
        specular_sum[0] += light.intensity[0] * spec;
        specular_sum[1] += light.intensity[1] * spec;
        specular_sum[2] += light.intensity[2] * spec;
    }  // I * S_i

    // result = al0 * (Kd * ΣDi + Ks * ΣSi) + Ka + Ke
    Vector result(
        mat.albedo[0] *
                (mat.diffuse_color[0] * diffuse_sum[0] + mat.specular_color[0] * specular_sum[0]) +
            mat.ambient_color[0] + mat.intensity[0],
        mat.albedo[0] *
                (mat.diffuse_color[1] * diffuse_sum[1] + mat.specular_color[1] * specular_sum[1]) +
            mat.ambient_color[1] + mat.intensity[1],
        mat.albedo[0] *
                (mat.diffuse_color[2] * diffuse_sum[2] + mat.specular_color[2] * specular_sum[2]) +
            mat.ambient_color[2] + mat.intensity[2]);

    bool entering = !hit->inside;

    if (mat.albedo[1] > 0.0 && entering) {
        Vector refl_dir = Reflect(ray.GetDirection(), normal);
        refl_dir.Normalize();

        Vector refl_origin(point[0] + normal[0] * kEps, point[1] + normal[1] * kEps,
                           point[2] + normal[2] * kEps);
        Ray refl_ray(refl_origin, refl_dir);

        Vector refl_color = RayTrace(refl_ray, scene, opts, depth - 1);
        result[0] += mat.albedo[1] * refl_color[0];
        result[1] += mat.albedo[1] * refl_color[1];
        result[2] += mat.albedo[1] * refl_color[2];
    }

    if (mat.albedo[2] > 0.0 || !entering) {
        double eta;
        Vector refract_normal;
        double transparency;

        if (entering) {
            eta = 1.0 / mat.refraction_index;
            refract_normal = normal;
            transparency = mat.albedo[2];
        } else {
            eta = mat.refraction_index;
            refract_normal = normal;  // ошибка со знаком была
            transparency = 1.0;
        }

        auto refr = Refract(ray.GetDirection(), refract_normal, eta);
        if (refr) {
            refr->Normalize();

            Vector refr_origin(point[0] - refract_normal[0] * kEps,
                               point[1] - refract_normal[1] * kEps,
                               point[2] - refract_normal[2] * kEps);

            Ray refr_ray(refr_origin, *refr);

            Vector refr_color = RayTrace(refr_ray, scene, opts, depth - 1);
            result[0] += transparency * refr_color[0];
            result[1] += transparency * refr_color[1];
            result[2] += transparency * refr_color[2];
        }
    }

    return result;
}
