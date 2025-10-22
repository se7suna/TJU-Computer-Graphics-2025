#ifndef RASTERIZER_TRIANGLE_H
#define RASTERIZER_TRIANGLE_H

#include <Eigen/Eigen>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Triangle {
public:
	Triangle();
	
	Vec3 vertex[3];     // original coordinates of three vertices, in counter clockwise order
	Vec3 color[3];      // color at each vertex
	Vec2 text_coord[3]; // texture coordinates (u, v)
	Vec3 normal[3];     //normal vector for each vertex

	Vec3 a() const;
	Vec3 b() const;
	Vec3 c() const;

	void setVertex(int i, Vec3 v);
	void setColor(int i, Vec3 c);
	void setTextCoord(int i, Vec2 t);
	void setNormal(int i, Vec3 n);
};

#endif
