#pragma once
#include <vector>
#include <unordered_map>
#include "Shader.h"
#include "SimplexNoise.h"

struct Chunk {
	static SimplexNoise terrainNoise;
	glm::vec2 position;
	std::vector<int> blocks;
	std::vector<uint8_t> vertices;
	Chunk* NorthNeighbor = nullptr;
	Chunk* EastNeighbor = nullptr;
	Chunk* SouthNeighbor = nullptr;
	Chunk* WestNeighbor = nullptr;
	bool generated = false;
	bool meshBuildQueued = false;
	bool meshed = false;
	GLuint MeshVAO = 0, MeshVBO = 0;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
	inline int GetBlock(int x, int y, int z) const noexcept {
    // Fast path, no branching if you already guarantee valid ranges (0–15, 0–15, 0–255)
		return blocks[x * (16 * 256) + y * 256 + z];
	}
	void SetBlock(int x, int y, int z, int ID);
};