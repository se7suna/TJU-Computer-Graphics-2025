#include "skybox.hpp"
#include <cmath>
#include <algorithm>

const double PI = 3.14159265358979323846;

using namespace std;

Skybox::Skybox() {
}

bool Skybox::loadFromFile(const string& filename) {
	try {
		texture = Texture(filename);
		return true;
	} catch (...) {
		return false;
	}
}

Vec2 Skybox::directionToUV(const Vec3& dir) const {
	// Normalize direction safely
	float dir_len = dir.norm();
	Vec3 normalized;
	if (dir_len > 1e-6f) {
		normalized = dir / dir_len;
	} else {
		normalized = Vec3(0, 0, 1); // Default direction if invalid
	}
	
	// Convert direction to spherical coordinates
	// u = atan2(z, x) / (2*PI) + 0.5
	// v = asin(y) / PI + 0.5
	
	float u = atan2(normalized.z(), normalized.x()) / (2.0f * PI) + 0.5f;
	float v = asin(std::clamp(normalized.y(), -1.0f, 1.0f)) / PI + 0.5f;
	
	return Vec2(u, v);
}

Vec3 Skybox::getColor(const Vec3& direction) const {
	if (!texture.has_value()) {
		return Vec3(0.0f, 0.0f, 0.0f); // Default to black if not loaded
	}
	
	Vec2 uv = directionToUV(direction);
	return texture->getColor(uv.x(), uv.y()) * (1.0f / 255.0f);
}

