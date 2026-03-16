#pragma once
#include <imgui.h>
// Inclusion des maths OpenGL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <string>
#include "Mesh.hpp"

class ChessEngine; // forward declaration

class Renderer3D {
private:
    // --- Identifiants OpenGL ---
    unsigned int _fbo                = 0; // Framebuffer (le canevas)
    unsigned int _textureColorBuffer = 0; // La texture finale
    unsigned int _rbo                = 0; // Renderbuffer (pour la profondeur/Z-buffer)

    unsigned int _shaderProgram = 0;             // Le programme combinant Vertex + Fragment shaders
    unsigned int _squareVAO = 0, _squareVBO = 0; // Les données géométriques du carré

    // --- Caméra ---
    glm::mat4 _projection;
    glm::mat4 _view;

    float     _yaw      = 90.0f;
    float     _pitch    = 45.0f;
    float     _distance = 12.0f;
    glm::vec3 _target   = glm::vec3(4.0f, 0.0f, 4.0f);
    glm::vec3 _camPos;

    // --- Modèles 3D ---
    std::map<std::string, Mesh> _pieceModels;

    // Fonction utilitaire pour compiler un shader (on la cachera dans le .cpp)
    unsigned int compileShader(unsigned int type, const char* source);

public:
    Renderer3D();
    ~Renderer3D();

    void init(int width, int height);
    void loadModels();
    
    void render(int width, int height, const ChessEngine& engine);
    void updateCamera();
    void updateViewMatrix();

    ImTextureID getTextureID() const { return (void*)(intptr_t)_textureColorBuffer; }
};