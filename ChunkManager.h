#pragma once
#include "ThreadPool.h"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include "Shader.h"

class ChunkManager {
public:
	int RenderDistance = 48;
	int MaxUploadsPerFrame = 10;
	ChunkManager() {
		generationPool = std::make_unique<ThreadPool>(1);
		meshingPool = std::make_unique<ThreadPool>(1);
		worldUpdatePool = std::make_unique<ThreadPool>(1);

		//TODO: update based on where the player position starts at
		//create first 9 chunks that the player is standing on
		for (int x = -1; x <= 1; x++) {
			for (int y = -1; y <= 1; y++) {
				glm::ivec2 position(x, y);
				worldChunks[position] = new Chunk();
				Chunk* newChunk = worldChunks[position];
				newChunk->position = position;
				newChunk->Generate();
			}
		}
	}
	void Update(const glm::vec3 &playerPosition, Shader& blockShader);
	void Terminate();
	int GetGlobalBlock(const glm::ivec3& position);
	std::atomic<bool> clearingChunks{ false };
private:
	struct IVec2Hash {
		size_t operator()(const glm::ivec2& v) const noexcept {
			return (std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1));
		}
	};
	std::unordered_map<glm::ivec2, Chunk*, IVec2Hash> worldChunks;
	std::unique_ptr<ThreadPool> generationPool;
	std::unique_ptr<ThreadPool> meshingPool;
	std::unique_ptr<ThreadPool> worldUpdatePool;
	const int maxUploadsPerFrame = 5;
	std::mutex cleanupMutex;
	std::mutex worldChunksMutex;
	std::vector<Chunk*> cleanupQueue;
	std::queue<Chunk*> meshUploadQueue;

	void CheckChunksForDeletion(const glm::vec3& playerPosition);
	void ProcessChunkCleanup();
	void ProcessMeshUpload();
};