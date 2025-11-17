#pragma once

#include "triangle.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include <vector>
#include <optional>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

using namespace std;
using Vec2 = Eigen::Vector2f;
using Vec3 = Eigen::Vector3f;
using Vec4 = Eigen::Vector4f;
using Mat2 = Eigen::Matrix2f;
using Mat3 = Eigen::Matrix3f;
using Mat4 = Eigen::Matrix4f;

class Rasterizer {
public:
	Rasterizer(int w, int h);

	void clear();

	void setModel(const Mat4& m);
	void setView(const Mat4& v);
	void setProjection(const Mat4& p);

	void setVertexShader(function<Vec3(const Shader::VertexPayload&)> v_s);
	void setFragmentShader(function<Vec3(const Shader::FragmentPayload&, const vector<Shader::Light>&)> f_s);
	void setTexture(Texture t);

	cv::Mat getPixels() const;

	void drawTriangle(const Triangle& t);

private:
	int width, height;

	Mat4 model;
	Mat4 view;
	Mat4 projection;

	function<Vec3(const Shader::VertexPayload&)> vertexShader;
	function<Vec3(const Shader::FragmentPayload&, const vector<Shader::Light>&)> fragmentShader;
	optional<Texture> texture;

	cv::Mat pixel_buffer; // store color of each pixel, provide to OpenCV to draw image
	vector<float> depth_buffer;
};
