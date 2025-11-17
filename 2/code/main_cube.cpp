/* rasterize a colorful cube

#include "rasterizer.hpp"
#include "triangle.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "OBJ_Loader.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>

int main(int argc, char** argv) {
	int w = 1600, h = 900;
	Rasterizer rasterizer(w, h);

	Vec3 pos(0.7, 0.8, 1.0);
	Vec3 center(0.0, 0.0, 0.0);
	Vec3 up(0.0, 1.0, 0.0);
	rasterizer.setModel(model());
	rasterizer.setView(camera(pos, center, up));
	rasterizer.setProjection(perspective(80, (float)w/(float)h, 0.1, 50));

	vector<Vec3> cube_vertices = {
		{-0.5, -0.5, -0.5}, // 0
		{-0.5, -0.5,  0.5}, // 1
		{-0.5,  0.5, -0.5}, // 2
		{ 0.5, -0.5, -0.5}, // 3
		{-0.5,  0.5,  0.5}, // 4
		{ 0.5, -0.5,  0.5}, // 5
		{ 0.5,  0.5, -0.5}, // 6
		{ 0.5,  0.5,  0.5}  // 7
	};
	vector<Vec3> cube_colors = {
		{0.0, 0.0, 0.0}, // black
		{1.0, 0.0, 0.0}, // red
		{0.0, 1.0, 0.0}, // green
		{0.0, 0.0, 1.0}, // blue
		{1.0, 1.0, 0.0}, // yellow
		{1.0, 0.0, 1.0}, // pink
		{0.0, 1.0, 1.0}, // cyan
		{1.0, 1.0, 1.0}, // white
	};
	vector<array<int, 4>> cube_faces = {
		{0, 1, 2, 4}, // left
		{0, 1, 3, 5}, // bottom
		{0, 2, 3, 6}, // back
		{1, 4, 5, 7}, // front
		{2, 4, 6, 7}, // top
		{3, 5, 6, 7}, // right
	};

	vector<Triangle> triangles;
	for (auto& f : cube_faces) {
		Triangle t1, t2;
		t1.setVertex(0, cube_vertices[f[0]]);
		t1.setVertex(1, cube_vertices[f[1]]);
		t1.setVertex(2, cube_vertices[f[2]]);
		t2.setVertex(0, cube_vertices[f[3]]);
		t2.setVertex(1, cube_vertices[f[1]]);
		t2.setVertex(2, cube_vertices[f[2]]);
		t1.setColor(0, cube_colors[f[0]]);
		t1.setColor(1, cube_colors[f[1]]);
		t1.setColor(2, cube_colors[f[2]]);
		t2.setColor(0, cube_colors[f[3]]);
		t2.setColor(1, cube_colors[f[1]]);
		t2.setColor(2, cube_colors[f[2]]);
		triangles.push_back(t1);
		triangles.push_back(t2);
	}

	for (auto& t : triangles)
		rasterizer.drawTriangle(t);

	cv::imshow("rasterizer", rasterizer.getPixels());
	cv::imwrite("cube.png", rasterizer.getPixels());
	cv::waitKey(0);

	return 0;
}

*/