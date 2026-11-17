#pragma once

#include "material.h"
#include "vector.h"
#include "object.h"
#include "light.h"

#include <vector>
#include <unordered_map>
#include <string>
#include <filesystem>
#include <iostream>
#include <fstream>
#include <sstream>

class Scene {
public:
    const std::vector<Object>& GetObjects() const {
        return objects_;
    }
    const std::vector<SphereObject>& GetSphereObjects() const {
        return sphere_objects_;
    }
    const std::vector<Light>& GetLights() const {
        return lights_;
    }

    const std::unordered_map<std::string, Material>& GetMaterials() const {
        return materials_;
    }
    friend Scene ReadScene(const std::filesystem::path& path);

private:
    std::vector<Object> objects_;
    std::vector<SphereObject> sphere_objects_;
    std::vector<Light> lights_;
    std::unordered_map<std::string, Material> materials_;
};

std::unordered_map<std::string, Material> ReadMaterials(const std::filesystem::path& path) {
    std::unordered_map<std::string, Material> materials;
    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Could not open mtl file: " + path.string());
    }

    Material current;
    // значения по умолчанию
    current.albedo = Vector(1, 0, 0);
    current.specular_exponent = 1.0;
    current.refraction_index = 1.0;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "newmtl") {
            if (!current.name.empty()) {
                materials[current.name] = current;
                current = Material();
                // значения по умолчанию
                current.albedo = Vector(1, 0, 0);
                current.specular_exponent = 1.0;
                current.refraction_index = 1.0;
            }
            iss >> current.name;

        } else if (token == "Ka") {
            double r, g, b;
            iss >> r >> g >> b;
            current.ambient_color = Vector(r, g, b);

        } else if (token == "Kd") {
            double r, g, b;
            iss >> r >> g >> b;
            current.diffuse_color = Vector(r, g, b);

        } else if (token == "Ks") {
            double r, g, b;
            iss >> r >> g >> b;
            current.specular_color = Vector(r, g, b);

        } else if (token == "Ke") {
            double r, g, b;
            iss >> r >> g >> b;
            current.intensity = Vector(r, g, b);

        } else if (token == "Ns") {
            iss >> current.specular_exponent;

        } else if (token == "Ni") {
            iss >> current.refraction_index;

        } else if (token == "al") {
            double a, b, c;
            iss >> a >> b >> c;
            current.albedo = Vector(a, b, c);
        }
    }

    // последний материал
    if (!current.name.empty()) {
        materials[current.name] = current;
    }

    return materials;
}

Scene ReadScene(const std::filesystem::path& path) {
    Scene scene;

    std::ifstream file(path);

    if (!file) {
        throw std::runtime_error("Could not open scene file: " + path.string());
    }

    std::vector<Vector> vertices;
    std::vector<Vector> normals;
    const Material* current_material = nullptr;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        std::istringstream iss(line);
        std::string token;
        iss >> token;

        if (token == "mtllib") {
            std::string mtl_filename;
            iss >> mtl_filename;
            scene.materials_ = ReadMaterials(path.parent_path() / mtl_filename);
        }

        else if (token == "v") {
            Vector v;
            iss >> v[0] >> v[1] >> v[2];
            vertices.push_back(v);
        }

        else if (token == "vn") {
            Vector n;
            iss >> n[0] >> n[1] >> n[2];
            normals.push_back(n);
        } else if (token == "usemtl") {
            std::string material_name;
            iss >> material_name;
            auto it = scene.materials_.find(material_name);
            if (it != scene.materials_.end()) {
                current_material = &it->second;
            } else {
                current_material = nullptr;
            }
        }

        else if (token == "S") {
            double x, y, z, r;
            iss >> x >> y >> z >> r;
            scene.sphere_objects_.push_back({current_material, Sphere(Vector(x, y, z), r)});
        }

        else if (token == "P") {
            double x, y, z, r, g, b;
            iss >> x >> y >> z >> r >> g >> b;
            scene.lights_.push_back(Light{Vector(x, y, z), Vector(r, g, b)});
        }

        else if (token == "f") {
            std::vector<std::string> face_tokens;
            std::string face_token;
            while (iss >> face_token) {
                face_tokens.push_back(face_token);
            }

            if (face_tokens.size() < 3) {
                continue;
            }

            std::vector<int> vertex_indices;
            std::vector<int> normal_indices;
            bool has_normals = false;

            for (const auto& token : face_tokens) {
                size_t first = token.find('/');
                int vi;
                if (first == std::string::npos) {
                    vi = std::stoi(token);
                    if (vi < 0) {
                        vi += vertices.size() + 1;
                    }
                    vertex_indices.push_back(vi - 1);
                }

                else {
                    size_t second = token.find('/', first + 1);
                    vi = std::stoi(token.substr(0, first));
                    if (vi < 0) {
                        vi += vertices.size() + 1;
                    }
                    vertex_indices.push_back(vi - 1);
                    if (second != std::string::npos) {
                        int ni = std::stoi(token.substr(second + 1));
                        if (ni < 0) {
                            ni += normals.size() + 1;
                        }
                        normal_indices.push_back(ni - 1);
                        has_normals = true;
                    }
                }
            }

            if (has_normals && normal_indices.size() != vertex_indices.size()) {
                has_normals = false;
            }

            for (size_t i = 1; i + 1 < vertex_indices.size(); ++i) {
                Vector n0, n1, n2;
                if (has_normals && !normal_indices.empty()) {
                    n0 = normals[normal_indices[0]];
                    n1 = normals[normal_indices[i]];
                    n2 = normals[normal_indices[i + 1]];
                }

                scene.objects_.push_back(
                    {current_material,
                     Triangle(vertices[vertex_indices[0]], vertices[vertex_indices[i]],
                              vertices[vertex_indices[i + 1]]),
                     {n0, n1, n2}});
            }
        }
    }

    return scene;
}
