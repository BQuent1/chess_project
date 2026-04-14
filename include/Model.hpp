#pragma once

#include <vector>
#include <string>
#include <glm/glm.hpp>
#include "Mesh.hpp"

class Model {
public:
    Model() = default;
    ~Model() = default;

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    Model(Model&& other) noexcept = default;
    Model& operator=(Model&& other) noexcept = default;

    bool loadFromObj(const std::string& filepath);
    bool loadFromGLTF(const std::string& filepath);

    glm::vec3 getAABBMin() const { return _aabbMin; }
    glm::vec3 getAABBMax() const { return _aabbMax; }

    std::vector<Mesh> _meshes;

private:
    glm::vec3 _aabbMin = glm::vec3(0.0f);
    glm::vec3 _aabbMax = glm::vec3(0.0f);
};
