#ifndef RASTERIZER_SKYBOX_H
#define RASTERIZER_SKYBOX_H

#include "texture.hpp"
#include <Eigen/Eigen>
#include <string>
#include <optional>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Skybox {
public:
	Skybox();
	
	// Load skybox from a single equirectangular image
	bool loadFromFile(const string& filename);
	
	// Get sky color for a given direction (normalized direction vector)
	Vec3 getColor(const Vec3& direction) const;
	
	// Check if skybox is loaded
	bool isLoaded() const { return texture.has_value(); }
	
private:
	optional<Texture> texture;
	
	// Convert direction vector to equirectangular UV coordinates
	Vec2 directionToUV(const Vec3& dir) const;
};

#endif

