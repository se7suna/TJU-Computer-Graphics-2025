#include "geometry.hpp"
#include <Eigen/Eigen>

const double PI = 3.1415;
double Deg2Rad(double deg) {
    return deg * PI / 180;
}

Mat4 view(const Vec3& pos, const Vec3& center, const Vec3& up) {
	Vec3 dir = (center - pos).normalized();
    Vec3 u = dir.cross(up).normalized();
    Vec3 v = u.cross(dir);

    Mat4 rotation;
    rotation <<
        u.x(), u.y(), u.z(), 0,
        v.x(), v.y(), v.z(), 0,
        -dir.x(), -dir.y(), -dir.z(), 0,
        0, 0, 0, 1;

    Mat4 translation;
    translation <<
        1, 0, 0, -pos.x(),
        0, 1, 0, -pos.y(),
        0, 0, 1, -pos.z(),
        0, 0, 0, 1;

    return rotation * translation;
}

Mat4 perspective(float eye_fov, float aspect_ratio, float z_near, float z_far) {
    float fov = Deg2Rad(eye_fov);
    float f = 1.0 / tan(fov * 0.5);
    Mat4 scale;
    scale <<
        f / aspect_ratio, 0, 0, 0,
        0, f, 0, 0,
        0, 0, (z_far + z_near) / (z_near - z_far), (2.0 * z_far * z_near) / (z_near - z_far),
        0, 0, -1, 0;

    return scale;
}

Mat4 model(const Vec3& angles, const Vec3& axis) {
    float  angle_x = Deg2Rad(angles[0]);
    float angle_y = Deg2Rad(angles[1]);
    float angle_z = Deg2Rad(angles[2]);

    Mat4 rotation_x;
    rotation_x <<
        1, 0, 0, 0,
        0, cos(angle_x), -sin(angle_x), 0,
        0, sin(angle_x), cos(angle_x), 0,
        0, 0, 0, 1;

    Mat4 rotation_y;
    rotation_y <<
        cos(angle_y), 0, sin(angle_y), 0,
        0, 1, 0, 0,
        -sin(angle_y), 0, cos(angle_y), 0,
        0, 0, 0, 1;

    Mat4 rotation_z;
    rotation_z <<
        cos(angle_z), -sin(angle_z), 0, 0,
        sin(angle_z), cos(angle_z), 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1;

    Mat4 rotation = rotation_z * rotation_y * rotation_x;

    Mat4 scale;
    scale <<
        2.5, 0, 0, 0,
        0, 2.5, 0, 0,
        0, 0, 2.5, 0,
        0, 0, 0, 1;
    
    Mat4 translate_to_origin;
    translate_to_origin <<
        1, 0, 0, -axis.x(),
        0, 1, 0, -axis.y(),
        0, 0, 1, -axis.z(),
        0, 0, 0, 1;

    Mat4 translate_back;
    translate_back <<
        1, 0, 0, axis.x(),
        0, 1, 0, axis.y(),
        0, 0, 1, axis.z(),
        0, 0, 0, 1;

    return translate_back * rotation * translate_to_origin * scale;
}


void barycentric(float x, float y, const Vec2& a, const Vec2& b, const Vec2& c, float& alpha, float& beta, float& gamma) {
    float demon = (b.y() - c.y()) * (a.x() - c.x()) + (c.x() - b.x()) * (a.y() - c.y());
    if (fabs(demon) < 1e-8) {
        alpha = beta = gamma = -1.0;
        return;
    }

    alpha = ((b.y() - c.y()) * (x - c.x()) + (c.x() - b.x()) * (y - c.y())) / demon;
    beta = ((c.y() - a.y()) * (x - c.x()) + (a.x() - c.x()) * (y - c.y())) / demon;
    gamma = 1.0 - alpha - beta;
}
