#include "texture.hpp"
#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>

Texture::Texture(const string& filename) {
    image_data = cv::imread(filename);
    cv::cvtColor(image_data, image_data, cv::COLOR_RGB2BGR);
    width = image_data.cols;
    height = image_data.rows;
}

Vec3 Texture::getColor(float u, float v) {
    auto u_img = u * width;
    auto v_img = (1 - v) * height;
    auto color = image_data.at<cv::Vec3b>(v_img, u_img);
    return Vec3(color[0], color[1], color[2]);
}

int Texture::w() {
    return width;
}

int Texture::h() {
    return height;
}