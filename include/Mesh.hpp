#pragma once

#include <string>
#include <vector>
#include <glm/glm.hpp>

class Mesh {
public:
    Mesh() = default;
    ~Mesh();

    // Prevent copying because of OpenGL resources
    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    // Allow moving
    Mesh(Mesh&& other) noexcept;
    Mesh& operator=(Mesh&& other) noexcept;

    bool loadFromObj(const std::string& filepath);
    void bind() const;
    void unbind() const;
    unsigned int getVertexCount() const { return _vertexCount; }

    glm::vec3 getAABBMin() const;
    glm::vec3 getAABBMax() const;

private:
    unsigned int _vao = 0;
    unsigned int _vbo = 0;
    unsigned int _vertexCount = 0;

    glm::vec3 _aabbMin = glm::vec3(0.0f);
    glm::vec3 _aabbMax = glm::vec3(0.0f);
};
