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

    Vec3 angles(0, 0, 0);
    Vec3 axis(0, 0, 0);
	Vec3 pos(-2, 3, -4);
	Vec3 center(0.0, 0.3, 0.0);
	Vec3 up(0.0, 1.0, 0.0);	
	rasterizer.setModel(model(angles, axis));
	rasterizer.setView(view(pos, center, up));
	rasterizer.setProjection(perspective(80, (float)w/(float)h, 0.1, 50));
	
	/*
	// load materials and triangles
	vector<Triangle*> triangles;

	objl::Loader loader;
	bool loadout = loader.LoadFile("../res/bot.obj");
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
	rasterizer.setTexture(Texture("../res/texture.png"));
	//rasterizer.setTexture(Texture("../res/hmap_texture.jpg"));


	// rasterize per triangle
	for (auto& t : triangles)
		rasterizer.drawTriangle(*t);


	// show result
	cv::imshow("rasterizer", rasterizer.getPixels());
	cv::imwrite("../output/output.png", rasterizer.getPixels());
	cv::waitKey(0);
	*/

    vector<Triangle*> head_triangles;
    vector<Triangle*> body_triangles;
    vector<Triangle*> left_arm_triangles;
    vector<Triangle*> right_arm_triangles;
    vector<Triangle*> left_leg_triangles;
    vector<Triangle*> right_leg_triangles;
    
    objl::Loader loader;
    bool loadout = loader.LoadFile("../res/bot.obj");

    for (auto& mesh : loader.LoadedMeshes) {
        string name = mesh.MeshName;
        std::vector<Triangle*>* target = nullptr;

        if (name.find("head") != string::npos)
            target = &head_triangles;
        else if (name.find("body") != string::npos)
            target = &body_triangles;
        else if (name.find("left_arm") != string::npos) 
            target = &left_arm_triangles;
        else if (name.find("right_arm") != string::npos)
            target = &right_arm_triangles;
        else if (name.find("left_leg") != string::npos) 
            target = &left_leg_triangles;
        else if (name.find("right_leg") != string::npos)
            target = &right_leg_triangles;
        else 
            target = &body_triangles;

        for (int i = 0; i < mesh.Vertices.size(); i += 3) {
            Triangle* t = new Triangle();
            for (int j = 0; j < 3; j++) {
                t->setVertex(j, Vec3(
                    mesh.Vertices[i + j].Position.X,
                    mesh.Vertices[i + j].Position.Y,
                    mesh.Vertices[i + j].Position.Z));
                t->setNormal(j, Vec3(
                    mesh.Vertices[i + j].Normal.X,
                    mesh.Vertices[i + j].Normal.Y,
                    mesh.Vertices[i + j].Normal.Z));
                t->setTextCoord(j, Vec2(
                    mesh.Vertices[i + j].TextureCoordinate.X,
                    mesh.Vertices[i + j].TextureCoordinate.Y));
            }
            target->push_back(t);
        }
    }

    rasterizer.setTexture(Texture("../res/hmap_texture.jpg"));

    float angle_y = 0;
    for (int f = 0; f < 30; f++) {
        rasterizer.clear();

        rasterizer.setModel(model(angles, axis));
        for (auto& t : body_triangles) 
            rasterizer.drawTriangle(*t);
        for (auto& t : left_arm_triangles)
            rasterizer.drawTriangle(*t);
        for (auto& t : right_arm_triangles) 
            rasterizer.drawTriangle(*t);
        for (auto& t : left_leg_triangles) 
            rasterizer.drawTriangle(*t);
        for (auto& t : right_leg_triangles) 
            rasterizer.drawTriangle(*t);

        rasterizer.setModel(model(Vec3(0, angle_y, 0), axis));
        for (auto& t : head_triangles) 
            rasterizer.drawTriangle(*t);

        // Êä³öÖ¡
        string filename = "../output/frames_bot/frame_" + to_string(f) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        angle_y += 2.0f;
    }

    float angle_x = 0;
    for (int f = 0; f < 60; f++) {
        rasterizer.clear();

        rasterizer.setModel(model(Vec3(0, 60, 0), axis));
        for (auto& t : head_triangles)
            rasterizer.drawTriangle(*t);
        rasterizer.setModel(model(angles, axis));
        for (auto& t : body_triangles) 
            rasterizer.drawTriangle(*t);
        for (auto& t : left_arm_triangles)
            rasterizer.drawTriangle(*t);
        for (auto& t : left_leg_triangles) 
            rasterizer.drawTriangle(*t);
        for (auto& t : right_leg_triangles) 
            rasterizer.drawTriangle(*t);

        rasterizer.setModel(model(Vec3(angle_x, 0, 0), Vec3(0, 1.4, 0)));
        for (auto& t : right_arm_triangles)
            rasterizer.drawTriangle(*t);

        // Êä³öÖ¡
        string filename = "../output/frames_bot/frame_" + to_string(f + 30) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        angle_x += 2.0f;
    }

	return 0;
}