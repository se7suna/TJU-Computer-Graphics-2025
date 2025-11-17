#include "material.hpp"
#include <Eigen/Eigen>
#include <fstream>

Material::Material() {
	Ks = Vec3(0.7937, 0.7937, 0.7937);
	Kd = Vec3(1.0, 1.0, 1.0);
	Ka = Vec3(0.005, 0.005, 0.005);
	Ns = 32.0;
	Ni = 1.0;
	d = 1.0;
	illum = 2.0;
}

void loadMaterials(const string& filename) {
	ifstream file(filename);
	string line, name;
	Material new_mat;

	while (getline(file, line)) {
		stringstream ss(line);
		string str;
		ss >> str;

		if (str == "newmtl") {
			if (!name.empty())
				all_materials[name] = new_mat;
			ss >> name;
			new_mat = Material();
		}
		else if (str == "Ks") {
			float x, y, z;
			ss >> x >> y >> z;
			new_mat.Ks = Vec3(x, y, z);
		}
		else if (str == "Kd") {
			float x, y, z;
			ss >> x >> y >> z;
			new_mat.Kd = Vec3(x, y, z);
		}
		else if (str == "Ka") {
			float x, y, z;
			ss >> x >> y >> z;
			new_mat.Ka = Vec3(x, y, z);
		}
		else if (str == "Ns") {
			float tmp;
			ss >> tmp;
			new_mat.Ns = tmp;
		}
		else if (str == "Ni") {
			float tmp;
			ss >> tmp;
			new_mat.Ni = tmp;
		}
		else if (str == "d") {
			float tmp;
			ss >> tmp;
			new_mat.d = tmp;
		}
		else if (str == "illum") {
			float tmp;
			ss >> tmp;
			new_mat.illum = tmp;
		}
	}

	return;
}