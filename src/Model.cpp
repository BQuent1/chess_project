#include "Model.hpp"
#include <iostream>
#include <limits>
#include <algorithm>
#include "stb_image.h"

#define TINYGLTF_IMPLEMENTATION
#define TINYGLTF_NO_STB_IMAGE_WRITE
#include "tiny_gltf.h"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <functional>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

#include <glad/glad.h>

bool Model::loadFromObj(const std::string& filepath) {
    tinyobj::ObjReaderConfig reader_config;
    size_t last_slash_idx = filepath.find_last_of("\\/");
    if (std::string::npos != last_slash_idx) {
        reader_config.mtl_search_path = filepath.substr(0, last_slash_idx + 1);
    } else {
        reader_config.mtl_search_path = "./";
    }

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

    float globMinX = std::numeric_limits<float>::max(); float globMinY = std::numeric_limits<float>::max(); float globMinZ = std::numeric_limits<float>::max();
    float globMaxX = -std::numeric_limits<float>::max(); float globMaxY = -std::numeric_limits<float>::max(); float globMaxZ = -std::numeric_limits<float>::max();

    auto& materials = reader.GetMaterials();

    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++) {
        std::vector<float> vertexData;
        float minX = std::numeric_limits<float>::max(); float minY = std::numeric_limits<float>::max(); float minZ = std::numeric_limits<float>::max();
        float maxX = -std::numeric_limits<float>::max(); float maxY = -std::numeric_limits<float>::max(); float maxZ = -std::numeric_limits<float>::max();

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

                minX = std::min(minX, vx); minY = std::min(minY, vy); minZ = std::min(minZ, vz);
                maxX = std::max(maxX, vx); maxY = std::max(maxY, vy); maxZ = std::max(maxZ, vz);

                globMinX = std::min(globMinX, vx); globMinY = std::min(globMinY, vy); globMinZ = std::min(globMinZ, vz);
                globMaxX = std::max(globMaxX, vx); globMaxY = std::max(globMaxY, vy); globMaxZ = std::max(globMaxZ, vz);

                vertexData.push_back(vx); vertexData.push_back(vy); vertexData.push_back(vz);

                if (idx.normal_index >= 0 && (3 * size_t(idx.normal_index) + 2) < attrib.normals.size()) {
                    tinyobj::real_t nx = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    tinyobj::real_t ny = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    tinyobj::real_t nz = attrib.normals[3 * size_t(idx.normal_index) + 2];
                    vertexData.push_back(nx); vertexData.push_back(ny); vertexData.push_back(nz);
                } else {
                    vertexData.push_back(0.0f); vertexData.push_back(1.0f); vertexData.push_back(0.0f);
                }

                if (idx.texcoord_index >= 0 && (2 * size_t(idx.texcoord_index) + 1) < attrib.texcoords.size()) {
                    tinyobj::real_t tx = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    tinyobj::real_t ty = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                    vertexData.push_back(tx);
                    vertexData.push_back(1.0f - ty); // Inverser l'axe Y pour OpenGL !
                } else {
                    vertexData.push_back(0.0f); vertexData.push_back(0.0f);
                }
            }
            index_offset += fv;
        }

        unsigned int textureID = 0;
        bool hasTexture = false;

        // On essaye de récupérer le material pour cette shape
        int material_id = shapes[s].mesh.material_ids.empty() ? -1 : shapes[s].mesh.material_ids[0];
        if (material_id >= 0 && material_id < materials.size()) {
            std::string diffuseTex = materials[material_id].diffuse_texname;
            if (!diffuseTex.empty()) {
                std::string texPath = reader_config.mtl_search_path + diffuseTex;
                
                int width, height, nrChannels;
                unsigned char *data = stbi_load(texPath.c_str(), &width, &height, &nrChannels, 0);
                if (data) {
                    glGenTextures(1, &textureID);
                    glBindTexture(GL_TEXTURE_2D, textureID);
                    
                    GLenum format = GL_RGB;
                    if (nrChannels == 4) format = GL_RGBA;
                    else if (nrChannels == 1) format = GL_RED;
                    
                    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
                    glGenerateMipmap(GL_TEXTURE_2D);
                    
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

                    stbi_image_free(data);
                    hasTexture = true;
                    std::cout << "Loaded texture: " << texPath << std::endl;
                } else {
                    std::cerr << "Failed to load texture: " << texPath << std::endl;
                }
            }
        }

        Mesh shapeMesh;
        shapeMesh.init(vertexData, textureID, hasTexture, glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
        _meshes.push_back(std::move(shapeMesh));

    }

    _aabbMin = glm::vec3(globMinX, globMinY, globMinZ);
    _aabbMax = glm::vec3(globMaxX, globMaxY, globMaxZ);

    return true;
}

