#include "Mesh.hpp"
#include <glad/glad.h>
#include <iostream>
#include <limits>
#include <algorithm>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

Mesh::~Mesh() {
    if (_vao != 0) {
        glDeleteVertexArrays(1, &_vao);
    }
    if (_vbo != 0) {
        glDeleteBuffers(1, &_vbo);
    }
}

Mesh::Mesh(Mesh&& other) noexcept
    : _vao(other._vao), _vbo(other._vbo), _vertexCount(other._vertexCount) {
    other._vao = 0;
    other._vbo = 0;
    other._vertexCount = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept {
    if (this != &other) {
        if (_vao != 0) glDeleteVertexArrays(1, &_vao);
        if (_vbo != 0) glDeleteBuffers(1, &_vbo);

        _vao = other._vao;
        _vbo = other._vbo;
        _vertexCount = other._vertexCount;
        _aabbMin = other._aabbMin;
        _aabbMax = other._aabbMax;

        other._vao = 0;
        other._vbo = 0;
        other._vertexCount = 0;
    }
    return *this;
}

bool Mesh::loadFromObj(const std::string& filepath) {
    tinyobj::ObjReaderConfig reader_config;
    reader_config.mtl_search_path = ""; // Don't care about materials for now

    tinyobj::ObjReader reader;

    if (!reader.ParseFromFile(filepath, reader_config)) {
        if (!reader.Error().empty()) {
            std::cerr << "TinyObjReader error: " << reader.Error();
        }
        return false;
    }

    if (!reader.Warning().empty()) {
        std::cout << "TinyObjReader warning: " << reader.Warning();
    }

    auto& attrib = reader.GetAttrib();
    auto& shapes = reader.GetShapes();

    std::vector<float> vertexData;

    float minX = std::numeric_limits<float>::max();
    float minY = std::numeric_limits<float>::max();
    float minZ = std::numeric_limits<float>::max();
    float maxX = -std::numeric_limits<float>::max();
    float maxY = -std::numeric_limits<float>::max();
    float maxZ = -std::numeric_limits<float>::max();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);

            // Loop over vertices in the face.
            for (size_t v = 0; v < fv; v++) {
                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];
                tinyobj::real_t vx = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                tinyobj::real_t vy = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                tinyobj::real_t vz = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                minX = std::min(minX, vx);
                minY = std::min(minY, vy);
                minZ = std::min(minZ, vz);
                maxX = std::max(maxX, vx);
                maxY = std::max(maxY, vy);
                maxZ = std::max(maxZ, vz);

                // Append position
                vertexData.push_back(vx);
                vertexData.push_back(vy);
                vertexData.push_back(vz);

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    
                    vertexData.push_back(nx);
                    vertexData.push_back(ny);
                    vertexData.push_back(nz);
                } else {
                    // Default normal
                    vertexData.push_back(0.0f);
                    vertexData.push_back(1.0f);
                    vertexData.push_back(0.0f);
                }
            }
            index_offset += fv;
        }
    }

    _aabbMin = glm::vec3(minX, minY, minZ);
    _aabbMax = glm::vec3(maxX, maxY, maxZ);

    _vertexCount = vertexData.size() / 6;

    if (_vao == 0) glGenVertexArrays(1, &_vao);
    if (_vbo == 0) glGenBuffers(1, &_vbo);

    glBindVertexArray(_vao);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, vertexData.size() * sizeof(float), vertexData.data(), GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
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
