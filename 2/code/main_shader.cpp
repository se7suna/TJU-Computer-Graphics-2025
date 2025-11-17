/* process each shader

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
	// initialize rasterizer
	int w = 1600, h = 900;
	Rasterizer rasterizer(w, h);

	float angle = 0;
	Vec3 pos(-4, 3, -4);
	Vec3 center(0.0, 0.0, 0.0);
	Vec3 up(0.0, 1.0, 0.0);	
	rasterizer.setModel(model(angle));
	rasterizer.setView(view(pos, center, up));
	rasterizer.setProjection(perspective(60, (float)w/(float)h, 0.1, 50));
	

	// load triangles
	vector<Triangle*> triangles;

	objl::Loader loader;
	bool loadout = loader.LoadFile("../res/test.obj");
	for (auto mesh : loader.LoadedMeshes) {
		for (int i = 0; i < mesh.Vertices.size(); i += 3) {
			Triangle* t = new Triangle();
			for (int j = 0; j < 3; j++) {
				t->setVertex(j, Vec3(mesh.Vertices[i + j].Position.X,
					mesh.Vertices[i + j].Position.Y,
					mesh.Vertices[i + j].Position.Z));
				t->setNormal(j, Vec3(mesh.Vertices[i + j].Normal.X,
					mesh.Vertices[i + j].Normal.Y,
					mesh.Vertices[i + j].Normal.Z));
				t->setTextCoord(j, Vec2(mesh.Vertices[i + j].TextureCoordinate.X,
					mesh.Vertices[i + j].TextureCoordinate.Y));
			}
			triangles.push_back(t);
		}
	}


	// load texture
	//rasterizer.setTexture(Texture("../res/texture.png"));
	rasterizer.setTexture(Texture("../res/hmap_texture.jpg"));


	// rasterize per triangle
	for (auto& t : triangles)
		rasterizer.drawTriangle(*t);


	// show result
	cv::imshow("rasterizer", rasterizer.getPixels());
	cv::imwrite("../output/output.png", rasterizer.getPixels());
	cv::waitKey(0);

	return 0;
}

*/