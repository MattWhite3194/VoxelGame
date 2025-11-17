#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

//Use triangle strips to only have 8 vertices per chunk
//On generation, create a vertex buffer of the vertices in the geometry, that way only one draw call is needed to render the entire chunk
//check each block and it's surrounding face to determine which vertices to add

SimplexNoise Chunk::terrainNoise(0.04f);

const uint8_t frontFace[] = {
    0, 1, 0,  // v0 bottom-left
    1, 1, 0,  // v1 bottom-right
    0, 1, 1,  // v2 top-left

    0, 1, 1,  // v2 top-left
    1, 1, 0,  // v1 bottom-right
    1, 1, 1,  // v3 top-right
};

const uint8_t backFace[] = {
    1, 0, 0,  // v0 bottom-right
    0, 0, 0,  // v1 bottom-left
    1, 0, 1,  // v2 top-right

    1, 0, 1,  // v2 top-right
    0, 0, 0,  // v1 bottom-left
    0, 0, 1,  // v3 top-left
};

const uint8_t leftFace[] = {
    0, 0, 0,  // v0 bottom-back
    0, 1, 0,  // v1 bottom-front
    0, 0, 1,  // v2 top-back

    0, 0, 1,  // v2 top-back
    0, 1, 0,  // v1 bottom-front
    0, 1, 1,  // v3 top-front
};

const uint8_t rightFace[] = {
    1, 1, 0,  // v0 bottom-front
    1, 0, 0,  // v1 bottom-back
    1, 1, 1,  // v2 top-front

    1, 1, 1,  // v2 top-front
    1, 0, 0,  // v1 bottom-back
    1, 0, 1,  // v3 top-back
};

const uint8_t bottomFace[] = {
    0, 0, 0,  // v0 back-left
    1, 0, 0,  // v1 back-right
    0, 1, 0,  // v2 front-left

    0, 1, 0,  // v2 front-left
    1, 0, 0,  // v1 back-right
    1, 1, 0,  // v3 front-right
};

const uint8_t topFace[] = {
    0, 1, 1,  // v0 front-left
    1, 1, 1,  // v1 front-right
    0, 0, 1,  // v2 back-left

    0, 0, 1,  // v2 back-left
    1, 1, 1,  // v1 front-right
    1, 0, 1   // v3 back-right
};

void Chunk::Generate() {
	blocks = std::vector<int>(16 * 16 * 256, 0);
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            //Generation for physics testing
            //for (int z = 120; z >= 0; z--) {
            //    if (x > 7 && x < 10 && y > 7 && y < 10 || z < 30)
            //        SetBlock(x, y, z, 1);
            //}
            

            int terrainHeight = (int)(50 * terrainNoise.fractal(5, x / 16.0f + (float)position.x, y / 16.0f + (float)position.y));
            int totalHeight = 60 + terrainHeight;
            for (int z = totalHeight; z >= 0; z--) {
                //Blocks
                //3 grass
                //2 dirt
                //1 stone
                
                if (z == totalHeight)
                    SetBlock(x, y, z, 3);
                else if (z > totalHeight - 3)
                    SetBlock(x, y, z, 2);
                else
                    SetBlock(x, y, z, 1);
            }
        }
    }
    generated.store(true);
    requiresRemesh.store(true);
}

void Chunk::Render(Shader& shader) {
    shader.use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position * 16.0f, 0.0f));
    shader.setMat4("model", model);
    glBindVertexArray(MeshVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void Chunk::BuildMesh() {
    if ((!NorthNeighbor || !NorthNeighbor->generated.load()) ||
        (!SouthNeighbor || !SouthNeighbor->generated.load()) ||
        (!EastNeighbor || !EastNeighbor->generated.load()) ||
        (!WestNeighbor || !WestNeighbor->generated.load())
        ) {
        meshBuildQueued.store(false);
        return;
    }
    // Reserve space for all possible faces once
    auto start = std::chrono::high_resolution_clock::now();

    //if (vertices.size() == 0)
        //vertices.reserve(16 * 16 * 16 * 8);
    stagingVertices.clear();

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 256; z++) {

                int block = GetBlock(x, y, z);
                if (block == 0)
                    continue;

                glm::ivec3 position(x, y, z);
                // Back face (−Y)
                if (y == 0) {
                    if (SouthNeighbor->GetBlock(x, 15, z) == 0) 
                        AddFace(backFace, position, 1, block);
                }
                else if (GetBlock(x, y - 1, z) == 0)
                    AddFace(backFace, position, 1, block);

                // Front face (+Y)
                if (y == 15) {
                    if (NorthNeighbor->GetBlock(x, 0, z) == 0)
                        AddFace(frontFace, position, 0, block);
                }
                else if (GetBlock(x, y + 1, z) == 0) {
                    AddFace(frontFace, position, 0, block);
                }

                // Left face (−X)
                if (x == 0) {
                    if (WestNeighbor && WestNeighbor->GetBlock(15, y, z) == 0)
                        AddFace(leftFace, position, 2, block);
                }
                else if (GetBlock(x - 1, y, z) == 0) {
                    AddFace(leftFace, position, 2, block);
                }

                // Right face (+X)
                if (x == 15) {
                    if (EastNeighbor && EastNeighbor->GetBlock(0, y, z) == 0)
                        AddFace(rightFace, position, 3, block);
                }
                else if (GetBlock(x + 1, y, z) == 0) {
                    AddFace(rightFace, position, 3, block);
                }

                // Bottom face (−Z)
                if (z == 0 || GetBlock(x, y, z - 1) == 0) {
                    AddFace(bottomFace, position, 4, block);
                }

                // Top face (+Z)
                if (z == 255 || GetBlock(x, y, z + 1) == 0) {
                    AddFace(topFace, position, 5, block);
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    //std::cout << "BuildMesh took " << duration << " ms\n";

    stagingVertices.shrink_to_fit();
    requiresRemesh.store(false);
    meshBuildQueued.store(false);
}

void Chunk::AddFace(const uint8_t(&face)[18], const glm::ivec3& position, uint8_t texIndex, uint8_t blockID) {
    for (int i = 0; i < 6; i++)
        stagingVertices.push_back(Vertex(
            face[i * 3] + position.x, face[i * 3 + 1] + position.y, face[i * 3 + 2] + position.z,
            texIndex, i, blockID
        ));
}

void Chunk::SetBlock(int x, int y, int z, int ID) {
    blocks[x * (16 * 256) + y * 256 + z] = ID;
}

void Chunk::UploadToGPU() {
    vertices.clear();
    vertices = std::move(stagingVertices);
    glGenVertexArrays(1, &MeshVAO);
    glGenBuffers(1, &MeshVBO);
    //bind VAO
    glBindVertexArray(MeshVAO);

    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

    //position attrbute
    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 6 * sizeof(uint8_t), (void*)0);
    glEnableVertexAttribArray(0);
    //face index
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 6 * sizeof(uint8_t), (void*)(3 * sizeof(uint8_t)));
    glEnableVertexAttribArray(1);
    //tex index
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 6 * sizeof(uint8_t), (void*)(4 * sizeof(uint8_t)));
    glEnableVertexAttribArray(2);
    //block id
    glVertexAttribIPointer(3, 1, GL_UNSIGNED_BYTE, 6 * sizeof(uint8_t), (void*)(5 * sizeof(uint8_t)));
    glEnableVertexAttribArray(3);

    glBindVertexArray(0);
}

void Chunk::ClearGPU() {
    //clear mesh gpu data when the chunk is deleted
    glDeleteBuffers(1, &MeshVBO);
    glDeleteVertexArrays(1, &MeshVAO);
}
