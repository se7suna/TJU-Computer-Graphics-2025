#ifndef RASTERIZER_MATERIAL_H
#define RASTERIZER_MATERIAL_H

#include <Eigen/Eigen>
#include <iostream>
#include "texture.hpp"
#include <optional>

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

// PBR Material for physically-based rendering
class PBRMaterial {
public:
	PBRMaterial();
	
	// Texture maps (optional)
	optional<Texture> albedo_map;      // Base color / Albedo
	optional<Texture> normal_map;      // Normal map
	optional<Texture> metallic_map;    // Metallic map
	optional<Texture> roughness_map;   // Roughness map
	optional<Texture> ao_map;          // Ambient occlusion map
	
	// Base values (used when maps are not available)
	Vec3 albedo;      // Base color (default: white)
	float metallic;   // Metallic factor (0.0 - 1.0)
	float roughness;  // Roughness factor (0.0 - 1.0)
	
	// Check if texture maps are available
	bool hasAlbedoMap() const { return albedo_map.has_value(); }
	bool hasNormalMap() const { return normal_map.has_value(); }
	bool hasMetallicMap() const { return metallic_map.has_value(); }
	bool hasRoughnessMap() const { return roughness_map.has_value(); }
	bool hasAOMap() const { return ao_map.has_value(); }
};

extern map<string, Material> all_materials;
extern map<string, PBRMaterial> all_pbr_materials;

void loadMaterials(const string& filename);
PBRMaterial loadPBRMaterial(const string& materialPath);

#endif
