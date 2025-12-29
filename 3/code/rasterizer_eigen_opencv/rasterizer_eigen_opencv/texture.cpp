#include "texture.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>
#include <algorithm>

Texture::Texture(const string& filename) {
    image_data = cv::imread(filename);
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    width = image_data.cols;
    height = image_data.rows;
}

Vec3 Texture::getColor(float u, float v) const {
    // Clamp u and v to valid range [0, 1]
    u = std::clamp(u, 0.0f, 1.0f);
    v = std::clamp(v, 0.0f, 1.0f);
    
    auto u_img = (int)(u * (width - 1));
    auto v_img = (int)((1.0f - v) * (height - 1));
    
    // Ensure indices are within bounds
    u_img = std::clamp(u_img, 0, width - 1);
    v_img = std::clamp(v_img, 0, height - 1);
    
    auto color = image_data.at<cv::Vec3b>(v_img, u_img);
    return Vec3(color[0], color[1], color[2]);
}

int Texture::w() const {
    return width;
}

int Texture::h() const {
    return height;
}