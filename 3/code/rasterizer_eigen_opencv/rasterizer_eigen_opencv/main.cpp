#include "rasterizer.hpp"
#include "triangle.hpp"
#include "texture.hpp"
#include "shader.hpp"
#include "geometry.hpp"
#include "material.hpp"
#include "skybox.hpp"
#include "OBJ_Loader.h"
#include <vector>
#include <opencv2/opencv.hpp>
#include <Eigen/Eigen>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv) {
	// initialize rasterizer
	int w = 1600, h = 900;
	Rasterizer rasterizer(w, h);

	Vec3 pos(-3, 8, -5);
	Vec3 center(0.0, 5.0, 0.0);
	Vec3 up(0.0, 1.0, 0.0);	
	rasterizer.setView(view(pos, center, up));
	rasterizer.setProjection(perspective(80, (float)w/(float)h, 0.1, 50));
	
	// Load skybox
	Skybox skybox;
	Skybox* skybox_ptr = nullptr;
	if (skybox.loadFromFile("../res/skyboxes/HdrOutdoorFieldBaseballDayClear001/HdrOutdoorFieldBaseballDayClear001_JPG_4K.JPG")) {
		rasterizer.setSkybox(skybox);
		skybox_ptr = &skybox;
	}
	
	// Set PBR shader
	Shader shader;
	rasterizer.setFragmentShader(
		[&shader, skybox_ptr](const Shader::FragmentPayload& fragment_payload, const std::vector<Shader::Light>& lights) {
			return shader.pbrShader(fragment_payload, lights, skybox_ptr);
		}
	);
	
	// Load PBR materials
	PBRMaterial metal_material = loadPBRMaterial("../res/materials/Poliigon_MetalPaintedMatte_7037/1K");
	PBRMaterial stone_material = loadPBRMaterial("../res/materials/Poliigon_StoneQuartzite_8060/1K");
	
	// Load cube geometry
	objl::Loader loader;
	bool loadout = false;
	try {
		loadout = loader.LoadFile("../res/objects/test.obj");
	} catch (const std::exception& e) {
		std::cerr << "Error loading OBJ file: " << e.what() << std::endl;
		return 1;
	}
	
	if (!loadout) {
		std::cerr << "Failed to load OBJ file: ../res/objects/cube.obj" << std::endl;
		return 1;
	}
	
	vector<Triangle*> testobj_triangles;
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
			testobj_triangles.push_back(t);
		}
	}

	// Draw first object with metal material (left)
	Vec3 angles1(0, 0, 0);
	Vec3 axis1(0, 0, 0);
	Mat4 model1 = model(angles1, axis1);
	Mat4 translation1;
	translation1 << 
		1, 0, 0, -1.5,
		0, 1, 0, 5.0,
		0, 0, 1, 0,
		0, 0, 0, 1;
	rasterizer.setModel(translation1 * model1);
	rasterizer.setPBRMaterial(&stone_material);
	for (auto& t : testobj_triangles)
		rasterizer.drawTriangle(*t);
	
	// Draw second object with stone material (right)
	Vec3 angles2(0, 0, 0);
	Vec3 axis2(0, 0, 0);
	Mat4 model2 = model(angles2, axis2);
	Mat4 translation2;
	translation2 << 
		1, 0, 0, 1.5,
		0, 1, 0, 5.0,
		0, 0, 1, 0,
		0, 0, 0, 1;
	rasterizer.setModel(translation2 * model2);
	rasterizer.setPBRMaterial(&metal_material);
	for (auto& t : testobj_triangles)
		rasterizer.drawTriangle(*t);
	
	// Draw skybox (should be drawn after geometry for proper depth testing)
	rasterizer.drawSkybox();

	// Save result
	cv::imwrite("../output/output.png", rasterizer.getPixels());

	return 0;
}

#if 0
// Old code for reference
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

        // ���֡
        string filename = "../output/frames_bot/frame_" + to_string(f) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        angle_y += 2.0f;
    }

    float angle_x = 0;
    for (int f = 30; f < 90; f++) {
        rasterizer.clear();

        rasterizer.setModel(model(Vec3(0, angle_y, 0), axis));
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

        // ���֡
        string filename = "../output/frames_bot/frame_" + to_string(f) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        angle_x += 2.0f;
    }
    
    for (int f = 90; f < 150; f++) {
        rasterizer.clear();

        rasterizer.setModel(model(Vec3(0, angle_y, 0), axis));
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

        // ���֡
        string filename = "../output/frames_bot/frame_" + to_string(f) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        angle_x -= 2.0f;
        angle_y -= 0.8f;
    }

    for (int f = 150; f < 300; f++) {
        rasterizer.clear();

        rasterizer.setModel(model(Vec3(0, angle_y, 0), axis));
        for (auto& t : head_triangles)
            rasterizer.drawTriangle(*t);
        rasterizer.setModel(model(angles, axis));
        for (auto& t : body_triangles)
            rasterizer.drawTriangle(*t);

        rasterizer.setModel(model(Vec3(-angle_x, 0, 0), Vec3(0, 1.4, 0)));
        for (auto& t : right_arm_triangles)
            rasterizer.drawTriangle(*t);
        rasterizer.setModel(model(Vec3(angle_x, 0, 0), Vec3(0, 1.4, 0)));
        for (auto& t : left_arm_triangles)
            rasterizer.drawTriangle(*t);
        rasterizer.setModel(model(Vec3(-angle_x, 0, 0), Vec3(0, 0.6, 0)));
        for (auto& t : left_leg_triangles)
            rasterizer.drawTriangle(*t);
        rasterizer.setModel(model(Vec3(angle_x, 0, 0), Vec3(0, 0.6, 0)));
        for (auto& t : right_leg_triangles)
            rasterizer.drawTriangle(*t);


        // ���֡
        string filename = "../output/frames_bot/frame_" + to_string(f) + ".png";
        cv::imwrite(filename, rasterizer.getPixels());

        if (((f - 150) / 15) % 4 == 0 || ((f - 150) / 15) % 4 == 3)
            angle_x -= 2.0f;
        else
            angle_x += 2.0f;
    }
#endif