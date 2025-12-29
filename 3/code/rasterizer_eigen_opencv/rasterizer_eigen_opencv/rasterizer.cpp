#include "rasterizer.hpp"
#include "geometry.hpp"
#include <algorithm>
#include <vector>
#include <limits>
#include <cmath>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

Rasterizer::Rasterizer(int w, int h) : width(w), height(h), pbr_material(nullptr) {
	pixel_buffer = cv::Mat(h, w, CV_8UC3, cv::Scalar(0, 0, 0));
	depth_buffer.resize(w * h, numeric_limits<float>::infinity());
}

void Rasterizer::clear() {
	pixel_buffer = cv::Mat(height, width, CV_8UC3, cv::Scalar(0, 0, 0));
	depth_buffer.clear();
	depth_buffer.resize(width * height, numeric_limits<float>::infinity());
}

void Rasterizer::setModel(const Mat4& m) {
	model = m;
}

void Rasterizer::setView(const Mat4& v) {
	view = v;
	view_inv = v.inverse();
}

void Rasterizer::setProjection(const Mat4& p) {
	projection = p;
}

void Rasterizer::setVertexShader(function<Vec3(const Shader::VertexPayload&)> v_s) {
	vertex_shader = v_s;
}

void Rasterizer::setFragmentShader(function<Vec3(const Shader::FragmentPayload&, const vector<Shader::Light>&)> f_s) {
	fragment_shader = f_s;
}

void Rasterizer::setTexture(Texture t) {
	texture = t;
}

void Rasterizer::setSkybox(const Skybox& sb) {
	skybox = sb;
}

void Rasterizer::setPBRMaterial(PBRMaterial* material) {
	pbr_material = material;
}

cv::Mat Rasterizer::getPixels() const {
	return pixel_buffer;
}

void Rasterizer::drawTriangle(const Triangle& t) {
	Mat4 mvp = projection * view * model;
	Vec4 vec[] = {
		mvp * Vec4(t.a().x(), t.a().y(), t.a().z(), 1.0),
		mvp * Vec4(t.b().x(), t.b().y(), t.b().z(), 1.0),
		mvp * Vec4(t.c().x(), t.c().y(), t.c().z(), 1.0)
	};

	// Check if any vertex is behind camera or has invalid w before perspective division
	// If any vertex is behind camera (w <= 0), skip this triangle
	for (int i = 0; i < 3; i++) {
		if (vec[i].w() <= 0.0f || std::abs(vec[i].w()) < 1e-6f) {
			return;
		}
	}

	for (auto& v : vec)
		v /= v.w();

	// Convert to screen space
	Vec3 vec_screen[] = {
		Vec3((vec[0].x() + 1.0) * (float)width * 0.5, (1.0 - vec[0].y()) * (float)height * 0.5, (vec[0].z() + 1.0f) * 0.5),
		Vec3((vec[1].x() + 1.0) * (float)width * 0.5, (1.0 - vec[1].y()) * (float)height * 0.5, (vec[1].z() + 1.0f) * 0.5),
		Vec3((vec[2].x() + 1.0) * (float)width * 0.5, (1.0 - vec[2].y()) * (float)height * 0.5, (vec[2].z() + 1.0f) * 0.5)
	};
	int minx = floor(min({ vec_screen[0].x(), vec_screen[1].x(), vec_screen[2].x() }));
	int maxx = ceil(max({ vec_screen[0].x(), vec_screen[1].x(), vec_screen[2].x() }));
	int miny = floor(min({ vec_screen[0].y(), vec_screen[1].y(), vec_screen[2].y() }));
	int maxy = ceil(max({ vec_screen[0].y(), vec_screen[1].y(), vec_screen[2].y() }));

	// Check if triangle bounding box is completely outside screen bounds [0, width) x [0, height)
	// Only skip if the entire triangle is outside
	if (maxx < 0 || minx >= width || maxy < 0 || miny >= height) {
		return;
	}

	// Clamp to screen bounds for safe array access
	minx = std::max(0, minx);
	maxx = std::min(width - 1, maxx);
	miny = std::max(0, miny);
	maxy = std::min(height - 1, maxy);

	// Skip if triangle is completely outside screen after clamping (shouldn't happen, but be safe)
	if (minx > maxx || miny > maxy) {
		return;
	}

	// Use lights (default lights if fragment_shader is set)
	auto l1 = Shader::Light{ {-20, 20, -20}, {500, 500, 500} };
	auto l2 = Shader::Light{ {-20, 20, 0}, {500, 500, 500} };
	vector<Shader::Light> lights = { l1, l2 };

	// run rasterizer
	for (int y = miny; y <= maxy; y++) {
		for (int x = minx; x <= maxx; x++) {
			// Additional bounds check before accessing arrays
			if (x < 0 || x >= width || y < 0 || y >= height) {
				continue;
			}
			
			float px = x + 0.5;
			float py = y + 0.5;
			float alpha, beta, gamma;

			barycentric(px, py,
				Vec2(vec_screen[0].x(), vec_screen[0].y()),
				Vec2(vec_screen[1].x(), vec_screen[1].y()),
				Vec2(vec_screen[2].x(), vec_screen[2].y()),
				alpha, beta, gamma);
			if (alpha < 0 || beta < 0 || gamma < 0)
				continue;

			float z_interpolated = alpha * vec_screen[0].z() + beta * vec_screen[1].z() + gamma * vec_screen[2].z();
			int idx = (height - 1 - y) * width + x;
			if (idx < 0 || idx >= (int)depth_buffer.size()) {
				continue;
			}
			if (z_interpolated < depth_buffer[idx]) {
				depth_buffer[idx] = z_interpolated;

				Vec4 pos_vec4 = model * Vec4(alpha * t.vertex[0].x() + beta * t.vertex[1].x() + gamma * t.vertex[2].x(),
										alpha * t.vertex[0].y() + beta * t.vertex[1].y() + gamma * t.vertex[2].y(),
										alpha * t.vertex[0].z() + beta * t.vertex[1].z() + gamma * t.vertex[2].z(),
										1.0f);
				Vec3 pos = pos_vec4.head<3>();
				Vec3 color = alpha * t.color[0] + beta * t.color[1] + gamma * t.color[2];
				Vec3 normal = alpha * t.normal[0] + beta * t.normal[1] + gamma * t.normal[2];
				Vec3 transformed_normal = (model.inverse()).transpose().block<3, 3>(0, 0) * normal;
				float norm_len = transformed_normal.norm();
				if (norm_len > 1e-6f) {
					normal = transformed_normal / norm_len;
				} else {
					normal = Vec3(0, 0, 1); // Default to up vector if normal is invalid
				}
				Vec2 text_coord = alpha * t.text_coord[0] + beta * t.text_coord[1] + gamma * t.text_coord[2];
				
				Shader::FragmentPayload f_p(pos, color, text_coord, normal, texture ? &*texture : nullptr, pbr_material);
				if (!fragment_shader) {
					continue; // Skip if fragment shader not set
				}
				Vec3 shaded_color = fragment_shader(f_p, lights);
				
				// Final bounds check before pixel buffer access
				if (x >= 0 && x < width && y >= 0 && y < height) {
					pixel_buffer.at<cv::Vec3b>(y, x)[0] = (uchar)(shaded_color.z() * 255);
					pixel_buffer.at<cv::Vec3b>(y, x)[1] = (uchar)(shaded_color.y() * 255);
					pixel_buffer.at<cv::Vec3b>(y, x)[2] = (uchar)(shaded_color.x() * 255);
				}
			}
		}
	}

	return;
}

