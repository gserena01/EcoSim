#pragma once
#include <string>
#include <vector>
#include <array>
class Texture
{
public:
	Texture();
	Texture(int resolution);
	static void loadFile(const std::string& fileName);
	std::vector<std::vector<float>> tex_arr_2d;
	std::array<std::array<std::array<float, 4>, 32>, 32> tex_arr_3d;
};

