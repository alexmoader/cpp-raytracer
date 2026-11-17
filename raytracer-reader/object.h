#pragma once

#include "triangle.h"
#include "material.h"
#include "sphere.h"
#include "vector.h"

struct Object {
    const Material* material = nullptr;
    Triangle polygon;
    Vector normals[3];  // нормали к каждой вершине

    const Vector* GetNormal(size_t index) const {
        if (index < 3) {
            return &normals[index];
        }
        return nullptr;
    }
};

struct SphereObject {
    const Material* material = nullptr;
    Sphere sphere;
};