static glm::mat4 getNodeTransform(const tinygltf::Node& node) {
    if (node.matrix.size() == 16) {
        return glm::make_mat4(node.matrix.data());
    }
    
    glm::mat4 m(1.0f);
    if (node.translation.size() == 3) {
        m = glm::translate(m, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
    }
    if (node.rotation.size() == 4) {
        glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
        m *= glm::mat4_cast(q);
    }
    if (node.scale.size() == 3) {
        m = glm::scale(m, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
    }
    return m;
}

bool Model::loadFromGLTF(const std::string& filepath) {
    tinygltf::Model model;
    tinygltf::TinyGLTF loader;
    std::string err, warn;

    bool ret = loader.LoadBinaryFromFile(&model, &err, &warn, filepath);
    if (!ret) {
        ret = loader.LoadASCIIFromFile(&model, &err, &warn, filepath);
    }

    if (!warn.empty()) std::cout << "TinyGLTF Warn: " << warn << std::endl;
    if (!err.empty()) std::cerr << "TinyGLTF Error: " << err << std::endl;
    if (!ret) return false;

    float globMinX = std::numeric_limits<float>::max(); float globMinY = std::numeric_limits<float>::max(); float globMinZ = std::numeric_limits<float>::max();
    float globMaxX = -std::numeric_limits<float>::max(); float globMaxY = -std::numeric_limits<float>::max(); float globMaxZ = -std::numeric_limits<float>::max();

    std::function<void(int, glm::mat4)> processNode = [&](int nodeIndex, glm::mat4 parentTransform) {
        const tinygltf::Node& node = model.nodes[nodeIndex];
        glm::mat4 globalTransform = parentTransform * getNodeTransform(node);
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(globalTransform)));

        if (node.mesh >= 0) {
            const tinygltf::Mesh& mesh = model.meshes[node.mesh];
            for (const auto& primitive : mesh.primitives) {
                std::vector<float> primitiveVertexData;
                float minX = std::numeric_limits<float>::max(); float minY = std::numeric_limits<float>::max(); float minZ = std::numeric_limits<float>::max();
                float maxX = -std::numeric_limits<float>::max(); float maxY = -std::numeric_limits<float>::max(); float maxZ = -std::numeric_limits<float>::max();
                
                const unsigned char* posBuffer = nullptr;
                const unsigned char* normBuffer = nullptr;
                const unsigned char* texBuffer = nullptr;
                size_t numPoints = 0;
                size_t posStride = 0, normStride = 0, texStride = 0;
                int posComp = -1, normComp = -1, texComp = -1;
                bool posNorm = false, normNorm = false, texNorm = false;

                auto getBufferData = [&](const std::string& attribStr, const unsigned char*& outBuf, size_t& outCount, size_t& outStride, int& outComp, bool& outNorm) {
                    auto it = primitive.attributes.find(attribStr);
                    if (it != primitive.attributes.end()) {
                        const tinygltf::Accessor& acc = model.accessors[it->second];
                        const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];
                        const tinygltf::Buffer& buf = model.buffers[view.buffer];
                        outBuf = &buf.data[view.byteOffset + acc.byteOffset];
                        outCount = acc.count;
                        outStride = acc.ByteStride(view);
                        outComp = acc.componentType;
                        outNorm = acc.normalized;
                    }
                };

                size_t dummy;
                getBufferData("POSITION", posBuffer, numPoints, posStride, posComp, posNorm);
                getBufferData("NORMAL", normBuffer, dummy, normStride, normComp, normNorm);
                getBufferData("TEXCOORD_0", texBuffer, dummy, texStride, texComp, texNorm);

                if (!posBuffer) continue;

                const void* indexBuffer = nullptr;
                int indexComponentType = -1;
                size_t numIndices = 0;

                if (primitive.indices >= 0) {
                    const tinygltf::Accessor& acc = model.accessors[primitive.indices];
                    const tinygltf::BufferView& view = model.bufferViews[acc.bufferView];
                    const tinygltf::Buffer& buf = model.buffers[view.buffer];
                    indexBuffer = &buf.data[view.byteOffset + acc.byteOffset];
                    indexComponentType = acc.componentType;
                    numIndices = acc.count;
                }

                auto readFloat = [](const unsigned char* p, int comp, bool norm, int index) -> float {
                    if (comp == TINYGLTF_COMPONENT_TYPE_FLOAT) return reinterpret_cast<const float*>(p)[index];
                    if (comp == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        float v = reinterpret_cast<const uint16_t*>(p)[index];
                        return norm ? v / 65535.0f : v;
                    }
                    if (comp == TINYGLTF_COMPONENT_TYPE_SHORT) {
                        float v = reinterpret_cast<const int16_t*>(p)[index];
                        return norm ? std::max(v / 32767.0f, -1.0f) : v;
                    }
                    if (comp == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                        float v = reinterpret_cast<const uint8_t*>(p)[index];
                        return norm ? v / 255.0f : v;
                    }
                    if (comp == TINYGLTF_COMPONENT_TYPE_BYTE) {
                        float v = reinterpret_cast<const int8_t*>(p)[index];
                        return norm ? std::max(v / 127.0f, -1.0f) : v;
                    }
                    return 0.0f;
                };

                auto addVertex = [&](size_t idx) {
                    glm::vec3 pos(
                        readFloat(posBuffer + idx * posStride, posComp, posNorm, 0),
                        readFloat(posBuffer + idx * posStride, posComp, posNorm, 1),
                        readFloat(posBuffer + idx * posStride, posComp, posNorm, 2)
                    );
                    pos = glm::vec3(globalTransform * glm::vec4(pos, 1.0f));

                    minX = std::min(minX, pos.x); minY = std::min(minY, pos.y); minZ = std::min(minZ, pos.z);
                    maxX = std::max(maxX, pos.x); maxY = std::max(maxY, pos.y); maxZ = std::max(maxZ, pos.z);
                    globMinX = std::min(globMinX, pos.x); globMinY = std::min(globMinY, pos.y); globMinZ = std::min(globMinZ, pos.z);
                    globMaxX = std::max(globMaxX, pos.x); globMaxY = std::max(globMaxY, pos.y); globMaxZ = std::max(globMaxZ, pos.z);

                    primitiveVertexData.push_back(pos.x); primitiveVertexData.push_back(pos.y); primitiveVertexData.push_back(pos.z);

                    if (normBuffer) {
                        glm::vec3 norm(
                            readFloat(normBuffer + idx * normStride, normComp, normNorm, 0),
                            readFloat(normBuffer + idx * normStride, normComp, normNorm, 1),
                            readFloat(normBuffer + idx * normStride, normComp, normNorm, 2)
                        );
                        norm = glm::normalize(normalMatrix * norm);
                        primitiveVertexData.push_back(norm.x); primitiveVertexData.push_back(norm.y); primitiveVertexData.push_back(norm.z);
                    } else {
                        primitiveVertexData.push_back(0.0f); primitiveVertexData.push_back(1.0f); primitiveVertexData.push_back(0.0f);
                    }

                    if (texBuffer) {
                        primitiveVertexData.push_back(readFloat(texBuffer + idx * texStride, texComp, texNorm, 0));
                        primitiveVertexData.push_back(readFloat(texBuffer + idx * texStride, texComp, texNorm, 1)); // Inversion Y
                    } else {
                        primitiveVertexData.push_back(0.0f); primitiveVertexData.push_back(0.0f);
                    }
                };

                if (numIndices > 0) {
                    for (size_t i = 0; i < numIndices; i++) {
                        size_t realIdx = 0;
                        if (indexComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                            realIdx = reinterpret_cast<const uint16_t*>(indexBuffer)[i];
                        } else if (indexComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                            realIdx = reinterpret_cast<const uint32_t*>(indexBuffer)[i];
                        } else if (indexComponentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE) {
                            realIdx = reinterpret_cast<const uint8_t*>(indexBuffer)[i];
                        }
                        addVertex(realIdx);
                    }
                } else {
                    for (size_t i = 0; i < numPoints; i++) {
                        addVertex(i);
                    }
                }

                unsigned int textureID = 0;
                bool hasTexture = false;

                if (primitive.material >= 0) {
                    const tinygltf::Material& mat = model.materials[primitive.material];
                    int texIndex = mat.pbrMetallicRoughness.baseColorTexture.index;
                    if (texIndex >= 0 && texIndex < static_cast<int>(model.textures.size())) {
                        const tinygltf::Texture& tex = model.textures[texIndex];
                        if (tex.source >= 0) {
                            const tinygltf::Image& image = model.images[tex.source];
                            
                            glGenTextures(1, &textureID);
                            glBindTexture(GL_TEXTURE_2D, textureID);
                            
                            GLenum format = GL_RGBA;
                            if (image.component == 3) format = GL_RGB;
                            else if (image.component == 1) format = GL_RED;
                            
                            glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, image.pixel_type, image.image.data());
                            glGenerateMipmap(GL_TEXTURE_2D);
                            
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
                            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
                            
                            hasTexture = true;
                            std::cout << "Loaded GLTF texture: " << image.name << std::endl;
                        }
                    }
                }

                Mesh primitiveMesh;
                primitiveMesh.init(primitiveVertexData, textureID, hasTexture, glm::vec3(minX, minY, minZ), glm::vec3(maxX, maxY, maxZ));
                _meshes.push_back(std::move(primitiveMesh));
            }
        }

        for (int child : node.children) {
            processNode(child, globalTransform);
        }
    };

    const tinygltf::Scene& defaultScene = model.scenes[model.defaultScene > -1 ? model.defaultScene : 0];
    for (int rootNode : defaultScene.nodes) {
        processNode(rootNode, glm::mat4(1.0f));
    }

    _aabbMin = glm::vec3(globMinX, globMinY, globMinZ);
    _aabbMax = glm::vec3(globMaxX, globMaxY, globMaxZ);

    return true;
}
