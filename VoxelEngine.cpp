#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "ChunkManager.h"
#include <queue>

//GLFW for window management
//Glad for initializing opengl functions with gpu driver
//glm for linear algebra
Camera camera;

double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0;
int viewportWidth = 1000, viewportHeight = 1000;
glm::mat4 Projection = glm::perspective(glm::radians(70.0f),
    (float)viewportWidth / (float)viewportHeight,
    0.1f, 1000.0f);
std::unique_ptr<ChunkManager> chunkManager;

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
    Texture textureAtlas("terrain.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    textureAtlas.texUnit(blockShader, "TextureAtlas", 0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    
    camera.Position = glm::vec3(0.0f, 0.0f, 80.0f);
    chunkManager = std::make_unique<ChunkManager>();
    chunkManager->Init();
    //main window loop
    
    while (!glfwWindowShouldClose(window)) {
        //calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        //std::cout << 1.0f / deltaTime << std::endl;
        process_input(window);
        glClearColor(0.0f, 0.54f, 0.84f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        textureAtlas.Bind();
        blockShader.setMat4("projection", Projection);
        blockShader.setMat4("view", camera.GetViewMatrix());
        blockShader.setVec3("CameraPos", camera.Position);
        blockShader.setFloat("fadeStartDistance", chunkManager->RenderDistance * 16 - 20);

        chunkManager->Update(camera.Position, blockShader);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    chunkManager->Terminate();
    glfwTerminate();
    return 0;
}
