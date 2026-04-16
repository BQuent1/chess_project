#pragma once
#include <imgui.h>
// Inclusion des maths OpenGL
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <map>
#include <set>
#include <string>
#include <memory>
#include "Camera.hpp"
#include "Shader.hpp"
#include "Model.hpp"
#include "AssetManager.hpp"
#include "ParticleSystem.hpp"
#include "Skybox.hpp"
#include "Piece.hpp"

class ChessEngine; // forward declaration

class Renderer3D {
private:
    // --- Identifiants OpenGL ---
    unsigned int _fbo                = 0; // Framebuffer (le canevas)
    unsigned int _textureColorBuffer = 0; // La texture finale
    unsigned int _rbo                = 0; // Renderbuffer (pour la profondeur/Z-buffer)

    Shader* _boardShader = nullptr;
    unsigned int _squareVAO = 0, _squareVBO = 0; // Les données géométriques du carré

    // --- Skybox ---
    Skybox _skybox;
    Shader* _skyboxShader = nullptr;

    // --- Caméra ---
    Camera _camera;

    // --- Modèles 3D ---
    AssetManager _assetManager;



    // --- Animation ---
    struct MoveAnimation {
        bool  active = false;
        int   startX, startY;
        int   endX, endY;
        float progress = 0.0f; // 0.0 to 1.0
        float _duration = 0.8f; // seconds
        Piece piece;
    } _currentAnim;

    float _lastFrameTime = 0.0f;
    bool _isChaosMode = false;
    bool _randomPerPieceInit = false;

    // --- Variables Aléatoires (Projet Probas/Stats) ---
    // Poisson + Permutation
    struct Spectator {
        int seatIndex;
    };
    std::vector<Spectator> _spectators;
    std::vector<glm::mat4> _seatTransforms;
    std::vector<glm::mat4> _spectatorSeatLocalTransforms;

    // Uniforme Continue
    glm::vec3 _boardAmbientColor;
    Color _lastPlayer = Color::Blanc;
    
    // NORMALE Gaussienne
    std::map<int, glm::vec2> _pieceOffsets;

    // Exponentielle
    float _timeSinceLastThunder = 0.0f;
    float _nextThunderTime = 0.0f;
    float _thunderIntensity = 0.0f;

    // Weibull
    std::map<int, float> _pieceFatigue; // ID ou hash (i*8+j) -> distance max restante

    // Binomiale
    std::set<int> _enchantedPieces;

    // Particules pour Binomiale
    // Particules pour Binomiale
    ParticleSystem _particleSystem;

    // Animation Variables
    int _currentAnimFlips = 0; // Number of flips from Geometric Law
    float _orbitalLightAngle = 0.0f;

public:
    Renderer3D();
    ~Renderer3D();

    int getHoveredX() const { return _camera.getHoveredX(); }
    int getHoveredY() const { return _camera.getHoveredY(); }
    void setChaosMode(bool chaos) { _isChaosMode = chaos; }
    void resetChaosState() { _randomPerPieceInit = false; _enchantedPieces.clear(); _pieceOffsets.clear(); }

    void init(int width, int height);
    void loadModels();
    
    void render(int width, int height, const ChessEngine& engine, int selectedX = -1, int selectedY = -1);
    void updateCamera() { _camera.updateInputs(); }
    void updateRaycast(float mouseX, float mouseY, int screenWidth, int screenHeight) {
        _camera.updateRaycast(mouseX, mouseY, screenWidth, screenHeight);
    }

    void triggerAnimation(int startX, int startY, int endX, int endY, Piece p);

    void setFpsMode(bool enabled, glm::vec3 pos = glm::vec3(0.0f)) {
        _camera.setFpsMode(enabled, pos);
    }

    ImTextureID getTextureID() const { return (void*)(intptr_t)_textureColorBuffer; }
};