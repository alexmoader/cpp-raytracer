#pragma once

// В этом модуле я реализую генерацию луча исходящего из камеры в направлении определенного пикселя
// на экране

#include "options/camera_options.h"
#include "ray.h"
#include "vector.h"

#include <cmath>

// возвращает луч, исходящий из камеры в направлении пикселя (row, col)
inline Ray GenerateRay(const CameraOptions& opts, int row, int col) {
    double aspect = static_cast<double>(opts.screen_width) / opts.screen_height;
    double scale = std::tan(opts.fov / 2.0);

    // px, py - координаты центра пикселя в системе координат камеры
    double px = (2.0 * (col + 0.5) / opts.screen_width - 1.0) * scale * aspect;
    double py = (1.0 - 2.0 * (row + 0.5) / opts.screen_height) * scale;

    // Vec3f rayOrigin = Point3(0, 0, 0);
    // Matrix44f cameraToWorld;
    // cameraToWorld.set(...); // set matrix
    // Vec3f rayOriginWorld, rayPWorld;
    // cameraToWorld.multVectMatrix(rayOrigin, rayOriginWorld);
    // cameraToWorld.multVectMatrix(Vec3f(Px, Py, -1), rayPWorld);
    // Vec3f rayDirection = rayPWorld - rayOriginWorld;
    // rayDirection.normalize(); // it's a direction so don't forget to normalize

    // оталось выразить точку на экране через мировые координаты
    // для этого поймем где у камеры верх право и перед, чтобы найти базис и выразить через него
    // точку

    Vector forward(opts.look_to[0] - opts.look_from[0], opts.look_to[1] - opts.look_from[1],
                   opts.look_to[2] - opts.look_from[2]);
    forward.Normalize();

    Vector up(0, 1, 0);
    Vector right = CrossProduct(forward, up);
    if (Length(right) < 1e-9) {
        right = Vector(1, 0, 0);
        up = CrossProduct(right, forward);
    } else {
        right.Normalize();
        up = CrossProduct(right, forward);
    }

    // выражаем : world_point = look_from + px*right + py*up + forward
    Vector world_point(opts.look_from[0] + right[0] * px + up[0] * py + forward[0],
                       opts.look_from[1] + right[1] * px + up[1] * py + forward[1],
                       opts.look_from[2] + right[2] * px + up[2] * py + forward[2]);

    Vector origin = opts.look_from;

    Vector direction(world_point[0] - origin[0], world_point[1] - origin[1],
                     world_point[2] - origin[2]);
    direction.Normalize();

    return Ray(origin, direction);
}
