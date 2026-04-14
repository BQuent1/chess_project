#include "Mesh.hpp"
#include <glad/glad.h>
#include <iostream>
#include <limits>
#include <algorithm>

Mesh::~Mesh() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
    }
    if (_textureID != 0) {
        glDeleteTextures(1, &_textureID);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
    : _vao(other._vao), _vbo(other._vbo), _textureID(other._textureID), _vertexCount(other._vertexCount), _hasTexture(other._hasTexture) {
    other._vao = 0;
    other._vbo = 0;
    other._textureID = 0;
    other._vertexCount = 0;
    other._hasTexture = false;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (_vao != 0) glDeleteVertexArrays(1, &_vao);
        if (_vbo != 0) glDeleteBuffers(1, &_vbo);
        if (_textureID != 0) glDeleteTextures(1, &_textureID);

        _vao = other._vao;
        _vbo = other._vbo;
        _textureID = other._textureID;
        _vertexCount = other._vertexCount;
        _hasTexture = other._hasTexture;
        _aabbMin = other._aabbMin;
        _aabbMax = other._aabbMax;

        other._vao = 0;
        other._vbo = 0;
        other._textureID = 0;
        other._vertexCount = 0;
        other._hasTexture = false;
    }
    return *this;
}

void Mesh::init(const std::vector<float>& vertexData, unsigned int textureID, bool hasTexture, glm::vec3 aabbMin, glm::vec3 aabbMax) {
    _aabbMin = aabbMin;
    _aabbMax = aabbMax;
    _vertexCount = vertexData.size() / 8;
    _textureID = textureID;
    _hasTexture = hasTexture;

    if (_vao == 0) glGenVertexArrays(1, &_vao);
    if (_vbo == 0) glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

void Mesh::bind() const {
    glBindVertexArray(_vao);
}

void Mesh::unbind() const {
    glBindVertexArray(0);
}

glm::vec3 Mesh::getAABBMin() const {
    return _aabbMin;
}

glm::vec3 Mesh::getAABBMax() const {
    return _aabbMax;
}

void Mesh::bindTexture(unsigned int shaderProgram) const {
    if (_hasTexture && _textureID != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, _textureID);
        glUniform1i(glGetUniformLocation(shaderProgram, "diffuseTex"), 0);
        glUniform1i(glGetUniformLocation(shaderProgram, "hasTexture"), 1);
    } else {
        glUniform1i(glGetUniformLocation(shaderProgram, "hasTexture"), 0);
    }
}
