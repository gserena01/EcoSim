#pragma once
#include <string>
#include <vector>
class Texture
{
public:
	Texture();
	Texture(int resolution);
	static void loadFile(const std::string& fileName);
	std::vector<std::vector<float>> tex_arr_2d;
	std::vector<std::vector<std::vector<float>>> tex_arr_3d;
};

