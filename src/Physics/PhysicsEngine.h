#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include "World/ChunkManager.h"
#include "Entities/Player.h"

class PhysicsEngine {
public:
	PhysicsEngine(std::shared_ptr<Player> player, std::shared_ptr<ChunkManager> chunkManager) : _player(player), _chunkManager(chunkManager) {
		_entities.push_back(_player);
	}
	void Update(double delta);
private:
	std::shared_ptr<Player> _player;
	std::vector<std::shared_ptr<Entity>> _entities;
	std::shared_ptr<ChunkManager> _chunkManager;
};