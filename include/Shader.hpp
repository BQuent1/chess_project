#pragma once

#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader {
public:
    unsigned int ID;

    // Constructeur qui parcourt, compile et lie le shader
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();

    // Active le shader
    void use() const;

    // Fonctions utilitaires pour envoyer des variables (Uniforms)
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, float value) const;
    void setVec2(const std::string &name, const glm::vec2 &value) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const;
    void setVec3(const std::string &name, float x, float y, float z) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;

private:
    void checkCompileErrors(unsigned int shader, std::string type);
};
