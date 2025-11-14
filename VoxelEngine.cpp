#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Player.h"
#include "Texture.h"
#include "ChunkManager.h"
#include "PhysicsEngine.h"
#include <queue>

//GLFW for window management
//Glad for initializing opengl functions with gpu driver
//glm for linear algebra
double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
double deltaTime = 0.0;
double lastFrame = 0.0;
int viewportWidth = 1000, viewportHeight = 1000;
glm::mat4 Projection = glm::perspective(glm::radians(70.0f),
    (float)viewportWidth / (float)viewportHeight,
    0.1f, 1000.0f);
std::shared_ptr<ChunkManager> chunkManager;
std::shared_ptr<Player> player;
std::unique_ptr<PhysicsEngine> physicsEngine;

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
    player->ProcessMouseMovement(xoffset, yoffset);

    lastX = xpos;
    lastY = ypos;
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    player->HandleKeyboardInput(window, key, scancode, action, mods);
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
    glfwSetKeyCallback(window, key_callback);

    Shader blockShader("block.vert", "block.frag");
    Texture textureAtlas("terrain.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    textureAtlas.texUnit(blockShader, "TextureAtlas", 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    player = std::make_shared<Player>(glm::vec3(5.0f, 5.0f, 100.0f));
    chunkManager = std::make_shared<ChunkManager>();
    physicsEngine = std::make_unique<PhysicsEngine>(player, chunkManager);
    //main window loop
    
    while (!glfwWindowShouldClose(window)) {
        //calculate delta time
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        physicsEngine->Update(deltaTime);
        glClearColor(0.0f, 0.54f, 0.84f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        textureAtlas.Bind();
        blockShader.setMat4("projection", Projection);
        blockShader.setMat4("view", player->GetView());
        blockShader.setVec3("CameraPos", player->GetPosition());
        blockShader.setFloat("fadeStartDistance", chunkManager->RenderDistance * 16 - 20);
        chunkManager->Update(player->GetPosition(), blockShader);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    chunkManager->Terminate();
    glfwTerminate();
    return 0;
}
