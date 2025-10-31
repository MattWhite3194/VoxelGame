#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Chunk.hpp"
#include <unordered_map>

//GLFW for window management
//Glad for initializing opengl functions with gpu driver
//glm for linear algebra
Camera camera;
double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0;
int viewportWidth = 1000, viewportHeight = 1000;
glm::mat4 Projection = glm::perspective(glm::radians(51.0f),
    (float)viewportWidth / (float)viewportHeight,
    0.1f, 1000.0f);


struct IVec2Hash {
    size_t operator()(const glm::ivec2& v) const noexcept {
        return (std::hash<int>()(v.x) ^ (std::hash<int>()(v.y) << 1));
    }
};
std::unordered_map<glm::ivec2, Chunk*, IVec2Hash> worldChunks;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    viewportWidth = width;
    viewportHeight = height;
    Projection = glm::perspective(glm::radians(51.0f),
        (float)viewportWidth / (float)viewportHeight,
        0.1f, 1000.0f);
    glViewport(0, 0, width, height);
}

void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouse) {
        lastX = xpos;
        lastY = ypos;
        firstMouse = false;
    }

    double xoffset = xpos - lastX;
    double yoffset = lastY - ypos;
    camera.ProcessMouseMovement(xoffset, yoffset);

    lastX = xpos;
    lastY = ypos;
}

void process_input(GLFWwindow* window) {
    float cameraSpeed = static_cast<float>(2.5 * deltaTime);
    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.ProcessKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.ProcessKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.ProcessKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.ProcessKeyboard(RIGHT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        camera.ProcessKeyboard(UP, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        camera.ProcessKeyboard(DOWN, deltaTime);
}

int main()
{
    //Set up glfw window and load glad
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(1000, 1000, "Minecraft", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate;
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    //attach callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    Shader blockShader("block.vert", "block.frag");
    Texture textureAtlas("Blocks.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    textureAtlas.texUnit(blockShader, "TextureAtlas", 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);

    //initialize chunks in 16 x 16 grid (16 render distance)
    for (int x = -8; x <= 8; x++) {
        for (int y = -8; y <= 8; y++) {
            glm::ivec2 position(x, y);
            worldChunks[position] = new Chunk();
            Chunk* newChunk = worldChunks[position];
            newChunk->Generate();
            newChunk->BuildMesh();
            newChunk->position = position;
        }
    }
    //main window loop
    while (!glfwWindowShouldClose(window)) {
        //calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        std::cout << 1.0f / deltaTime << std::endl;
        process_input(window);
        glClearColor(0.4f, 0.55f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        textureAtlas.Bind();
        blockShader.setMat4("projection", Projection);
        blockShader.setMat4("view", camera.GetViewMatrix());
        //chunk loop
        for (auto c : worldChunks) {
            c.second->Render(blockShader);
        }
            
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    glfwTerminate();
    return 0;
}
