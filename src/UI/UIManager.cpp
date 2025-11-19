#include "UI/UIManager.h"
#include <iostream>
#include <glm/gtx/transform.hpp>

void UIManager::Initialize(int viewportWidth, int viewportHeight) {
    this->viewportWidth = viewportWidth;
    this->viewportHeight = viewportHeight;
	//upload plane to gpu
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    //bind VAO
    glBindVertexArray(VAO);

    //bind VBO
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 24 * sizeof(float), _vertices, GL_STATIC_DRAW);

    //position attrbute
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    //texcoord attribute
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);
}

void UIManager::Update(Shader& shader, const glm::mat4& projection) {
    glDisable(GL_DEPTH_TEST);
	for (auto c : _components) {
		shader.setMat4("Transform", c->transformMatrix);
        c->texture->texUnit(shader, "Texture", 0);
        c->texture->Bind();
		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}
}

void UIManager::AddUIComponent(std::shared_ptr<UIComponent> component) {
    component->UpdateAnchorMatrix(viewportWidth, viewportHeight);
    _components.push_back(std::move(component));
}

void UIManager::OnViewportResized(int viewportWidth, int viewportHeight) {
    this->viewportWidth = viewportWidth;
    this->viewportHeight = viewportHeight;
    for (auto c : _components) {
        c->UpdateAnchorMatrix(viewportWidth, viewportHeight);
    }
}