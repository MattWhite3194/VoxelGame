#pragma once
#include <glm/gtc/type_ptr.hpp>
#include <string>


class Shader
{
public:
    unsigned int ID;
    Shader(const char* vertexPath, const char* fragmentPath);
    void use();
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec2(const std::string& name, glm::vec2 vector2);
    void setVec3(const std::string& name, glm::vec3 vector3);
    void setVec4(const std::string& name, glm::vec4 vector4);
    void setMat4(const std::string& name, glm::mat4 matrix4);

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};