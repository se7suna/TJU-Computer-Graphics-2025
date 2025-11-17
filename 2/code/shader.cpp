#include "shader.hpp"
#include "texture.hpp"
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

Shader::Shader() {
	ks = Vec3(0.7937, 0.7937, 0.7937);
	kd = Vec3(1.0, 1.0, 1.0);
	ka = Vec3(0.005, 0.005, 0.005);
	ia = Vec3(10, 10, 10);
	kh = 0.2f;
	kn = 0.1f;
	eye_pos = Vec3(0, 0, 10);
}

Vec3 Shader::vertexShader(const VertexPayload& vertex_payload) {
	return vertex_payload.pos;
}

Vec3 Shader::phongShader(const FragmentPayload& fragment_payload, const vector<Light>& lights) {
	Vec3 result_color(0, 0, 0);
	
	kd = fragment_payload.color;
	Vec3 view_dir = (eye_pos - fragment_payload.pos).normalized();

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir = (light.pos - fragment_payload.pos).normalized();
		Vec3 half_dir = (view_dir + light_dir).normalized();
		float r_square = (light.pos - fragment_payload.pos).squaredNorm();

		for (size_t j = 0; j < 3; j++) {
			float i = light.intensity[j] / r_square;
			ls[j] = ks[j] * i * pow(max(0.0f, fragment_payload.normal.dot(half_dir)), 150.0f);
			ld[j] = kd[j] * i * max(0.0f, fragment_payload.normal.dot(light_dir));
			la[j] = ka[j] * ia[j];
		}

		result_color += ls;
		result_color += ld;
		result_color += la;
	}

	return result_color;
}

Vec3 Shader::textureShader(const FragmentPayload& fragment_payload, const vector<Light>& lights) {
	Vec3 result_color(0, 0, 0);

	float u = clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
	float v = clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
	
	kd = fragment_payload.texture->getColor(u, v) / 255.f;
	Vec3 view_dir = (eye_pos - fragment_payload.pos).normalized();

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir = (light.pos - fragment_payload.pos).normalized();
		Vec3 half_dir = (view_dir + light_dir).normalized();
		float r_square = (light.pos - fragment_payload.pos).squaredNorm();

		for (size_t j = 0; j < 3; j++) {
			float i = light.intensity[j] / r_square;
			ls[j] = ks[j] * i * pow(max(0.0f, fragment_payload.normal.dot(half_dir)), 150.0f);
			ld[j] = kd[j] * i * max(0.0f, fragment_payload.normal.dot(light_dir));
			la[j] = ka[j] * ia[j];
		}

		result_color += ls;
		result_color += ld;
		result_color += la;
	}

	return result_color;
}

Vec3 Shader::normalShader(const FragmentPayload& fragment_payload, const vector<Light>& lights) {
	// normal
	Vec3 normal = fragment_payload.normal;
	float x = normal.x();
	float y = normal.y();
	float z = normal.z();
	
	Vec3 tangent(x * y / sqrt(x * x + z * z), sqrt(x * x + z * z), z * y / sqrt(x * x + z * z));
	Vec3 bitangent = normal.cross(tangent);
	Mat3 tbn;
	tbn.col(0) = tangent.normalized();
	tbn.col(1) = bitangent.normalized();
	tbn.col(2) = normal;

	int w = fragment_payload.texture->w();
	int h = fragment_payload.texture->h();
	float u = clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
	float v = clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
	float u_next = min(u + 1.0f / w, 1.0f);
	float v_next = min(v + 1.0f / h, 1.0f);
	float d_u = kh * kn * (fragment_payload.texture->getColor(u_next, v).norm() - fragment_payload.texture->getColor(u, v).norm());
	float d_v = kh * kn * (fragment_payload.texture->getColor(u, v_next).norm() - fragment_payload.texture->getColor(u, v).norm());

	Vec3 new_tangent(-d_u, -d_v, 1.0);
	Vec3 new_normal = (tbn * new_tangent).normalized();
	Vec3 new_pos = fragment_payload.pos + new_normal * fragment_payload.texture->getColor(u, v).norm() * kn;
	

	// phong
	Vec3 result_color(0, 0, 0);

	kd = fragment_payload.color;
	Vec3 view_dir = (eye_pos - new_pos).normalized();

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir = (light.pos - new_pos).normalized();
		Vec3 half_dir = (view_dir + light_dir).normalized();
		float r_square = (light.pos - new_pos).squaredNorm();

		for (size_t j = 0; j < 3; j++) {
			float i = light.intensity[j] / r_square;
			ls[j] = ks[j] * i * pow(max(0.0f, new_normal.dot(half_dir)), 150.0f);
			ld[j] = kd[j] * i * max(0.0f, new_normal.dot(light_dir));
			la[j] = ka[j] * ia[j];
		}

		result_color += ls;
		result_color += ld;
		result_color += la;
	}

	return result_color;
}
