#pragma once
#include "ThreadPool.h"
#include "Chunk.hpp"
#include <glm/glm.hpp>
#include "Shader.h"

class ChunkManager {
public:
	int RenderDistance = 24;
	int MaxUploadsPerFrame = 5;
	void Init();
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