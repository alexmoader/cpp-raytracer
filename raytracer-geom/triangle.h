#pragma once

#include "vector.h"

#include <cstddef>

class Triangle {
public:
    Triangle(const Vector& a, const Vector& b, const Vector& c) : vertices_{a, b, c} {
    }

    const Vector& operator[](size_t ind) const {
        return vertices_[ind];
    }

    double Area() const {
        Vector ab(vertices_[1][0] - vertices_[0][0], vertices_[1][1] - vertices_[0][1],
                  vertices_[1][2] - vertices_[0][2]);

        Vector ac(vertices_[2][0] - vertices_[0][0], vertices_[2][1] - vertices_[0][1],
                  vertices_[2][2] - vertices_[0][2]);
        return 0.5 * Length(CrossProduct(ab, ac));
    }

private:
    Vector vertices_[3];
};
