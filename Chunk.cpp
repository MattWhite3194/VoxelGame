#include "Chunk.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

//Use triangle strips to only have 8 vertices per chunk
//On generation, create a vertex buffer of the vertices in the geometry, that way only one draw call is needed to render the entire chunk
//check each block and it's surrounding face to determine which vertices to add
void Chunk::Generate() {
	blocks = std::vector<int>(16 * 16 * 256, 0);
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 255; z >= 0; z--) {
                //for now just fill blocks below 60
                if (z <= 10)
                    blocks[x * (16 * 256) + y * 256 + z] = 1;
            }
        }
    }
}

void Chunk::Render(Shader& shader) {
    for (int x = 0; x < 16; x++) {
        for (int y = 0; y < 16; y++) {
            for (int z = 0; z < 256; z++) {
                glm::mat4 model = glm::mat4(1.0f); // make sure to initialize matrix to identity matrix first
                model = glm::translate(model, glm::vec3(x, y, z));
                shader.setMat4("model", model);
                if (blocks[x * (16 * 256) + y * 256 + z] != 0)
                    glDrawArrays(GL_TRIANGLE_STRIP, 0, 36);
            }
        }
    }
}