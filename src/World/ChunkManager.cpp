#include "World/ChunkManager.h"

void ChunkManager::Update(const glm::vec3& playerPosition, Shader& blockShader) {
    if (!clearingChunks.load()) {
        worldUpdatePool->enqueue([this, playerPosition] {
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
        std::lock_guard<std::mutex> lock(worldChunksMutex);
        glm::ivec2 position(playerPosition.x / 16.0f + x, playerPosition.y / 16.0f + y);
        if (std::sqrt(x * x + y * y) <= RenderDistance) {
            Chunk* chunk = worldChunks[position];
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
                else if (!chunk->meshBuildQueued.load() && chunk->requiresRemesh.load()) {
                    //std::cout << "here" << std::endl;
                    //Update the chunks neighbors when the mesh is built so it can access the neighbor chunks for proper face culling
                    auto it = worldChunks.find(position + glm::ivec2(0, 1));
                    if (it != worldChunks.end())
                        chunk->NorthNeighbor = it->second;

                    it = worldChunks.find(position + glm::ivec2(1, 0));
                    if (it != worldChunks.end())
                        chunk->EastNeighbor = it->second;

                    it = worldChunks.find(position + glm::ivec2(0, -1));
                    if (it != worldChunks.end())
                        chunk->SouthNeighbor = it->second;

                    it = worldChunks.find(position + glm::ivec2(-1, 0));
                    if (it != worldChunks.end())
                        chunk->WestNeighbor = it->second;
                    chunk->meshBuildQueued.store(true);
                    meshingPool->enqueue([this, chunk] {
                        chunk->BuildMesh();
                        if (!chunk->requiresRemesh.load())
                            meshUploadQueue.push(chunk);
                    });
                    //meshUploadQueue.push(chunk);
                }
            }
            else {
                //If chunk is nullptr, create the chunk and add it's generation to the generation ThreadPool
                worldChunks[position] = new Chunk();
                Chunk* newChunk = worldChunks[position];
                newChunk->position = position;
                generationPool->enqueue([newChunk] {
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
    //No seams
    //if chunk terrain generated -> queue mesh build
    //if no neighbors -> stop mesh build, requeue it
    //if neighbors -> build mesh, queue gpu upload

    //seams
    //if chunk terrain generated -> queue mesh build
    //if no neighbors -> generate mesh, queue upload to gpu, mark neighbors as dirty
    //if neighbors -> build mesh, queue gpu upload
}

void ChunkManager::CheckChunksForDeletion(const glm::vec3& playerPosition) {
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::vector<std::pair<glm::ivec2, Chunk*>> pairs;
    //minize lock time, get snapshot of valid keys
    {
        std::lock_guard<std::mutex> lock(worldChunksMutex);
        if (worldChunks.empty()) 
            return;
        pairs.reserve(worldChunks.size());
        for (const auto& pair : worldChunks)
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
            std::lock_guard<std::mutex> lock(cleanupMutex);
            cleanupQueue.push_back(chunk);
        }
    }
    clearingChunks.store(false);
}

void ChunkManager::ProcessChunkCleanup() {
    std::lock_guard<std::mutex> lock(cleanupMutex);
    for (Chunk* chunk : cleanupQueue) {
        worldChunks.erase(chunk->position);
        delete chunk;
        std::cout << "Chunk Deleted" << std::endl;
    }
    cleanupQueue.clear();
}

void ChunkManager::ProcessMeshUpload() {
    int numUploads = 0;
    while (meshUploadQueue.size() > 0) {
        Chunk* chunk = meshUploadQueue.front();
        meshUploadQueue.pop();
        chunk->UploadToGPU();
        chunk->uploadComplete.store(true);
        numUploads++;
        if (numUploads >= maxUploadsPerFrame)
            break;
    }
}

void ChunkManager::Terminate() {
    worldUpdatePool->join();
    meshingPool->join();
    generationPool->join();
}

int ChunkManager::GetGlobalBlock(const glm::ivec3& position) {
    int chunkX = static_cast<int>(std::floor(position.x / 16.0f));
    int chunkY = static_cast<int>(std::floor(position.y / 16.0f));

    glm::ivec2 chunkPos(chunkX, chunkY);
    if (!worldChunks.count(chunkPos))
        return 0;
    Chunk* chunk = worldChunks[chunkPos];
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