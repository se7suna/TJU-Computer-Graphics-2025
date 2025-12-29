#pragma once

#include <Eigen/Eigen>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

double Deg2Rad(double deg); // degree to radian

Mat4 view(const Vec3& pos, const Vec3& center, const Vec3& up); // camera / view transformation
Mat4 perspective(float eye_fov, float aspect_ratio, float z_near, float z_far); // perspective / projection transformation
Mat4 model(const Vec3& angles, const Vec3& axis); // model transformation

void barycentric(float x, float y, const Vec2& a, const Vec2& b, const Vec2& c, float& alpha, float& beta, float& gamma); // calculate barycentric coordinates within triangle ABC
