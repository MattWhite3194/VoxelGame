#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include "Shader.h"
#include "Camera.h"
#include "Texture.h"
#include "Chunk.hpp"
#include <unordered_map>
#include "ThreadPool.h"
#include <queue>

//GLFW for window management
//Glad for initializing opengl functions with gpu driver
//glm for linear algebra
Camera camera;

//Only one thread to prevent thread oversaturation - leads to extremely slow execution and stuttering due to synchronization and constant memory allocations.
ThreadPool generationPool(1);
ThreadPool meshingPool(1);
ThreadPool worldUpdatePool(1);
double lastX = 0.0, lastY = 0.0;
bool firstMouse = true;
float deltaTime = 0.0f;
float lastFrame = 0.0;
int viewportWidth = 1000, viewportHeight = 1000;
glm::mat4 Projection = glm::perspective(glm::radians(70.0f),
    (float)viewportWidth / (float)viewportHeight,
    0.1f, 1000.0f);
int renderDistance = 24;
std::mutex cleanupMutex;
std::mutex worldChunksMutex;
std::vector<Chunk*> cleanupQueue;
std::queue<Chunk*> meshUploadQueue;
std::atomic<bool> running{ true };
const int maxUploadsPerFrame = 5;


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


void CheckChunksForDeletion() {
    while (running.load()) {
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        std::vector<std::pair<glm::ivec2, Chunk*>> pairs;
        //minize lock time, get snapshot of valid keys
        {
            std::lock_guard<std::mutex> lock(worldChunksMutex);
            if (worldChunks.empty()) continue;
            pairs.reserve(worldChunks.size());
            for (const auto& pair : worldChunks)
                pairs.push_back(pair);
        }
        //std::cout << pairs.size();
        for (const auto& pair : pairs) {
            //std::cout << "Checking chunk" << std::endl;
            Chunk* chunk = pair.second;
            if (!chunk)
                continue;
            //don't queue the chunk if it was already scheduled, or if it hasn't finished loading
            if (chunk->scheduledForDeletion.load() || !chunk->uploadComplete.load())
                continue;
            //std::cout << glm::distance(chunk->position, glm::vec2(camera.Position / 16.0f)) << std::endl;
            if (glm::distance(chunk->position, glm::vec2(camera.Position / 16.0f)) > renderDistance + 2) {
                chunk->scheduledForDeletion.store(true);
                std::lock_guard<std::mutex> lock(cleanupMutex);
                cleanupQueue.push_back(chunk);
            }
        }
    }
}

void ProcessChunkCleanup() {
    std::lock_guard<std::mutex> lock(cleanupMutex);
    for (Chunk* chunk : cleanupQueue) {
        worldChunks.erase(chunk->position);
        delete chunk;
        std::cout << "Chunk Deleted" << std::endl;
    }
    cleanupQueue.clear();
}

void ProcessMeshUpload() {
    int numUploads = 0;
    while (meshUploadQueue.size() > 0) {
        Chunk* chunk = meshUploadQueue.front();
        if (!chunk->meshBuildQueued.load()) {
            meshUploadQueue.pop();
            chunk->UploadToGPU();
            chunk->uploadComplete.store(true);
        }
        numUploads++;
        if (numUploads >= maxUploadsPerFrame)
            break;
    }
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

    //create first 9 chunks that the player is standing on
    for (int x = -1; x <= 1; x++) {
        for (int y = -1; y <= 1; y++) {
            glm::ivec2 position(x, y);
            worldChunks[position] = new Chunk();
            Chunk* newChunk = worldChunks[position];
            newChunk->position = position;
            newChunk->Generate();
        }
    }
    camera.Position = glm::vec3(0.0f, 0.0f, 80.0f);
    //main window loop
    worldUpdatePool.enqueue(CheckChunksForDeletion);
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
        blockShader.setFloat("fadeStartDistance", renderDistance * 16 - 20);

        //loop through chunks in spiral starting from player position
        int x = 0;
        int y = 0;
        int dx = 0;
        int dy = -1;

        //Free any chunks in cleanup buffer
        ProcessMeshUpload();
        ProcessChunkCleanup();
        for (int i = 0; i < (renderDistance * 2) * (renderDistance * 2); i++) {
            std::lock_guard<std::mutex> lock(worldChunksMutex);
            glm::ivec2 position(camera.Position.x / 16.0f + x, camera.Position.y / 16.0f + y);
            if (std::sqrt(x * x + y * y) <= renderDistance) {
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
                    else if (chunk->requiresRemesh.load()) {
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
                        meshingPool.enqueue([chunk] {
                            chunk->BuildMesh();
                            });
                        chunk->meshBuildQueued.store(true);
                        chunk->requiresRemesh.store(false);
                        meshUploadQueue.push(chunk);
                    }
                }
                else {
                    //If chunk is nullptr, create the chunk and add it's generation to the generation ThreadPool
                    worldChunks[position] = new Chunk();
                    Chunk* newChunk = worldChunks[position];
                    newChunk->position = position;
                    generationPool.enqueue([newChunk] {
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

        glfwPollEvents();
        glfwSwapBuffers(window);
    }
    running.store(false);
    generationPool.join();
    meshingPool.join();
    worldUpdatePool.join();
    glfwTerminate();
    return 0;
}
