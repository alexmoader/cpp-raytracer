#pragma once

#include "ray.h"
#include "geometry.h"
#include "intersection.h"
#include "scene.h"

#include <optional>
#include <limits>

struct SceneHit {
    Vector position;
    Vector normal;
    double distance;
    const Material* material;
    bool inside;
};
// находит куда попадает луч в сцене
std::optional<Intersection> FindIntersection(const Ray& ray, const Scene& scene) {
    double closest = std::numeric_limits<double>::max();
    std::optional<Intersection> result;

    // spheres
    for (const auto& sphere : scene.GetSphereObjects()) {
        std::optional<Intersection> inter = GetIntersection(ray, sphere.sphere);
        if (inter && inter->GetDistance() < closest) {
            closest = inter->GetDistance();
            result = inter;
        }
    }

    // for triangles
    for (const auto& triangle : scene.GetObjects()) {
        std::optional<Intersection> inter = GetIntersection(ray, triangle.polygon);
        if (inter && inter->GetDistance() < closest) {
            closest = inter->GetDistance();
            result = inter;

            // В .obj файле сцены для вершин треугольника могут быть явно заданы нормали,
            // в таком случае нормаль к треугольнику в точке считается
            // как проинтерполированное значение нормалей в вершинах с использованием
            // барицентрических координат. это взвешенное среднее трех нормалей, где веса -
            // барицентрические координаты!!
            Vector barycentric_coords =
                GetBarycentricCoords(triangle.polygon, inter->GetPosition());
            Vector n0 = *triangle.GetNormal(0);
            Vector n1 = *triangle.GetNormal(1);
            Vector n2 = *triangle.GetNormal(2);

            bool has_normals = (Length(n0) > 0 || Length(n1) > 0 || Length(n2) > 0);

            Vector normal;
            if (has_normals) {
                normal = Vector(n0[0] * barycentric_coords[0] + n1[0] * barycentric_coords[1] +
                                    n2[0] * barycentric_coords[2],
                                n0[1] * barycentric_coords[0] + n1[1] * barycentric_coords[1] +
                                    n2[1] * barycentric_coords[2],
                                n0[2] * barycentric_coords[0] + n1[2] * barycentric_coords[1] +
                                    n2[2] * barycentric_coords[2]);
                normal.Normalize();
            } else {
                normal = inter->GetNormal();
            }

            // против луча
            if (DotProduct(normal, ray.GetDirection()) > 0.0) {
                normal = Vector(-normal[0], -normal[1], -normal[2]);
            }

            result = Intersection(inter->GetPosition(), normal, inter->GetDistance());
        }
    }

    return result;
}

// находит куда попадает луч в сцене и возвращает информацию о материале
std::optional<SceneHit> FindIntersectionWithMaterial(const Ray& ray, const Scene& scene) {
    double closest = std::numeric_limits<double>::max();
    std::optional<SceneHit> result;

    for (const auto& sphere : scene.GetSphereObjects()) {
        std::optional<Intersection> inter = GetIntersection(ray, sphere.sphere);
        if (inter && inter->GetDistance() < closest) {
            closest = inter->GetDistance();
            Vector outward_normal(inter->GetPosition()[0] - sphere.sphere.GetCenter()[0],
                                  inter->GetPosition()[1] - sphere.sphere.GetCenter()[1],
                                  inter->GetPosition()[2] - sphere.sphere.GetCenter()[2]);
            outward_normal.Normalize();
            bool inside = DotProduct(ray.GetDirection(), outward_normal) > 0.0;
            Vector normal = inside
                                ? Vector(-outward_normal[0], -outward_normal[1], -outward_normal[2])
                                : outward_normal;
            result = SceneHit{inter->GetPosition(), normal, inter->GetDistance(), sphere.material,
                              inside};
        }
    }

    for (const auto& triangle : scene.GetObjects()) {
        std::optional<Intersection> inter = GetIntersection(ray, triangle.polygon);
        if (inter && inter->GetDistance() < closest) {
            closest = inter->GetDistance();

            Vector barycentric_coords =
                GetBarycentricCoords(triangle.polygon, inter->GetPosition());
            Vector n0 = *triangle.GetNormal(0);
            Vector n1 = *triangle.GetNormal(1);
            Vector n2 = *triangle.GetNormal(2);

            bool has_normals = (Length(n0) > 0 || Length(n1) > 0 || Length(n2) > 0);

            Vector normal;
            if (has_normals) {
                normal = Vector(n0[0] * barycentric_coords[0] + n1[0] * barycentric_coords[1] +
                                    n2[0] * barycentric_coords[2],
                                n0[1] * barycentric_coords[0] + n1[1] * barycentric_coords[1] +
                                    n2[1] * barycentric_coords[2],
                                n0[2] * barycentric_coords[0] + n1[2] * barycentric_coords[1] +
                                    n2[2] * barycentric_coords[2]);
                normal.Normalize();
            } else {
                normal = inter->GetNormal();
            }

            bool inside = DotProduct(normal, ray.GetDirection()) > 0.0;
            if (inside) {
                normal = Vector(-normal[0], -normal[1], -normal[2]);
            }

            result = SceneHit{inter->GetPosition(), normal, inter->GetDistance(), triangle.material,
                              inside};
        }
    }

    return result;
}
