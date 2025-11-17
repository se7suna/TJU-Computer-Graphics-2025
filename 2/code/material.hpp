#ifndef RASTERIZER_MATERIAL_H
#define RASTERIZER_MATERIAL_H

#include <Eigen/Eigen>
#include <iostream>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Material {
public:
	Material();

	Vec3 Ks, Kd, Ka;
	float Ns, Ni;
	float d;
	float illum;
};

map<string, Material> all_materials;

void loadMaterials(const string& filename);

#endif