void Rasterizer::drawSkybox() {
	if (!skybox.has_value() || !skybox->isLoaded()) {
		return;
	}
	
	// Get camera position from view matrix (assuming view is look-at matrix)
	Vec3 cam_pos = Vec3(view_inv(0, 3), view_inv(1, 3), view_inv(2, 3));
	
	// Render skybox for each pixel
	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			// Only render skybox where depth buffer is infinity (background)
			// Bounds check for index calculation
			if (y < 0 || y >= height || x < 0 || x >= width) {
				continue;
			}
			int idx = (height - 1 - y) * width + x;
			if (idx < 0 || idx >= (int)depth_buffer.size()) {
				continue;
			}
			if (depth_buffer[idx] >= 1.0f) {
				// Calculate ray direction for this pixel
				float ndc_x = (x + 0.5f) / width * 2.0f - 1.0f;
				float ndc_y = 1.0f - (y + 0.5f) / height * 2.0f;
				float ndc_z = 1.0f; // Far plane
				
				// Transform to world space
				Mat4 proj_inv = projection.inverse();
				Vec4 clip_pos = Vec4(ndc_x, ndc_y, ndc_z, 1.0f);
				Vec4 view_pos = proj_inv * clip_pos;
				if (std::abs(view_pos.w()) < 1e-6f) {
					continue; // Skip if w is too small
				}
				view_pos /= view_pos.w();
				
				Vec4 world_pos = view_inv * view_pos;
				Vec3 direction_vec = world_pos.head<3>() - cam_pos;
				float dir_len = direction_vec.norm();
				Vec3 direction;
				if (dir_len > 1e-6f) {
					direction = direction_vec / dir_len;
				} else {
					direction = Vec3(0, 0, 1); // Default direction if invalid
				}
				
				// Sample skybox
				Vec3 sky_color = skybox->getColor(direction);
				
				// Final bounds check before pixel buffer access
				if (y >= 0 && y < height && x >= 0 && x < width) {
					pixel_buffer.at<cv::Vec3b>(y, x)[0] = (uchar)(std::clamp(sky_color.z() * 255.0f, 0.0f, 255.0f));
					pixel_buffer.at<cv::Vec3b>(y, x)[1] = (uchar)(std::clamp(sky_color.y() * 255.0f, 0.0f, 255.0f));
					pixel_buffer.at<cv::Vec3b>(y, x)[2] = (uchar)(std::clamp(sky_color.x() * 255.0f, 0.0f, 255.0f));
				}
			}
		}
	}
}
