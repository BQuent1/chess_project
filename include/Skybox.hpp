#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>

class Skybox {
public:
    Skybox();
    ~Skybox();

    void init();
    void loadCubemap(const std::vector<std::string>& faces);
    void draw(unsigned int shaderProgram, const glm::mat4& view, const glm::mat4& projection);

private:
    unsigned int _textureID = 0;
    unsigned int _vao = 0;
    unsigned int _vbo = 0;
};
