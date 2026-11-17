#pragma once

#include <array>
#include <cstddef>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <vector>

struct RGB {
    int r, g, b;
};

class Image {
public:
    Image(int width, int height)
        : width_(width), height_(height), pixels_(static_cast<std::size_t>(width) * height) {
    }

    void SetPixel(const RGB& pixel, int y, int x) {
        pixels_[static_cast<std::size_t>(y) * width_ + x] = {
            static_cast<unsigned char>(pixel.r),
            static_cast<unsigned char>(pixel.g),
            static_cast<unsigned char>(pixel.b),
        };
    }

    void Write(const std::filesystem::path& output) const {
        std::ofstream file(output, std::ios::binary);
        file << "P6\n" << width_ << ' ' << height_ << "\n255\n";
        for (const auto& pixel : pixels_) {
            file.write(reinterpret_cast<const char*>(pixel.data()), pixel.size());
        }
        if (!file) {
            throw std::runtime_error("Cannot write image: " + output.string());
        }
    }

private:
    int width_;
    int height_;
    std::vector<std::array<unsigned char, 3>> pixels_;
};
