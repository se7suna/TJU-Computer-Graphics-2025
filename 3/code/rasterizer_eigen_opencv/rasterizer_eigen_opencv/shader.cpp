#include "shader.hpp"
#include "texture.hpp"
#include "skybox.hpp"
#include <cmath>
#include <algorithm>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

const double PI = 3.14159265358979323846;

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
	Vec3 view_dir_vec = eye_pos - fragment_payload.pos;
	float view_dir_len = view_dir_vec.norm();
	Vec3 view_dir = (view_dir_len > 1e-6f) ? (view_dir_vec / view_dir_len) : Vec3(0, 0, 1);

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir_vec = light.pos - fragment_payload.pos;
		float light_dir_len = light_dir_vec.norm();
		Vec3 light_dir = (light_dir_len > 1e-6f) ? (light_dir_vec / light_dir_len) : Vec3(0, 0, 1);
		Vec3 half_dir_vec = view_dir + light_dir;
		float half_dir_len = half_dir_vec.norm();
		Vec3 half_dir = (half_dir_len > 1e-6f) ? (half_dir_vec / half_dir_len) : Vec3(0, 0, 1);
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

	float u = std::clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
	float v = std::clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
	
	kd = fragment_payload.texture->getColor(u, v) * (1.0f / 255.0f);
	Vec3 view_dir_vec = eye_pos - fragment_payload.pos;
	float view_dir_len = view_dir_vec.norm();
	Vec3 view_dir = (view_dir_len > 1e-6f) ? (view_dir_vec / view_dir_len) : Vec3(0, 0, 1);

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir_vec = light.pos - fragment_payload.pos;
		float light_dir_len = light_dir_vec.norm();
		Vec3 light_dir = (light_dir_len > 1e-6f) ? (light_dir_vec / light_dir_len) : Vec3(0, 0, 1);
		Vec3 half_dir_vec = view_dir + light_dir;
		float half_dir_len = half_dir_vec.norm();
		Vec3 half_dir = (half_dir_len > 1e-6f) ? (half_dir_vec / half_dir_len) : Vec3(0, 0, 1);
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
	float tangent_len = tangent.norm();
	tbn.col(0) = (tangent_len > 1e-6f) ? (tangent / tangent_len) : Vec3(1, 0, 0);
	float bitangent_len = bitangent.norm();
	tbn.col(1) = (bitangent_len > 1e-6f) ? (bitangent / bitangent_len) : Vec3(0, 1, 0);
	tbn.col(2) = normal;

	int w = fragment_payload.texture->w();
	int h = fragment_payload.texture->h();
	float u = std::clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
	float v = std::clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
	float u_next = min(u + 1.0f / w, 1.0f);
	float v_next = min(v + 1.0f / h, 1.0f);
	float d_u = kh * kn * (fragment_payload.texture->getColor(u_next, v).norm() - fragment_payload.texture->getColor(u, v).norm());
	float d_v = kh * kn * (fragment_payload.texture->getColor(u, v_next).norm() - fragment_payload.texture->getColor(u, v).norm());

	Vec3 new_tangent(-d_u, -d_v, 1.0);
	Vec3 new_normal_vec = tbn * new_tangent;
	float new_normal_len = new_normal_vec.norm();
	Vec3 new_normal = (new_normal_len > 1e-6f) ? (new_normal_vec / new_normal_len) : normal;
	Vec3 new_pos = fragment_payload.pos + new_normal * fragment_payload.texture->getColor(u, v).norm() * kn;
	

	// phong
	Vec3 result_color(0, 0, 0);

	kd = fragment_payload.color;
	Vec3 view_dir_vec = eye_pos - new_pos;
	float view_dir_len = view_dir_vec.norm();
	Vec3 view_dir = (view_dir_len > 1e-6f) ? (view_dir_vec / view_dir_len) : Vec3(0, 0, 1);

	for (auto light : lights) {
		Vec3 ls(0, 0, 0); // specular highlight
		Vec3 ld(0, 0, 0); // diffuse reflection
		Vec3 la(0, 0, 0); // ambient light

		Vec3 light_dir_vec = light.pos - new_pos;
		float light_dir_len = light_dir_vec.norm();
		Vec3 light_dir = (light_dir_len > 1e-6f) ? (light_dir_vec / light_dir_len) : Vec3(0, 0, 1);
		Vec3 half_dir_vec = view_dir + light_dir;
		float half_dir_len = half_dir_vec.norm();
		Vec3 half_dir = (half_dir_len > 1e-6f) ? (half_dir_vec / half_dir_len) : Vec3(0, 0, 1);
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

float Shader::distributionGGX(const Vec3& normal, const Vec3& half_dir, float roughness) {
	float a = roughness * roughness;
	float a_squared = a * a;
	float n_dot_h = max(normal.dot(half_dir), 0.0f);
	float n_dot_h_squared = n_dot_h * n_dot_h;
	
	float num = a_squared;
	float denom = (n_dot_h_squared * (a_squared - 1.0f) + 1.0f);
	denom = PI * denom * denom;
	
	return num / max(denom, 0.0000001f);
}

float Shader::geometrySchlickGGX(float n_dot_v, float roughness) {
	float r = (roughness + 1.0f);
	float k = (r * r) / 8.0f;
	
	float num = n_dot_v;
	float denom = n_dot_v * (1.0f - k) + k;
	
	return num / max(denom, 0.0000001f);
}

float Shader::geometrySmith(const Vec3& normal, const Vec3& view_dir, const Vec3& light_dir, float roughness) {
	float n_dot_v = max(normal.dot(view_dir), 0.0f);
	float n_dot_l = max(normal.dot(light_dir), 0.0f);
	float ggx_2 = geometrySchlickGGX(n_dot_v, roughness);
	float ggx_1 = geometrySchlickGGX(n_dot_l, roughness);
	
	return ggx_1 * ggx_2;
}

Vec3 Shader::fresnelSchlick(float cos_theta, const Vec3& fresnel) {
	return fresnel + (Vec3(1.0f, 1.0f, 1.0f) - fresnel) * pow(std::clamp(1.0f - cos_theta, 0.0f, 1.0f), 5.0f);
}

Vec3 Shader::pbrShader(const FragmentPayload& fragment_payload, const vector<Light>& lights, const Skybox* skybox) {
	Vec3 albedo = fragment_payload.color;
	float metallic = 0.0f;
	float roughness = 0.5f;
	float normal_len = fragment_payload.normal.norm();
	Vec3 normal = (normal_len > 1e-6f) ? (fragment_payload.normal / normal_len) : Vec3(0, 0, 1);
	
	if (fragment_payload.pbr_material) {
		float u = std::clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
		float v = std::clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
		
		if (fragment_payload.pbr_material->hasAlbedoMap())
			albedo = fragment_payload.pbr_material->albedo_map->getColor(u, v) * (1.0f / 255.0f);
		else
			albedo = fragment_payload.pbr_material->albedo;
		
		if (fragment_payload.pbr_material->hasMetallicMap()) {
			Vec3 metallic_color = fragment_payload.pbr_material->metallic_map->getColor(u, v) * (1.0f / 255.0f);
			metallic = (metallic_color.x() + metallic_color.y() + metallic_color.z()) / 3.0f;
		}
		else
			metallic = fragment_payload.pbr_material->metallic;
		
		if (fragment_payload.pbr_material->hasRoughnessMap()) {
			Vec3 roughness_color = fragment_payload.pbr_material->roughness_map->getColor(u, v) * (1.0f / 255.0f);
			roughness = (roughness_color.x() + roughness_color.y() + roughness_color.z()) / 3.0f;
		} 
		else
			roughness = fragment_payload.pbr_material->roughness;
		
		if (fragment_payload.pbr_material->hasNormalMap()) {
			Vec3 normal_color = fragment_payload.pbr_material->normal_map->getColor(u, v) * (1.0f / 255.0f);
			Vec3 tangent_normal = normal_color * 2.0f - Vec3(1.0f, 1.0f, 1.0f);
			Vec3 new_normal_vec = normal + tangent_normal * 0.5f;
			float new_normal_len = new_normal_vec.norm();
			normal = (new_normal_len > 1e-6f) ? (new_normal_vec / new_normal_len) : normal;
		}
	}
	
	Vec3 view_dir_vec = eye_pos - fragment_payload.pos;
	float view_dir_len = view_dir_vec.norm();
	Vec3 view_dir = (view_dir_len > 1e-6f) ? (view_dir_vec / view_dir_len) : Vec3(0, 0, 1);
	
	Vec3 fresnel = Vec3(0.04f, 0.04f, 0.04f);
	fresnel = fresnel * (1.0f - metallic) + albedo * metallic;
	
	Vec3 result_color(0, 0, 0);
	
	for (const auto& light : lights) {
		Vec3 light_dir_vec = light.pos - fragment_payload.pos;
		float light_dir_len = light_dir_vec.norm();
		Vec3 light_dir = (light_dir_len > 1e-6f) ? (light_dir_vec / light_dir_len) : Vec3(0, 0, 1);
		Vec3 half_dir_vec = view_dir + light_dir;
		float half_dir_len = half_dir_vec.norm();
		Vec3 half_dir = (half_dir_len > 1e-6f) ? (half_dir_vec / half_dir_len) : Vec3(0, 0, 1);
		float distance = (light.pos - fragment_payload.pos).norm();
		float attenuation = 1.0f / (distance * distance);
		Vec3 radiance = light.intensity * attenuation;
		
		// Cook-Torrance BRDF
		float ndf = distributionGGX(normal, half_dir, roughness);
		float geometry = geometrySmith(normal, view_dir, light_dir, roughness);
		Vec3 fresnel_factor = fresnelSchlick(max(half_dir.dot(view_dir), 0.0f), fresnel);
		
		Vec3 k_s = fresnel_factor;
		Vec3 k_d = Vec3(1.0f, 1.0f, 1.0f) - k_s;
		k_d *= 1.0f - metallic;
		
		Vec3 numerator = ndf * geometry * fresnel_factor;
		float denominator = 4.0f * max(normal.dot(view_dir), 0.0f) * max(normal.dot(light_dir), 0.0f) + 0.0001f;
		Vec3 specular = numerator / denominator;
		
		float n_dot_l = max(normal.dot(light_dir), 0.0f);
		result_color += (k_d.cwiseProduct(albedo) / PI + specular).cwiseProduct(radiance) * n_dot_l;
	}
	
	Vec3 ambient;
	if (skybox && skybox->isLoaded()) {

		Vec3 ambient_dir = normal;
		Vec3 skybox_ambient = skybox->getColor(ambient_dir);
		ambient = skybox_ambient.cwiseProduct(albedo) * 1.0f; 
	} else {
		ambient = Vec3(0.03f, 0.03f, 0.03f).cwiseProduct(albedo);
	}
	
	if (fragment_payload.pbr_material && fragment_payload.pbr_material->hasAOMap()) {
		float u = std::clamp(fragment_payload.text_coord.x(), 0.0f, 1.0f);
		float v = std::clamp(fragment_payload.text_coord.y(), 0.0f, 1.0f);
		Vec3 ao_color = fragment_payload.pbr_material->ao_map->getColor(u, v) * (1.0f / 255.0f);
		float ao = (ao_color.x() + ao_color.y() + ao_color.z()) / 3.0f;
		ambient *= ao;
	}
	
	Vec3 color = ambient + result_color;
	color = color.cwiseQuotient(color + Vec3(1.0f, 1.0f, 1.0f));
	color = Vec3(pow(color.x(), 1.0f/2.2f), pow(color.y(), 1.0f/2.2f), pow(color.z(), 1.0f/2.2f));
	
	return color;
}
