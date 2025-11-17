#ifndef RASTERIZER_TEXTURE_H
#define RASTERIZER_TEXTURE_H

#include <Eigen/Eigen>
#include <opencv2/opencv.hpp>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Texture {
public:
    Texture(const string& filename);
    
    int w();
    int h();

    Vec3 getColor(float u, float v);

private:
    cv::Mat image_data;
    int width, height;
};

#endif