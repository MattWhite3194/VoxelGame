#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Chunk.hpp"

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
GLuint blockVBO, blockVAO, blockEBO;
Chunk* firstChunk;

//TODO: do not need to store normal or texture coordinates, will always be the same for every block, calculate in the shader
float vertices[] = {
    // --- Front face (z = +0.5)
    -0.5f, -0.5f,  0.5f,   // bottom-left
     0.5f, -0.5f,  0.5f,   // bottom-right
    -0.5f,  0.5f,  0.5f,   // top-left
     0.5f,  0.5f,  0.5f,   // top-right

     // --- Back face (z = -0.5)
      0.5f, -0.5f, -0.5f,   // bottom-right
     -0.5f, -0.5f, -0.5f,   // bottom-left
      0.5f,  0.5f, -0.5f,   // top-right
     -0.5f,  0.5f, -0.5f,   // top-left

     // --- Left face (x = -0.5)
     -0.5f, -0.5f, -0.5f,   // bottom-back
     -0.5f, -0.5f,  0.5f,   // bottom-front
     -0.5f,  0.5f, -0.5f,   // top-back
     -0.5f,  0.5f,  0.5f,   // top-front

     // --- Right face (x = +0.5)
      0.5f, -0.5f,  0.5f,   // bottom-front
      0.5f, -0.5f, -0.5f,   // bottom-back
      0.5f,  0.5f,  0.5f,   // top-front
      0.5f,  0.5f, -0.5f,   // top-back

      // --- Bottom face (y = -0.5)
      -0.5f, -0.5f, -0.5f,   // back-left
       0.5f, -0.5f, -0.5f,   // back-right
      -0.5f, -0.5f,  0.5f,   // front-left
       0.5f, -0.5f,  0.5f,   // front-right

       // --- Top face (y = +0.5)
       -0.5f,  0.5f,  0.5f,   // front-left
        0.5f,  0.5f,  0.5f,   // front-right
       -0.5f,  0.5f, -0.5f,   // back-left
        0.5f,  0.5f, -0.5f    // back-right
};

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

    //Set up cube data for rendering
    //generate buffers 
    glGenVertexArrays(1, &blockVAO);
    glGenBuffers(1, &blockVBO);

    //bind VAO
    glBindVertexArray(blockVAO);

    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, blockVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    //position attrbute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    //normal attrbute
    //glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3* sizeof(float)));
    //glEnableVertexAttribArray(1);

    //glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    //glEnableVertexAttribArray(2);

    //texture attribute

    glBindVertexArray(0);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);

    firstChunk = new Chunk();
    firstChunk->Generate();

    //main window loop
    while (!glfwWindowShouldClose(window)) {
        //calculate delta time
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        process_input(window);
        glClearColor(0.4f, 0.55f, 0.8f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        textureAtlas.Bind();
        blockShader.setMat4("projection", Projection);
        blockShader.setMat4("view", camera.GetViewMatrix());
        glBindVertexArray(blockVAO);
        //chunk loop
        firstChunk->Render(blockShader);
            
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    delete(firstChunk);
    glfwTerminate();
    return 0;
}
