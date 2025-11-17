#ifndef RASTERIZER_SHADER_H
#define RASTERIZER_SHADER_H

#include "texture.hpp"
#include <Eigen/Eigen>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Shader {
public:
	Shader(); // presets

	struct VertexPayload { // store vertex information
		Vec3 pos;
	};

	struct FragmentPayload { //store fragment information
		FragmentPayload() {
			texture = nullptr;
		}

		FragmentPayload(const Vec3& p, const Vec3& c, const Vec2& t_c, const Vec3& n, Texture* t) :
			pos(p), normal(n), color(c), text_coord(t_c), texture(t) {}
	
		Vec3 pos; //view position
		Vec3 color;
		Vec2 text_coord;
		Vec3 normal;
		Texture* texture;
	};

	struct Light {
		Light(const Vec3& p, const Vec3& i) :
			pos(p), intensity(i) {}

		Vec3 pos;
		Vec3 intensity;
	};

	Vec3 vertexShader(const VertexPayload& vertex_payload);
	Vec3 phongShader(const FragmentPayload& fragment_payload, const vector<Light>& lights);   // blinn-phong lighting
	Vec3 textureShader(const FragmentPayload& fragment_payload, const vector<Light>& lights); // texture mapping
	Vec3 normalShader(const FragmentPayload& fragment_payload, const vector<Light>& lights);  // normal mapping

private:
	Vec3 ks, kd, ka;
	Vec3 ia; // ambient intensity
	float kh, kn;
	Vec3 eye_pos;
};

#endif
