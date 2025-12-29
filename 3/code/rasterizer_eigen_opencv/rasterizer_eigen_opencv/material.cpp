// Disable std::byte to avoid conflict with Windows SDK
#define _HAS_STD_BYTE 0

#include "material.hpp"
#include <Eigen/Eigen>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <opencv2/opencv.hpp>
#ifdef _WIN32
#include <windows.h>
#include <io.h>
#define access _access
#else
#include <unistd.h>
#endif
using namespace std;

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
	Material new_material;

	while (getline(file, line)) {
		stringstream ss(line);
		string str;
		ss >> str;

		if (str == "newmtl") {
			if (!name.empty())
				all_materials[name] = new_material;
			ss >> name;
			new_material = Material();
		}
		else if (str == "Ks") {
			float x, y, z;
			ss >> x >> y >> z;
			new_material.Ks = Vec3(x, y, z);
		}
		else if (str == "Kd") {
			float x, y, z;
			ss >> x >> y >> z;
			new_material.Kd = Vec3(x, y, z);
		}
		else if (str == "Ka") {
			float x, y, z;
			ss >> x >> y >> z;
			new_material.Ka = Vec3(x, y, z);
		}
		else if (str == "Ns") {
			float tmp;
			ss >> tmp;
			new_material.Ns = tmp;
		}
		else if (str == "Ni") {
			float tmp;
			ss >> tmp;
			new_material.Ni = tmp;
		}
		else if (str == "d") {
			float tmp;
			ss >> tmp;
			new_material.d = tmp;
		}
		else if (str == "illum") {
			float tmp;
			ss >> tmp;
			new_material.illum = tmp;
		}
	}

	return;
}

map<string, Material> all_materials;
map<string, PBRMaterial> all_pbr_materials;

PBRMaterial::PBRMaterial() {
	albedo = Vec3(1.0f, 1.0f, 1.0f);
	metallic = 0.0f;
	roughness = 0.5f;
}

bool fileExists(const string& filename) {
	return access(filename.c_str(), 0) == 0;
}

string findTextureFile(const string& dir, const vector<string>& patterns) {
	vector<string> extensions = { "*.jpg", "*.png", "*.tiff", "*.tif", "*.jpeg" };
	vector<string> files;
	
	// Normalize path separators for Windows
	string normalized_dir = dir;
	for (size_t i = 0; i < normalized_dir.size(); i++) {
		if (normalized_dir[i] == '/') {
			normalized_dir[i] = '\\';
		}
	}
	
	// Search in the directory itself
	for (const auto& ext : extensions) {
		string pattern = normalized_dir + "\\" + ext;
		vector<cv::String> cv_files;
		try {
			cv::glob(pattern, cv_files, false);
			for (const auto& f : cv_files) {
				files.push_back(string(f));
			}
		} catch (...) {
			// Ignore glob errors
		}
	}
	
	// Search in subdirectories (only directories, not files)
	vector<cv::String> all_items;
	try {
		cv::glob(normalized_dir + "\\*", all_items, false);
		for (const auto& item : all_items) {
			string item_str = string(item);
			// Check if it's a directory (simple heuristic: no extension or ends with path separator)
			// We'll use access to check if it's a directory
			#ifdef _WIN32
			DWORD attrs = GetFileAttributesA(item_str.c_str());
			if (attrs != INVALID_FILE_ATTRIBUTES && (attrs & FILE_ATTRIBUTE_DIRECTORY)) {
				for (const auto& ext : extensions) {
					string pattern = item_str + "\\" + ext;
					vector<cv::String> cv_files;
					try {
						cv::glob(pattern, cv_files, false);
						for (const auto& f : cv_files) {
							files.push_back(string(f));
						}
					} catch (...) {
						// Ignore glob errors
					}
				}
			}
			#else
			struct stat info;
			if (stat(item_str.c_str(), &info) == 0 && S_ISDIR(info.st_mode)) {
				for (const auto& ext : extensions) {
					string pattern = item_str + "/" + ext;
					vector<cv::String> cv_files;
					try {
						cv::glob(pattern, cv_files, false);
						for (const auto& f : cv_files) {
							files.push_back(string(f));
						}
					} catch (...) {
						// Ignore glob errors
					}
				}
			}
			#endif
		}
	} catch (...) {
		// Ignore glob errors
	}
	
	for (const auto& file : files) {
		string filename = file;
		size_t last_slash = filename.find_last_of("/\\");
		if (last_slash != string::npos) {
			filename = filename.substr(last_slash + 1);
		}
		
		string lower_filename = filename;
		transform(lower_filename.begin(), lower_filename.end(), lower_filename.begin(), ::tolower);
		
		for (const auto& pattern : patterns) {
			string pattern_lower = pattern;
			transform(pattern_lower.begin(), pattern_lower.end(), pattern_lower.begin(), ::tolower);
			
			if (lower_filename.find(pattern_lower) != string::npos) {
				return file;
			}
		}
	}
	
	return "";
}

PBRMaterial loadPBRMaterial(const string& materialPath) {
	PBRMaterial mat;
	
	vector<string> albedo_patterns = {"basecolor", "albedo", "_col_", "_color", "_diffuse"};
	string albedo_file = findTextureFile(materialPath, albedo_patterns);
	if (!albedo_file.empty()) {
		try {
			mat.albedo_map = Texture(albedo_file);
		} catch (...) {
			// Failed to load texture
		}
	}
	
	// Normal map
	vector<string> normal_patterns = {"normal", "_nrm_", "_norm", "_n_", "_bump"};
	string normal_file = findTextureFile(materialPath, normal_patterns);
	if (!normal_file.empty()) {
		try {
			mat.normal_map = Texture(normal_file);
		} catch (...) {
		}
	}
	
	// Metallic map
	vector<string> metallic_patterns = {"metallic", "_metal", "_met_", "_metallness"};
	string metallic_file = findTextureFile(materialPath, metallic_patterns);
	if (!metallic_file.empty()) {
		try {
			mat.metallic_map = Texture(metallic_file);
		} catch (...) {
		}
	}
	
	// Roughness map
	vector<string> roughness_patterns = {"roughness", "_rough", "_rgh_", "_roughness"};
	string roughness_file = findTextureFile(materialPath, roughness_patterns);
	if (!roughness_file.empty()) {
		try {
			mat.roughness_map = Texture(roughness_file);
		} catch (...) {
		}
	}
	
	// AO map
	vector<string> ao_patterns = {"ao", "ambientocclusion", "_occlusion", "_ao_", "_ambient"};
	string ao_file = findTextureFile(materialPath, ao_patterns);
	if (!ao_file.empty()) {
		try {
			mat.ao_map = Texture(ao_file);
		} catch (...) {
		}
	}
	
	return mat;
}