#pragma once
#include <vector>
#include <unordered_map>
#include "Shader.h"
#include "SimplexNoise.h"

struct Chunk {
	static SimplexNoise terrainNoise;
	glm::vec2 position;
	std::vector<int> blocks;
	std::vector<GLubyte> vertices;
	Chunk* NorthNeighbor = nullptr;
	Chunk* SouthNeighbor = nullptr;
	Chunk* EastNeighbor = nullptr;
	Chunk* WestNeighbor = nullptr;
	bool generated = false;
	bool meshed = false;
	GLuint MeshVAO = 0, MeshVBO = 0;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
	int GetBlock(int x, int y, int z);
	void SetBlock(int x, int y, int z, int ID);
};