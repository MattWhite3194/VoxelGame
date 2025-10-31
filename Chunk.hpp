#pragma once
#include <vector>
#include "Shader.h"

struct Chunk {
	std::vector<int> blocks;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
};