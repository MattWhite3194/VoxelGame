#pragma once
#include <vector>
#include <unordered_map>
#include "Shader.h"

struct Chunk {
	glm::vec2 position;
	std::vector<int> blocks;
	std::vector<GLubyte> vertices;
	Chunk* NorthNeighbor;
	Chunk* SouthNeighbor;
	Chunk* EastNeighbor;
	Chunk* WestNeighbor;
	bool generated = false;
	bool meshed = false;
	GLuint MeshVAO, MeshVBO;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
	int GetBlock(int x, int y, int z);
};