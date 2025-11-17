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

Rasterizer::Rasterizer(int w, int h) : width(w), height(h) {
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
}

void Rasterizer::setProjection(const Mat4& p) {
	projection = p;
}

void Rasterizer::setVertexShader(function<Vec3(const Shader::VertexPayload&)> v_s) {
	vertexShader = v_s;
}

void Rasterizer::setFragmentShader(function<Vec3(const Shader::FragmentPayload&, const vector<Shader::Light>&)> f_s) {
	fragmentShader = f_s;
}

void Rasterizer::setTexture(Texture t) {
	texture = t;
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

	for (auto& v : vec)
		v /= v.w();

	Vec3 vec_screen[] = {
		Vec3((vec[0].x() + 1.0) * (float)width * 0.5, (1.0 - vec[0].y()) * (float)height * 0.5, (vec[0].z() + 1.0f) * 0.5),
		Vec3((vec[1].x() + 1.0) * (float)width * 0.5, (1.0 - vec[1].y()) * (float)height * 0.5, (vec[1].z() + 1.0f) * 0.5),
		Vec3((vec[2].x() + 1.0) * (float)width * 0.5, (1.0 - vec[2].y()) * (float)height * 0.5, (vec[2].z() + 1.0f) * 0.5)
	};
	int minx = floor(min({ vec_screen[0].x(), vec_screen[1].x(), vec_screen[2].x() }));
	int maxx = ceil(max({ vec_screen[0].x(), vec_screen[1].x(), vec_screen[2].x() }));
	int miny = floor(min({ vec_screen[0].y(), vec_screen[1].y(), vec_screen[2].y() }));
	int maxy = ceil(max({ vec_screen[0].y(), vec_screen[1].y(), vec_screen[2].y() }));

	// set shader
	Shader shader;
	setFragmentShader(
		[&shader](const Shader::FragmentPayload& fragment_payload, const std::vector<Shader::Light>& lights) {
			return shader.phongShader(fragment_payload, lights);
		}
	);

	auto l1 = Shader::Light{ {-20, 20, -20}, {500, 500, 500} };
	auto l2 = Shader::Light{ {-20, 20, 0}, {500, 500, 500} };
	vector<Shader::Light> lights = { l1, l2 };

	// run rasterizer
	for (int y = miny; y <= maxy; y++) {
		for (int x = minx; x <= maxx; x++) {
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
			if (z_interpolated < depth_buffer[idx]) {
				depth_buffer[idx] = z_interpolated;

				Vec4 pos_vec4 = model * Vec4(alpha * t.vertex[0].x() + beta * t.vertex[1].x() + gamma * t.vertex[2].x(),
										alpha * t.vertex[0].y() + beta * t.vertex[1].y() + gamma * t.vertex[2].y(),
										alpha * t.vertex[0].z() + beta * t.vertex[1].z() + gamma * t.vertex[2].z(),
										1.0f);
				Vec3 pos = pos_vec4.head<3>();
				Vec3 color = alpha * t.color[0] + beta * t.color[1] + gamma * t.color[2];
				Vec3 normal = alpha * t.normal[0] + beta * t.normal[1] + gamma * t.normal[2];
				normal = ((model.inverse()).transpose().block<3, 3>(0, 0) * normal).normalized();
				Vec2 text_coord = alpha * t.text_coord[0] + beta * t.text_coord[1] + gamma * t.text_coord[2];
				
				Shader::FragmentPayload f_p(pos, color, text_coord, normal, texture ? &*texture : nullptr);
				Vec3 shaded_color = fragmentShader(f_p, lights);
				pixel_buffer.at<cv::Vec3b>(y, x)[0] = (uchar)(shaded_color.z() * 255);
				pixel_buffer.at<cv::Vec3b>(y, x)[1] = (uchar)(shaded_color.y() * 255);
				pixel_buffer.at<cv::Vec3b>(y, x)[2] = (uchar)(shaded_color.x() * 255);
			}
		}
	}

	return;
}
