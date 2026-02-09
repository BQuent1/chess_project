#pragma once
#include <imgui.h>
// Inclusion des maths OpenGL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

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

    // Fonction utilitaire pour compiler un shader (on la cachera dans le .cpp)
    unsigned int compileShader(unsigned int type, const char* source);

public:
    Renderer3D();
    ~Renderer3D();

    void init(int width, int height);
    void render(int width, int height); // On lui passe la taille de la fenêtre ImGui

    // Pour qu'ImGui puisse afficher le résultat
    ImTextureID getTextureID() const { return (void*)(intptr_t)_textureColorBuffer; }
};