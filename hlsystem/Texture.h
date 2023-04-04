#pragma once
#include <string>
#include <vector>
#include <array>
class Texture
{
public:
	Texture();
	Texture(int resolution, int depth);
	static void loadFile(const std::string& fileName);

	std::array<std::array<std::array<float, 4>, 32>, 32> tex_arr_3d;
	std::array<std::array<float, 32>, 32> tex_arr_2d;

};

