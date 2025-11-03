#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>

//Use triangle strips to only have 8 vertices per chunk
//On generation, create a vertex buffer of the vertices in the geometry, that way only one draw call is needed to render the entire chunk
//check each block and it's surrounding face to determine which vertices to add

SimplexNoise Chunk::terrainNoise(0.1f);

glm::u8vec3 frontFace[] = {
    glm::u8vec3(0, 1, 0),  // v0 bottom-left
    glm::u8vec3(1, 1, 0),  // v1 bottom-right
    glm::u8vec3(0, 1, 1),  // v2 top-left

    glm::u8vec3(0, 1, 1),  // v2 top-left
    glm::u8vec3(1, 1, 0),  // v1 bottom-right
    glm::u8vec3(1, 1, 1),  // v3 top-right
};

glm::u8vec3 backFace[] = {
    glm::u8vec3(1, 0, 0),  // v0 bottom-right
    glm::u8vec3(0, 0, 0),  // v1 bottom-left
    glm::u8vec3(1, 0, 1),  // v2 top-right

    glm::u8vec3(1, 0, 1),  // v2 top-right
    glm::u8vec3(0, 0, 0),  // v1 bottom-left
    glm::u8vec3(0, 0, 1),  // v3 top-left
};

glm::u8vec3 leftFace[] = {
    glm::u8vec3(0, 0, 0),  // v0 bottom-back
    glm::u8vec3(0, 1, 0),  // v1 bottom-front
    glm::u8vec3(0, 0, 1),  // v2 top-back

    glm::u8vec3(0, 0, 1),  // v2 top-back
    glm::u8vec3(0, 1, 0),  // v1 bottom-front
    glm::u8vec3(0, 1, 1),  // v3 top-front
};

glm::u8vec3 rightFace[] = {
    glm::u8vec3(1, 1, 0),  // v0 bottom-front
    glm::u8vec3(1, 0, 0),  // v1 bottom-back
    glm::u8vec3(1, 1, 1),  // v2 top-front

    glm::u8vec3(1, 1, 1),  // v2 top-front
    glm::u8vec3(1, 0, 0),  // v1 bottom-back
    glm::u8vec3(1, 0, 1),  // v3 top-back
};

glm::u8vec3 bottomFace[] = {
    glm::u8vec3(0, 0, 0),  // v0 back-left
    glm::u8vec3(1, 0, 0),  // v1 back-right
    glm::u8vec3(0, 1, 0),  // v2 front-left

    glm::u8vec3(0, 1, 0),  // v2 front-left
    glm::u8vec3(1, 0, 0),  // v1 back-right
    glm::u8vec3(1, 1, 0),  // v3 front-right
};

glm::u8vec3 topFace[] = {
    glm::u8vec3(0, 1, 1),  // v0 front-left
    glm::u8vec3(1, 1, 1),  // v1 front-right
    glm::u8vec3(0, 0, 1),  // v2 back-left

    glm::u8vec3(0, 0, 1),  // v2 back-left
    glm::u8vec3(1, 1, 1),  // v1 front-right
    glm::u8vec3(1, 0, 1)   // v3 back-right
};

void Chunk::Generate() {
	blocks = std::vector<int>(16 * 16 * 256, 0);
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            int height = (int)(30 * terrainNoise.fractal(3, x / 16.0f + (float)position.x, y / 16.0f + (float)position.y));
            for (int z = 40 + height; z >= 0; z--) {
                /*Blocks
                * 3 grass
                * 2 dirt
                * 1 stone
                */
                if (z == 40 + height)
                    SetBlock(x, y, z, 3);
                else if (z > height + 36)
                    SetBlock(x, y, z, 2);
                else
                    SetBlock(x, y, z, 1);
            }
        }
    }
    generated = true;
}

void Chunk::Render(Shader& shader) {
    if (!MeshVAO || !MeshVBO) {
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

    shader.use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position * 16.0f, 0.0f));
    shader.setMat4("model", model);
    glBindVertexArray(MeshVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    glBindVertexArray(0);
}

void Chunk::BuildMesh() {
    // Reserve space for all possible faces once
    auto start = std::chrono::high_resolution_clock::now();

    if (vertices.size() == 0)
        vertices.reserve(16 * 16 * 16 * 8);
    vertices.clear();

    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 256; z++) {

                int block = GetBlock(x, y, z);
                if (block == 0)
                    continue;

                // Back face (−Y)
                if (y == 0 || GetBlock(x, y - 1, z) == 0) {
                    int i = 0;
                    for (auto& v : backFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            1, uint8_t(i++), uint8_t(block)
                            ));
                }

                // Front face (+Y)
                if (y == 15 || GetBlock(x, y + 1, z) == 0) {
                    int i = 0;
                    for (auto& v : frontFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            0, uint8_t(i++), uint8_t(block)
                            ));
                }

                // Left face (−X)
                if (x == 0 || GetBlock(x - 1, y, z) == 0) {
                    int i = 0;
                    for (auto& v : leftFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            2, uint8_t(i++), uint8_t(block)
                            ));
                }

                // Right face (+X)
                if (x == 15 || GetBlock(x + 1, y, z) == 0) {
                    int i = 0;
                    for (auto& v : rightFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            3, uint8_t(i++), uint8_t(block)
                            ));
                }

                // Bottom face (−Z)
                if (z == 0 || GetBlock(x, y, z - 1) == 0) {
                    int i = 0;
                    for (auto& v : bottomFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            4, uint8_t(i++), uint8_t(block)
                            ));
                }

                // Top face (+Z)
                if (z == 255 || GetBlock(x, y, z + 1) == 0) {
                    int i = 0;
                    for (auto& v : topFace)
                        vertices.emplace_back(Vertex(
                            uint8_t(v.x + x), uint8_t(v.y + y), uint8_t(v.z + z),
                            5, uint8_t(i++), uint8_t(block)
                            ));
                }
            }
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    //std::cout << "BuildMesh took " << duration << " ms\n";
    meshed = true;
    meshBuildQueued = false;
}

void Chunk::SetBlock(int x, int y, int z, int ID) {
    blocks[x * (16 * 256) + y * 256 + z] = ID;
}

void Chunk::ClearGPU() {
    //clear mesh gpu data when the chunk is deleted
    glDeleteBuffers(1, &MeshVBO);
    glDeleteVertexArrays(1, &MeshVAO);
}