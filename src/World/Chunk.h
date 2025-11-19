#pragma once
#include <vector>
#include <unordered_map>
#include "OpenGL/Shader.h"
#include "Generation/SimplexNoise.h"
#include <optional>
#include <mutex>
#include <glad/glad.h>

struct Vertex {
	uint8_t x, y, z;
	uint8_t face;
	uint8_t corner;
	uint8_t block;
	Vertex(uint8_t x, uint8_t y, uint8_t z, uint8_t f, uint8_t c, uint8_t b) : x(x), y(y), z(z), face(f), corner(c), block(b) {}
};

struct Chunk {
	static SimplexNoise hillNoise;
	static SimplexNoise mountainNoise;
	static SimplexNoise ridgeNoise;
	//TODO: voronoi noise for cave generation
	static SimplexNoise caveNoise;
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
	//thread safety, used only by worker thread when building mesh
	std::vector<Vertex> stagingVertices;
	std::mutex meshMutex;
	Chunk* NorthNeighbor = nullptr;
	Chunk* EastNeighbor = nullptr;
	Chunk* SouthNeighbor = nullptr;
	Chunk* WestNeighbor = nullptr;

	//Thread Safety
	std::atomic<bool> generated{ false };
	/// <summary>
	/// True if the mesh operation has already been scheduled.
	/// </summary>
	std::atomic<bool> meshBuildQueued{ false };
	/// <summary>
	/// True if the mesh data has been uploaded to the gpu
	/// </summary>
	std::atomic<bool> uploadComplete{ false };
	/// <summary>
	/// True if the mesh needs to be rebuilt after an update
	/// </summary>
	std::atomic<bool> requiresRemesh{ false };
	std::atomic<bool> scheduledForDeletion{ false };
	GLuint MeshVAO = 0, MeshVBO = 0;
	void Generate();
	void Render(Shader& shader);
	void BuildMesh();
	void AddFace(const uint8_t(&face)[18], const glm::ivec3& position, uint8_t texIndex, uint8_t blockID);
	inline int GetBlock(int x, int y, int z) const noexcept {
    // Fast path, no branching if you already guarantee valid ranges (0–15, 0–15, 0–255)
		return blocks[x * (16 * 256) + y * 256 + z];
	}
	void SetBlock(int x, int y, int z, int ID);
	void UploadToGPU();
	void ClearGPU();
	~Chunk() {
		ClearGPU();
	}
};