#include "World/ChunkManager.h"
#include "Entities/Player.h"

ChunkManager::ChunkManager(std::shared_ptr<Player> player) : _player(player) {
    _generationPool = std::make_unique<ThreadPool>(1);
    _meshingPool = std::make_unique<ThreadPool>(1);
    _worldUpdatePool = std::make_unique<ThreadPool>(1);

    //TODO: update based on where the player position starts at
    //create first 9 chunks that the player is standing on
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            glm::ivec2 position(x, y);
            _worldChunks[position] = new Chunk();
            Chunk* newChunk = _worldChunks[position];
            newChunk->position = position;
            newChunk->Generate();
        }
    }
    //Set player spawn point
    int z = 255;
    while (GetGlobalBlock(glm::vec3(0, 0, z)) == 0) {
        z--;
    }
    _player->SetPosition(glm::vec3(0, 0, z));
}

void ChunkManager::Update(Shader& blockShader) {
    glm::vec3 playerPosition = _player->GetPosition();
    blockShader.use();
    if (!clearingChunks.load()) {
        _worldUpdatePool->enqueue([this, playerPosition] {
            CheckChunksForDeletion(playerPosition);
            });
        clearingChunks.store(true);
    }
    //loop through chunks in spiral starting from player position
    int x = 0;
    int y = 0;
    int dx = 0;
    int dy = -1;

    //Free any chunks in cleanup buffer
    ProcessMeshUpload();
    ProcessChunkCleanup();
    for (int i = 0; i < (RenderDistance * 2) * (RenderDistance * 2); i++) {
        std::lock_guard<std::mutex> lock(_worldChunksMutex);
        glm::ivec2 position(playerPosition.x / 16.0f + x, playerPosition.y / 16.0f + y);
        if (std::sqrt(x * x + y * y) <= RenderDistance) {
            Chunk* chunk = _worldChunks[position];
            //Don't operate on the chunk if it has been scheduled for deletion
            if (chunk) {
                //Do nothing if the chunk hasn't been generated yet
                if (!chunk->generated.load())
                    continue;

                if (chunk->scheduledForDeletion.load())
                    continue;

                //render the chunk if it has a valid mesh
                if (chunk->uploadComplete.load()) {
                    chunk->Render(blockShader);
                }
                if (!chunk->meshBuildQueued.load() && chunk->requiresRemesh.load()) {
                    //std::cout << "here" << std::endl;
                    //Update the chunks neighbors when the mesh is built so it can access the neighbor chunks for proper face culling
                    auto it = _worldChunks.find(position + glm::ivec2(0, 1));
                    if (it != _worldChunks.end())
                        chunk->NorthNeighbor = it->second;

                    it = _worldChunks.find(position + glm::ivec2(1, 0));
                    if (it != _worldChunks.end())
                        chunk->EastNeighbor = it->second;

                    it = _worldChunks.find(position + glm::ivec2(0, -1));
                    if (it != _worldChunks.end())
                        chunk->SouthNeighbor = it->second;

                    it = _worldChunks.find(position + glm::ivec2(-1, 0));
                    if (it != _worldChunks.end())
                        chunk->WestNeighbor = it->second;
                    chunk->meshBuildQueued.store(true);
                    _meshingPool->enqueue([this, chunk] {
                        chunk->BuildMesh();
                        if (!chunk->requiresRemesh.load())
                            _meshUploadQueue.push(chunk);
                    });
                    //meshUploadQueue.push(chunk);
                }
            }
            else {
                //If chunk is nullptr, create the chunk and add it's generation to the generation ThreadPool
                _worldChunks[position] = new Chunk();
                Chunk* newChunk = _worldChunks[position];
                newChunk->position = position;
                _generationPool->enqueue([newChunk] {
                    newChunk->Generate();
                    });
            }
        }
        if (x == y || (x < 0 && x == -y) || (x > 0 && x == 1 - y)) {
            int temp = dx;
            dx = -dy;
            dy = temp;
        }
        x += dx;
        y += dy;
    }
}

void ChunkManager::CheckChunksForDeletion(const glm::vec3& playerPosition) {
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::vector<std::pair<glm::ivec2, Chunk*>> pairs;
    //minize lock time, get snapshot of valid keys
    {
        std::lock_guard<std::mutex> lock(_worldChunksMutex);
        if (_worldChunks.empty()) 
            return;
        pairs.reserve(_worldChunks.size());
        for (const auto& pair : _worldChunks)
            pairs.push_back(pair);
    }
    for (const auto& pair : pairs) {
        Chunk* chunk = pair.second;
        if (!chunk)
            continue;
        //don't queue the chunk if it was already scheduled, or if it hasn't finished loading
        if (chunk->scheduledForDeletion.load() || !chunk->uploadComplete.load())
            continue;
        if (glm::distance(chunk->position, glm::vec2(playerPosition / 16.0f)) > RenderDistance + 2) {
            chunk->scheduledForDeletion.store(true);
            std::lock_guard<std::mutex> lock(_cleanupMutex);
            _cleanupQueue.push_back(chunk);
        }
    }
    clearingChunks.store(false);
}

void ChunkManager::ProcessChunkCleanup() {
    std::lock_guard<std::mutex> lock(_cleanupMutex);
    for (Chunk* chunk : _cleanupQueue) {
        _worldChunks.erase(chunk->position);
        delete chunk;
    }
    _cleanupQueue.clear();
}

void ChunkManager::ProcessMeshUpload() {
    int numUploads = 0;
    while (_meshUploadQueue.size() > 0) {
        Chunk* chunk = _meshUploadQueue.front();
        _meshUploadQueue.pop();
        chunk->UploadToGPU();
        chunk->uploadComplete.store(true);
        numUploads++;
        if (numUploads >= maxUploadsPerFrame)
            break;
    }
}

void ChunkManager::Terminate() {
    _worldUpdatePool->join();
    _meshingPool->join();
    _generationPool->join();
}

int ChunkManager::GetGlobalBlock(const glm::ivec3& position) {
    int chunkX = static_cast<int>(std::floor(position.x / 16.0f));
    int chunkY = static_cast<int>(std::floor(position.y / 16.0f));

    if (position.z < 0 || position.z > 255) 
        return 0;
    glm::ivec2 chunkPos(chunkX, chunkY);
    if (!_worldChunks.count(chunkPos))
        return 0;
    Chunk* chunk = _worldChunks[chunkPos];
    if (!chunk->generated.load())
        return 0;
    int x = position.x % 16;
    if (x < 0)
        x += 16;
    int y = position.y % 16;
    if (y < 0)
        y += 16;
    return chunk->GetBlock(x, y, position.z);
}

bool ChunkManager::TryBreakBlock(const glm::ivec3& position, bool forceUpdate) {
    int chunkX = static_cast<int>(std::floor(position.x / 16.0f));
    int chunkY = static_cast<int>(std::floor(position.y / 16.0f));
    if (position.z < 0 || position.z > 255)
        return 0;
    glm::ivec2 chunkPos(chunkX, chunkY);
    if (!_worldChunks.count(chunkPos))
        return false;
    Chunk* chunk = _worldChunks[chunkPos];
    if (!chunk->generated.load())
        return false;
    int x = position.x % 16;
    if (x < 0)
        x += 16;
    int y = position.y % 16;
    if (y < 0)
        y += 16;
    int blockID = chunk->GetBlock(x, y, position.z);
    if (blockID == 0)
        return false;
    chunk->SetBlock(x, y, position.z, 0);
    chunk->requiresRemesh.store(true);
    return true;
}