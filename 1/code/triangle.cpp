#include "triangle.hpp"
#include <Eigen/Eigen>

Triangle::Triangle()
{
    vertex[0] << 0.0, 0.0, 0.0;
    vertex[1] << 0.0, 0.0, 0.0;
    vertex[2] << 0.0, 0.0, 0.0;

    color[0] << 0.0, 0.0, 0.0;
    color[1] << 0.0, 0.0, 0.0;
    color[2] << 0.0, 0.0, 0.0;

    text_coord[0] << 0.0, 0.0;
    text_coord[1] << 0.0, 0.0;
    text_coord[2] << 0.0, 0.0;
}

Vec3 Triangle::a() const {
    return vertex[0];
}

Vec3 Triangle::b() const {
    return vertex[1];
}

Vec3 Triangle::c() const {
    return vertex[2];
}

void Triangle::setVertex(int i, Vec3 v) {
    vertex[i] = v;
}

void Triangle::setColor(int i, Vec3 c) {
    color[i] = c;
}

void Triangle::setTextCoord(int i, Vec2 t) {
    text_coord[i] = t;
}

void Triangle::setNormal(int i, Vec3 n) {
    normal[i] = n;
}