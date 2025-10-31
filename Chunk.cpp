#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Use triangle strips to only have 8 vertices per chunk
//On generation, create a vertex buffer of the vertices in the geometry, that way only one draw call is needed to render the entire chunk
//check each block and it's surrounding face to determine which vertices to add

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
            for (int z = 255; z >= 0; z--) {
                //for now just fill blocks below 60
                if (z <= 10 + x + y)
                    blocks[x * (16 * 256) + y * 256 + z] = 1;
            }
        }
    }
}

void Chunk::Render(Shader& shader) {
    shader.use();
    glm::mat4 model(1.0f);
    model = glm::translate(model, glm::vec3(position, 0.0f));
    shader.setMat4("model", model);
    glBindVertexArray(MeshVAO);
    glDrawArrays(GL_TRIANGLES, 0, vertices.size() / 3);
    glBindVertexArray(0);
}

void Chunk::BuildMesh() {
    if (!MeshVAO)
        glGenVertexArrays(1, &MeshVAO);
    if (!MeshVBO)
        glGenBuffers(1, &MeshVBO);

    if (vertices.size() == 0)
        //reserve 16x16x20 blocks of 72 vertices
        vertices.reserve(sizeof(float) * 16 * 16 * 20 * 72);
    else
        vertices.clear();
    //loop through blocks and build mesh
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 256; z++) {
                if (GetBlock(x, y, z) != 0) {
                    //x increasing -> going away from screen
                    //y increasing -> going to the right
                    //z increasing -> going up

                    if (y == 0 || GetBlock(x, y - 1, z) == 0) {
                        int i = 0;
                        for (auto v : backFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(1);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                    if (y == 15 || GetBlock(x, y + 1, z) == 0) {
                        int i = 0;
                        for (auto v : frontFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(0);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                    if (x == 0 || GetBlock(x - 1, y, z) == 0) {
                        int i = 0;
                        for (auto v : leftFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(2);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                    if (x == 15 || GetBlock(x + 1, y, z) == 0) {
                        int i = 0;
                        for (auto v : rightFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(3);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                    if (z == 0 || GetBlock(x, y, z - 1) == 0) {
                        int i = 0;
                        for (auto v : bottomFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(4);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                    if (z == 255 || GetBlock(x, y, z + 1) == 0) {
                        int i = 0;
                        for (auto v : topFace) {
                            vertices.insert(vertices.end(), { GLubyte(v.x + x), GLubyte(v.y + y), GLubyte(v.z + z) });
                            vertices.push_back(5);
                            vertices.push_back(i);
                            i++;
                        }
                    }
                }
            }
        }
    }
    //bind VAO
    glBindVertexArray(MeshVAO);

    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, MeshVBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLubyte), vertices.data(), GL_STATIC_DRAW);

    //position attrbute
    glVertexAttribPointer(0, 3, GL_UNSIGNED_BYTE, GL_FALSE, 5 * sizeof(GLubyte), (void*)0);
    glEnableVertexAttribArray(0);
    //face index
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_BYTE, 5 * sizeof(GLubyte), (void*)(3 * sizeof(GLubyte)));
    glEnableVertexAttribArray(1);
    //tex index
    glVertexAttribIPointer(2, 1, GL_UNSIGNED_BYTE, 5 * sizeof(GLubyte), (void*)(4 * sizeof(GLubyte)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

int Chunk::GetBlock(int x, int y, int z) {
    return blocks[x * (16 * 256) + y * 256 + z];
}