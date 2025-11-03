#pragma once
#include <vector>
#include <unordered_map>
#include "Shader.h"
#include "SimplexNoise.h"

struct Vertex {
	uint8_t x, y, z;
	uint8_t face;
	uint8_t corner;
	uint8_t block;
	Vertex(uint8_t x, uint8_t y, uint8_t z, uint8_t f, uint8_t c, uint8_t b) : x(x), y(y), z(z), face(f), corner(c), block(b) {}
};

struct Chunk {
	static SimplexNoise terrainNoise;
	/// <summary>
	/// The position of the chunk in the world. 
	/// Chunks are every 16 tiles. 
	/// Chunk position is stored in increments of 1.
	/// </summary>
	glm::vec2 position;
	/// <summary>
	/// The mesh generation data. Stores what blockID is at what position
	/// </summary>
	std::vector<int> blocks;
	/// <summary>
	/// The mesh vertex data that is uploaded to the gpu
	/// </summary>
	std::vector<Vertex> vertices;
	Chunk* NorthNeighbor = nullptr;
	Chunk* EastNeighbor = nullptr;
	Chunk* SouthNeighbor = nullptr;
	Chunk* WestNeighbor = nullptr;
	/// <summary>
	/// Thread safety. True if the generation operation has already been scheduled.
	/// </summary>
	bool generated = false;
	/// <summary>
	/// Thread safety. True if the mesh operation has already been scheduled.
	/// </summary>
	bool meshBuildQueued = false;
	/// <summary>
	/// True if the chunk has a valid mesh
	/// </summary>
	bool meshed = false;
	/// <summary>
	/// True if the mesh needs to be rebuilt
	/// </summary>
	bool meshDirty = false;
	/// <summary>
	/// Thread safety. True if the chunk has been scheduled for deletion
	/// </summary>
	bool scheduledForDeletion = false;
	GLuint MeshVAO = 0, MeshVBO = 0;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
	inline int GetBlock(int x, int y, int z) const noexcept {
    // Fast path, no branching if you already guarantee valid ranges (0–15, 0–15, 0–255)
		return blocks[x * (16 * 256) + y * 256 + z];
	}
	void SetBlock(int x, int y, int z, int ID);
	void ClearGPU();
	~Chunk() {
		ClearGPU();
	}
};