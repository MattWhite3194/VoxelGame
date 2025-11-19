#pragma once
#include "Thread/ThreadPool.h"
#include "World/Chunk.h"
#include <glm/glm.hpp>
#include "OpenGL/Shader.h"
#include <memory>

//Forward declaration because circular dependencies are a bitch
class Player;

class ChunkManager {
public:
	int RenderDistance = 12;
	int MaxUploadsPerFrame = 10;
	ChunkManager(std::shared_ptr<Player> player);
	void Update(Shader& blockShader);
	void Terminate();
	int GetGlobalBlock(const glm::ivec3& position);
	bool TryBreakBlock(const glm::ivec3& position, bool forceUpdate);
	std::atomic<bool> clearingChunks{ false };
private:
	struct IVec2Hash {
		size_t operator()(const glm::ivec2& v) const noexcept {
			return (std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1));
		}
	};
	std::unordered_map<glm::ivec2, Chunk*, IVec2Hash> _worldChunks;
	std::shared_ptr<Player> _player;
	std::unique_ptr<ThreadPool> _generationPool;
	std::unique_ptr<ThreadPool> _meshingPool;
	std::unique_ptr<ThreadPool> _worldUpdatePool;
	const int maxUploadsPerFrame = 5;
	std::mutex _cleanupMutex;
	std::mutex _worldChunksMutex;
	std::vector<Chunk*> _cleanupQueue;
	std::queue<Chunk*> _meshUploadQueue;

	void CheckChunksForDeletion(const glm::vec3& playerPosition);
	void ProcessChunkCleanup();
	void ProcessMeshUpload();
};