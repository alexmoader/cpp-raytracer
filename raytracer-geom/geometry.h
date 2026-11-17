#pragma once

#include "vector.h"
#include "sphere.h"
#include "intersection.h"
#include "triangle.h"
#include "ray.h"

#include <optional>
#include <limits>

std::optional<Intersection> GetIntersection(const Ray& ray, const Sphere& sphere) {

    constexpr float kEpsilon = std::numeric_limits<float>::epsilon();

    Vector oc(ray.GetOrigin()[0] - sphere.GetCenter()[0],
              ray.GetOrigin()[1] - sphere.GetCenter()[1],
              ray.GetOrigin()[2] - sphere.GetCenter()[2]);

    // коэфф-ты квадратного уравнения
    double a = DotProduct(ray.GetDirection(), ray.GetDirection());
    double b = 2.0f * DotProduct(oc, ray.GetDirection());
    double c = DotProduct(oc, oc) - sphere.GetRadius() * sphere.GetRadius();
    double discriminant = b * b - 4 * a * c;

    if (discriminant < 0) {
        return {};  // нет пересечения
    }

    double sqrt_disc = std::sqrt(discriminant);

    double t1 = (-b - sqrt_disc) / (2.0f * a);
    double t2 = (-b + sqrt_disc) / (2.0f * a);

    // наименшьй корень
    double t = t1;
    if (t < kEpsilon) {
        t = t2;
        if (t < kEpsilon) {
            return {};
        }
    }

    Vector position(ray.GetOrigin()[0] + ray.GetDirection()[0] * t,
                    ray.GetOrigin()[1] + ray.GetDirection()[1] * t,
                    ray.GetOrigin()[2] + ray.GetDirection()[2] * t);

    Vector normal(position[0] - sphere.GetCenter()[0], position[1] - sphere.GetCenter()[1],
                  position[2] - sphere.GetCenter()[2]);
    normal.Normalize();

    if (DotProduct(normal, ray.GetDirection()) > 0.0) {
        normal[0] = -normal[0];
        normal[1] = -normal[1];
        normal[2] = -normal[2];
    }
    return Intersection(position, normal, t);
}

std::optional<Intersection> GetIntersection(const Ray& ray, const Triangle& triangle) {

    constexpr float kEpsilon = std::numeric_limits<float>::epsilon();

    Vector edge1(triangle[1][0] - triangle[0][0], triangle[1][1] - triangle[0][1],
                 triangle[1][2] - triangle[0][2]);

    Vector edge2(triangle[2][0] - triangle[0][0], triangle[2][1] - triangle[0][1],
                 triangle[2][2] - triangle[0][2]);

    Vector normal = CrossProduct(edge1, edge2);

    if (DotProduct(normal, ray.GetDirection()) > 0) {
        // Тут надо развернуть нормаль по заданию
        normal[0] = -normal[0];
        normal[1] = -normal[1];
        normal[2] = -normal[2];
    }
    normal.Normalize();
    Vector ray_cross_e2 = CrossProduct(ray.GetDirection(), edge2);
    double det = DotProduct(edge1, ray_cross_e2);
    if (fabs(det) < kEpsilon) {
        return {};  // ray is parallel
    }

    double inv_det = 1.0 / det;
    Vector ray_origin = ray.GetOrigin();
    Vector s(ray_origin[0] - triangle[0][0], ray_origin[1] - triangle[0][1],
             ray_origin[2] - triangle[0][2]);
    double u = inv_det * DotProduct(s, ray_cross_e2);

    if (u < -kEpsilon || u - 1 > kEpsilon) {
        return {};  // ray passes outside edge2's bounds
    }

    Vector s_cross_e1 = CrossProduct(s, edge1);
    double v = inv_det * DotProduct(ray.GetDirection(), s_cross_e1);

    if (v < -kEpsilon || u + v - 1 > kEpsilon) {
        return {};  // ray passes outside edge1's bounds
    }

    // The ray line intersects with the triangle.
    // We compute t to find where on the ray the intersection is.
    double t = inv_det * DotProduct(edge2, s_cross_e1);

    if (t > kEpsilon)  // Ray intersection
    {
        Vector position(ray.GetOrigin()[0] + ray.GetDirection()[0] * t,
                        ray.GetOrigin()[1] + ray.GetDirection()[1] * t,
                        ray.GetOrigin()[2] + ray.GetDirection()[2] * t);
        return Intersection(position, normal, t);
    } else {
        // This means that there is a line intersection but not a ray intersection.
        return {};
    }
}

// функция для вычисления отраженного луча в точке
Vector Reflect(const Vector& ray, const Vector& normal) {
    // I - 2.0 * dot(N, I) * N. N should be normalized in order to achieve the desired result
    return Vector(ray[0] - 2.0 * DotProduct(normal, ray) * normal[0],
                  ray[1] - 2.0 * DotProduct(normal, ray) * normal[1],
                  ray[2] - 2.0 * DotProduct(normal, ray) * normal[2]);
}

// функция для вычисления преломленного луча в точке
std::optional<Vector> Refract(const Vector& ray, const Vector& normal, double eta) {
    // For a given incident vector I, surface normal N and ratio of indices of refraction, eta,
    // refract returns the refraction vector, R. R is calculated as:
    //     k = 1.0 - eta * eta * (1.0 - dot(N, I) * dot(N, I));
    //     if (k < 0.0)
    //         R = genType(0.0);       // or genDType(0.0)
    //     else
    //         R = eta * I - (eta * dot(N, I) + sqrt(k)) * N;
    // The input parameters I and N should be normalized in order to achieve the desired result.
    Vector r;
    double k = 1.0 - eta * eta * (1.0 - DotProduct(normal, ray) * DotProduct(normal, ray));
    if (k < 0.0) {
        return std::nullopt;
    } else {
        double temp = (eta * DotProduct(normal, ray) + std::sqrt(k));
        r[0] = eta * ray[0] - temp * normal[0];
        r[1] = eta * ray[1] - temp * normal[1];
        r[2] = eta * ray[2] - temp * normal[2];
    }
    return std::make_optional(r);
}

Vector GetBarycentricCoords(const Triangle& triangle, const Vector& point) {
    double s_abc = triangle.Area();
    double s_abp = Triangle(triangle[0], triangle[1], point).Area();
    double s_cap = Triangle(triangle[0], triangle[2], point).Area();
    double s_bcp = Triangle(triangle[1], triangle[2], point).Area();

    return Vector(s_bcp / s_abc, s_cap / s_abc, s_abp / s_abc);
}
