#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "OpenGL/Shader.h"
#include "Entities/Player.h"
#include "OpenGL/Texture.h"
#include "World/ChunkManager.h"
#include "Physics/PhysicsEngine.h"
#include "UI/UIManager.h"
#include "UI/UIComponent.h"
#include <queue>

//GLFW for window management
//Glad for initializing opengl functions with gpu driver
//glm for linear algebra
double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
double deltaTime = 0.0;
double lastFrame = 0.0;
int viewportWidth = 1000, viewportHeight = 600;
float fov = 70.0f;
glm::mat4 Projection = glm::perspective(glm::radians(fov),
    (float)viewportWidth / (float)viewportHeight,
    0.1f, 1000.0f);
glm::mat4 UIProjection = glm::ortho(0.0f, (float)viewportWidth, 0.0f, (float)viewportHeight, -1.0f, 1.0f);
std::shared_ptr<ChunkManager> chunkManager;
std::shared_ptr<Player> player;
std::unique_ptr<PhysicsEngine> physicsEngine;
std::unique_ptr<UIManager> uiManager;

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    //Pressing windows key or exiting the screen while in fullscreen sends 0 width and height, which breaks projection matrix
    if (height == 0)
        return;
    viewportWidth = width;
    viewportHeight = height;
    Projection = glm::perspective(glm::radians(fov),
        (float)viewportWidth / (float)viewportHeight,
        0.1f, 1000.0f); 
    UIProjection = glm::ortho(0.0f, (float)viewportWidth, 0.0f, (float)viewportHeight, -1.0f, 1.0f);
    uiManager->OnViewportResized(viewportWidth, viewportHeight);
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

void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    //get world-space mouse dir
    glm::vec4 rayClip(0, 0, -1.0f, 1.0f);

    glm::vec4 rayEye = glm::inverse(Projection) * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);

    glm::vec3 dir = glm::normalize(
        glm::vec3(glm::inverse(player->GetView()) * rayEye)
    );
    player->ProcessMouseInput(window, button, action, mods, chunkManager.get(), dir);
}

int main()
{
    player = std::make_shared<Player>(glm::vec3(5.0f, 5.0f, 200.0f));
    chunkManager = std::make_shared<ChunkManager>();
    physicsEngine = std::make_unique<PhysicsEngine>(player, chunkManager);
    uiManager = std::make_unique<UIManager>();

    //Set up glfw window and load glad
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 4);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow* window = glfwCreateWindow(viewportWidth, viewportHeight, "PHYSICS", NULL, NULL);
    if (window == NULL) {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    gladLoadGL();

    //attach callback functions
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_pos_callback);
    glfwSetKeyCallback(window, key_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    
    //Block Shader with single texture atlas
    Shader blockShader("res/shaders/block.vert", "res/shaders/block.frag");
    Texture textureAtlas("res/textures/terrain.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    textureAtlas.texUnit(blockShader, "TextureAtlas", 0);
    
    //UI Shader
    Shader uiShader("res/shaders/ui.vert", "res/shaders/ui.frag");

    //UI Textures
    std::shared_ptr<Texture> crosshairTexture = std::make_shared<Texture>("res/textures/crosshair.png", GL_TEXTURE_2D, GL_TEXTURE0, GL_RGBA, GL_UNSIGNED_BYTE);
    std::shared_ptr<UIComponent> crosshairComponent = std::make_shared<UIComponent>(crosshairTexture, glm::vec2(0.0f), glm::vec2(20.0f), Anchor::MiddleMiddle);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
     
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CW);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //Fullscreen
    //GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    //const GLFWvidmode* mode = glfwGetVideoMode(monitor);
    //glfwSetWindowMonitor(window, monitor, 0, 0, mode->width, mode->height, GLFW_DONT_CARE);

    //initialize UI manager with gpu
    uiManager->Initialize(viewportWidth, viewportHeight);
    uiManager->AddUIComponent(crosshairComponent);
    
    //main window loop
    std::cout << glm::to_string(UIProjection);
    while (!glfwWindowShouldClose(window)) {
        //calculate delta time
        double currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        physicsEngine->Update(deltaTime);
        glClearColor(0.0f, 0.54f, 0.84f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //Update and render chunks
        blockShader.use();
        textureAtlas.Bind();
        blockShader.setMat4("projection", Projection);
        blockShader.setMat4("view", player->GetView());
        blockShader.setVec3("CameraPos", player->GetPosition());
        blockShader.setFloat("fadeStartDistance", chunkManager->RenderDistance * 16 - 20);
        //chunkManager->Update(player->GetPosition(), blockShader);

        //Update and render UI
        uiShader.use();
        uiShader.setMat4("Projection", UIProjection);
        uiManager->Update(uiShader, UIProjection);
        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    chunkManager->Terminate();
    glfwTerminate();
    return 0;
}
