#include "Renderer3D.hpp"
#include "../include/ChessEngine.hpp"
#include <glad/glad.h>
#include <iostream>
#include "RandomGen.hpp"
#include "../include/stb_image.h"

static unsigned int loadTexture(const char* path) {
    unsigned int texID = 0;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int w, h, ch;
    stbi_set_flip_vertically_on_load(true);
    unsigned char* data = stbi_load(path, &w, &h, &ch, 0);
    if (data) {
        GLenum fmt = (ch == 4) ? GL_RGBA : GL_RGB;
        glTexImage2D(GL_TEXTURE_2D, 0, fmt, w, h, 0, fmt, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
        stbi_image_free(data);
    } else {
        std::cerr << "[Texture] Impossible de charger : " << path << std::endl;
    }
    return texID;
}

Renderer3D::Renderer3D() {}

Renderer3D::~Renderer3D()
{
    glDeleteVertexArrays(1, &_squareVAO);
    glDeleteBuffers(1, &_squareVBO);
    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(1, &_textureColorBuffer);
    glDeleteRenderbuffers(1, &_rbo);
    
    delete _boardShader;
    delete _skyboxShader;
}

// ================= INITIALISATION =================
void Renderer3D::init(int width, int height)
{
    // 1. Shaders
    _boardShader = new Shader("../../assets/shaders/board_square.vert", "../../assets/shaders/board_square.frag");
    _skyboxShader = new Shader("../../assets/shaders/skybox.vert", "../../assets/shaders/skybox.frag");

    // Setup de la Skybox avec ses textures
    _skybox.init(); // Initialize geometry after GL context is ready
    
    std::vector<std::string> faces = {
        "../../assets/skybox/right.png",
        "../../assets/skybox/left.png",
        "../../assets/skybox/top.png",
        "../../assets/skybox/bottom.png",
        "../../assets/skybox/front.png",
        "../../assets/skybox/back.png"
    };
    _skybox.loadCubemap(faces);

    // 2. Géométrie
    // géométrie d'un cube (pos + normales + UVs)
    float vertices[] = {
        // positions          // normales           // UVs
        // Face Arrière
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,    1.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,    1.0f, 1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,   0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,  0.0f, 0.0f,
        // Face Avant
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,    1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,     1.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,    0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,   0.0f, 0.0f,
        // Face Gauche
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,  1.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,   1.0f, 0.0f,
        // Face Droite
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,   1.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,  0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,   0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,    1.0f, 0.0f,
        // Face Bas
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,  1.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,   1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,  0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        // Face Haut
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,   1.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,    1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,   0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,  0.0f, 1.0f
    };


    glGenVertexArrays(1, &_squareVAO);
    glGenBuffers(1, &_squareVBO);

    glBindVertexArray(_squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // UVs
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);


    // 3. Setup du Framebuffer
    glGenFramebuffers(1, &_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo); // FIX : Pas de &

    glGenTextures(1, &_textureColorBuffer);
    glBindTexture(GL_TEXTURE_2D, _textureColorBuffer); // FIX : Pas de &
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, _textureColorBuffer, 0); // FIX : Pas de &

    glGenRenderbuffers(1, &_rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, _rbo); // FIX : Pas de &
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _rbo); // FIX : Pas de &

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // 4. Caméra
    _camera.init(width, height);

    loadModels();

    // 5. Textures du plateau
    _boardTexWhite = loadTexture("../../assets/textures/white tiles/stone_tiles_02_diff_1k.jpg");
    _boardTexBlack = loadTexture("../../assets/textures/black tiles/rabdentse_ruins_wall_diff_1k.jpg");

    // 6. Textures des pièces
    _pieceTexWhite = loadTexture("../../assets/textures/white pieces/plywood_diff_1k.jpg");
    _pieceTexBlack = loadTexture("../../assets/textures/black pieces/black_painted_planks_diff_1k.jpg");

    // === INITIALISATION DES PROBABILITES ===
    RandomGen::init();

    // Uniforme Continue : Teinte Ambiante
    _boardAmbientColor = glm::vec3(
        RandomGen::uniformContinuous(0.3f, 1.0f),
        RandomGen::uniformContinuous(0.3f, 1.0f),
        RandomGen::uniformContinuous(0.3f, 1.0f)
    );

    // Exponentielle : Prochain éclair (Moyenne = 10 secondes)
    _nextThunderTime = RandomGen::exponential(1.0 / 10.0);

    // Création des positions des sièges (4 groupes autour du plateau)
    _seatTransforms.clear();
    _spectatorSeatLocalTransforms.clear();

    // 4 gradins (gauche, droite, haut, bas)
    // Chaque gradin a 3 places disponibles dans le modèle, on va placer 2 modèles par côté = 8 gradins = 24 places
    for (int side = 0; side < 4; ++side) {
        for (int i = 0; i < 2; ++i) {
            glm::mat4 m = glm::mat4(1.0f);
            if (side == 0) { // X negatif
                m = glm::translate(m, glm::vec3(-2.0f, 0.0f, 2.0f + i * 4.0f));
                m = glm::rotate(m, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            } else if (side == 1) { // X positif
                m = glm::translate(m, glm::vec3(10.0f, 0.0f, 2.0f + i * 4.0f));
                m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            } else if (side == 2) { // Z negatif
                m = glm::translate(m, glm::vec3(2.0f + i * 4.0f, 0.0f, -2.0f));
            } else { // Z positif
                m = glm::translate(m, glm::vec3(2.0f + i * 4.0f, 0.0f, 10.0f));
                m = glm::rotate(m, glm::radians(180.0f), glm::vec3(0.0f, 1.0f, 0.0f));
            }
            // Echelle du gradin
            m = glm::scale(m, glm::vec3(0.5f));
            _seatTransforms.push_back(m);

            // Places relatives dans un objet 3D Seats (supposons 3 sièges espacés de 1.0 sur l'axe X)
            _spectatorSeatLocalTransforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.5f, 0.0f)));
            _spectatorSeatLocalTransforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.5f, 0.0f)));
            _spectatorSeatLocalTransforms.push_back(glm::translate(glm::mat4(1.0f), glm::vec3(1.0f, 0.5f, 0.0f)));
        }
    }

    int totalSeats = _seatTransforms.size() * _spectatorSeatLocalTransforms.size();

    // Variables Discrètes : Poisson + Permutation
    // Moyenne de 12 spectateurs autour du terrain
    _spectators.clear();
    int numSpectators = RandomGen::poisson(12.0);
    if (numSpectators > totalSeats) numSpectators = totalSeats;

    std::vector<int> seatPermutation = RandomGen::randomPermutation(totalSeats);
    for (int i = 0; i < numSpectators; ++i) {
        Spectator s;
        s.seatIndex = seatPermutation[i];
        _spectators.push_back(s);
    }
}

void Renderer3D::loadModels()
{
    // Load each piece type model. For now, we load what we have in assets/models.
    _assetManager.loadModel("pawn", "../../assets/models/pawn_model.obj");
    _assetManager.loadModel("knight", "../../assets/models/knight_model.obj");
    _assetManager.loadModel("bishop", "../../assets/models/bishop_model.obj");
    _assetManager.loadModel("rook", "../../assets/models/rook_model.obj");
    _assetManager.loadModel("queen", "../../assets/models/queen_model.obj");
    _assetManager.loadModel("king", "../../assets/models/king_model.obj");

    // Decors / Spectateurs
    _assetManager.loadModel("spectator", "../../assets/models/spectator_model/clean_mii.glb");
    _assetManager.loadModel("seats", "../../assets/models/normal_stadium_seats_v1_L1.123cda2b5658-a3c1-4927-86c0-def7dd9a8010/16949_Normal_Stadium_Seats_v1_NEW.obj");
}

// ================= RENDU =================
void Renderer3D::render(int width, int height, const ChessEngine& engine, int selectedX, int selectedY)
{
    if (_isChaosMode && !_randomPerPieceInit) {
        // -- Phase 3: Loi Binomiale --
        // Surbrillance magique. p=0.2 (20% de chances).
        int enchantedCount = RandomGen::binomial(32, 0.2); 
        
        // Liste tous les IDs de pieces présents
        std::vector<int> pieceIds;
        for (int i=0; i<8; ++i) {
            for (int j=0; j<8; ++j) {
                if (engine.plateau[i][j].has_value()) {
                    int pId = engine.plateau[i][j]->id;
                    pieceIds.push_back(pId);
                    
                    // -- Phase 4: Loi de Normale/Gaussienne --
                    // Petit décalage dans la pose de la pièce mu=0.0, sigma=0.05
                    double offset_x = RandomGen::normal(0.0, 0.08);
                    double offset_z = RandomGen::normal(0.0, 0.08);
                    _pieceOffsets[pId] = glm::vec2(offset_x, offset_z);
                }
            }
        }
        
        // Assigner les enchantements aléatoirement aux identifiants via Permutation
        std::vector<int> shuffledIdx = RandomGen::randomPermutation(pieceIds.size());
        for(int k=0; k < std::min(enchantedCount, (int)pieceIds.size()); ++k) {
            _enchantedPieces.insert(pieceIds[shuffledIdx[k]]);
        }
        _randomPerPieceInit = true;
    } else if (!_isChaosMode) {
        _randomPerPieceInit = false;
        _pieceOffsets.clear();
        _enchantedPieces.clear();
    }

    float currentFrame = ImGui::GetTime();
    float deltaTime = _lastFrameTime > 0.0f ? (currentFrame - _lastFrameTime) : 0.0f;
    _lastFrameTime = currentFrame;

    // Update Animation
    if (_currentAnim.active) {
        _currentAnim.progress += deltaTime / _currentAnim._duration;
        if (_currentAnim.progress >= 1.0f) {
            _currentAnim.active = false;
        }
    }

    glBindFramebuffer(GL_FRAMEBUFFER, _fbo); // FIX : Pas de &
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    _boardShader->use();

    unsigned int viewLoc = glGetUniformLocation(_boardShader->ID, "view");
    unsigned int projLoc = glGetUniformLocation(_boardShader->ID, "projection");
    unsigned int modelLoc = glGetUniformLocation(_boardShader->ID, "model");
    unsigned int colorLoc = glGetUniformLocation(_boardShader->ID, "squareColor");
    unsigned int viewPosLoc = glGetUniformLocation(_boardShader->ID, "uViewPos");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(_camera.getViewMatrix()));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(_camera.getProjectionMatrix()));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(_camera.getPosition()));

    // --- Illumination System ---
    float time = ImGui::GetTime();

    // -- Phase 4: Loi Uniforme (Modulation d'ambiance pure au hasard à chaque tour) --
    if (engine.current_player != _lastPlayer) {
        _lastPlayer = engine.current_player;
        _boardAmbientColor = glm::vec3(
            RandomGen::uniformContinuous(0.3f, 1.0f),
            RandomGen::uniformContinuous(0.3f, 1.0f),
            RandomGen::uniformContinuous(0.3f, 1.0f)
        );
    }

    // -- Phase 4: Loi Exponentielle (Flashs/Foudre) --
    _timeSinceLastThunder += deltaTime;
    if (_timeSinceLastThunder >= _nextThunderTime) {
        // Déclencher l'éclair
        _thunderIntensity = 3.0f; // Multiplicateur de lumière
        _timeSinceLastThunder = 0.0f;
        _nextThunderTime = RandomGen::exponential(1.0 / 8.0); // En moyenne toutes les 8s
    }
    // Décroissance rapide de l'éclair
    if (_thunderIntensity > 1.0f) {
        _thunderIntensity -= deltaTime * 8.0f;
    } else {
        _thunderIntensity = 1.0f;
    }

    // 1. Ambiance de base + aléatoire du tour + éclair
    glm::vec3 baseAmbianceColor = (engine.current_player == Color::Blanc) ? glm::vec3(0.8f, 0.75f, 0.6f) : glm::vec3(0.3f, 0.4f, 0.6f);
    glm::vec3 ambianceColor = baseAmbianceColor;
    if (_isChaosMode) {
        ambianceColor = _boardAmbientColor * baseAmbianceColor * _thunderIntensity;
    }
    glUniform3fv(glGetUniformLocation(_boardShader->ID, "ambientColor"), 1, glm::value_ptr(ambianceColor));

    // 2. Directional Light (Lune ou Soleil global)
    glUniform3f(glGetUniformLocation(_boardShader->ID, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
    glUniform3f(glGetUniformLocation(_boardShader->ID, "dirLight.color"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(_boardShader->ID, "dirLight.intensity"), 1.0f); // Augmenté

    // 3. Point Lights Mobiles
    float orbRadius = 6.0f;
    float orbSpeed = 0.5f;

    if (_isChaosMode) {
        // En mode chaos, pas de point lights, seulement ambiance + orages
        glUniform1i(glGetUniformLocation(_boardShader->ID, "numPointLights"), 0);
    } else {
        glUniform1i(glGetUniformLocation(_boardShader->ID, "numPointLights"), 2);
        if (engine.current_player == Color::Blanc) {
            _orbitalLightAngle += orbSpeed * deltaTime;
        }

        glm::vec3 posL1, posL2;
        glm::vec3 colL1, colL2;

        if (engine.current_player == Color::Blanc) {
            // Blancs : 2 lumières bleues qui bougent
            posL1 = glm::vec3(4.0f + cos(_orbitalLightAngle) * orbRadius, 3.0f, 4.0f + sin(_orbitalLightAngle) * orbRadius);
            posL2 = glm::vec3(4.0f + cos(_orbitalLightAngle + 3.14159f) * orbRadius, 3.0f, 4.0f + sin(_orbitalLightAngle + 3.14159f) * orbRadius);
            colL1 = glm::vec3(0.2f, 0.4f, 1.0f);
            colL2 = glm::vec3(0.2f, 0.4f, 1.0f);
        } else {
            // Noirs : 2 lumières rouges fixes
            posL1 = glm::vec3(4.0f + cos(0.0f) * orbRadius, 3.0f, 4.0f + sin(0.0f) * orbRadius);
            posL2 = glm::vec3(4.0f + cos(3.14159f) * orbRadius, 3.0f, 4.0f + sin(3.14159f) * orbRadius);
            colL1 = glm::vec3(1.0f, 0.1f, 0.1f);
            colL2 = glm::vec3(1.0f, 0.1f, 0.1f);
        }

        // Light 0
        glUniform3fv(glGetUniformLocation(_boardShader->ID, "pointLights[0].position"), 1, glm::value_ptr(posL1));
        glUniform3fv(glGetUniformLocation(_boardShader->ID, "pointLights[0].color"), 1, glm::value_ptr(colL1));
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[0].intensity"), 2.0f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[0].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[0].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[0].quadratic"), 0.032f);

        // Light 1
        glUniform3fv(glGetUniformLocation(_boardShader->ID, "pointLights[1].position"), 1, glm::value_ptr(posL2));
        glUniform3fv(glGetUniformLocation(_boardShader->ID, "pointLights[1].color"), 1, glm::value_ptr(colL2));
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[1].intensity"), 2.0f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[1].constant"), 1.0f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[1].linear"), 0.09f);
        glUniform1f(glGetUniformLocation(_boardShader->ID, "pointLights[1].quadratic"), 0.032f);
    }

    // ================= SOL DU PLATEAU =================
    glBindVertexArray(_squareVAO);

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // IMPORTANT: Bind tile VAO before drawing tile
            glBindVertexArray(_squareVAO);

            // Dans ta boucle de rendu pour le sol
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, glm::vec3((float)j + 0.5f, 0.0f, (float)i + 0.5f));

            // Ici on écrase le cube sur l'axe Y (0.1f) pour en faire une plaque
            model = glm::scale(model, glm::vec3(0.95f, 0.1f, 0.95f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            // Distinction visuelle Blanche/Noire tintée et Interactions
            glm::vec3 tileColor = ((i + j) % 2 == 0) ? glm::vec3(0.85f, 0.82f, 0.75f) : glm::vec3(0.18f, 0.20f, 0.22f);
            
            bool isHovered = (_camera.getHoveredX() == i && _camera.getHoveredY() == j);
            bool isSelected = (selectedX == i && selectedY == j);
            bool isPossible = (selectedX != -1 && engine.canIMove(selectedX, selectedY, i, j));

            if (isSelected) {
                // Highlight jaune pour la case sélectionnée
                tileColor = glm::mix(tileColor, glm::vec3(0.8f, 0.8f, 0.2f), 0.6f);
            } else if (isPossible) {
                // Highlight des mouvements possibles
                if (engine.plateau[i][j].has_value()) {
                    tileColor = glm::mix(tileColor, glm::vec3(0.8f, 0.2f, 0.2f), 0.6f); // Rouge si capture
                } else {
                    tileColor = glm::mix(tileColor, glm::vec3(0.2f, 0.8f, 0.2f), 0.6f); // Vert si vide
                }
            } else if (isHovered) {
                // Highlight leger pour le survol
                tileColor = glm::mix(tileColor, glm::vec3(0.5f, 0.8f, 1.0f), 0.4f);
            }

            glUniform3fv(colorLoc, 1, glm::value_ptr(tileColor));

            // Textures : cases blanches/noires (désactivé si highlight)
            bool isHighlighted = isSelected || isPossible || isHovered;
            bool isWhiteTile = ((i + j) % 2 == 0);
            unsigned int tileTexID = isWhiteTile ? _boardTexWhite : _boardTexBlack;

            if (!isHighlighted && tileTexID != 0) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, tileTexID);
                glUniform1i(glGetUniformLocation(_boardShader->ID, "diffuseTex"), 0);
                glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 1);
            } else {
                glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 0);
            }
            glDrawArrays(GL_TRIANGLES, 0, 36);


            // Logique d'état du jeu pour les pièces
            if (engine.plateau[i][j].has_value())
            {
                // Si la pièce courante est celle qui est en train de s'animer (à sa position finale), on ne la dessine pas encore.
                if (!(_currentAnim.active && i == _currentAnim.endX && j == _currentAnim.endY))
                {
                    const Piece& piece = engine.plateau[i][j].value();

                std::string modelStr = "";
                switch(piece.type) {
                    case PieceType::Pion: modelStr = "pawn"; break;
                    case PieceType::Tour: modelStr = "rook"; break;
                    case PieceType::Cavalier: modelStr = "knight"; break;
                    case PieceType::Fou: modelStr = "bishop"; break;
                    case PieceType::Reine: modelStr = "queen"; break;
                    case PieceType::Roi: modelStr = "king"; break;
                    default: break;
                }

                if (!modelStr.empty()) {
                    const Model* pModel = _assetManager.getModel(modelStr);
                    if (pModel != nullptr) {
                        const Model& model3D = *pModel;

                        // ====== CALCUL DE LA BOUNDING BOX ET CENTRAGE ======
                        glm::vec3 minAABB = model3D.getAABBMin();
                        glm::vec3 maxAABB = model3D.getAABBMax();
                        glm::vec3 centerAABB = (minAABB + maxAABB) * 0.5f;

                        // Scale et Rotation spécifiques selon la couleur
                        float scaleValue = 0.1f; // Maintenu à votre ajustement de 0.1
                        float rotAngleY = 0.0f;
                        float rotAngleX = glm::radians(-90.0f); // Pivoter de -90deg sur l'axe X (modèles orientés vers +Z)
                        float rotAngleZ = 0.0f;

                        if (piece.color == Color::Blanc) {
                            rotAngleY = glm::radians(180.0f);
                        } else {
                            rotAngleY = glm::radians(0.0f);
                        }

                        if (piece.type == PieceType::Cavalier) {
                            rotAngleY += glm::radians(180.0f); // Offset du cavalier
                        }

                        // 1. Matrice de transformation géométrique de base (Rotation + Scale)
                        glm::mat4 transform = glm::mat4(1.0f);
                        transform = glm::rotate(transform, rotAngleY, glm::vec3(0.0f, 1.0f, 0.0f));
                        transform = glm::rotate(transform, rotAngleX, glm::vec3(1.0f, 0.0f, 0.0f));
                        transform = glm::rotate(transform, rotAngleZ, glm::vec3(0.0f, 0.0f, 1.0f));
                        transform = glm::scale(transform, glm::vec3(scaleValue));

                        // 2. Trouver le point Y le plus bas du modèle une fois transformé
                        glm::vec3 corners[8] = {
                            glm::vec3(minAABB.x, minAABB.y, minAABB.z) - centerAABB,
                            glm::vec3(maxAABB.x, minAABB.y, minAABB.z) - centerAABB,
                            glm::vec3(minAABB.x, maxAABB.y, minAABB.z) - centerAABB,
                            glm::vec3(maxAABB.x, maxAABB.y, minAABB.z) - centerAABB,
                            glm::vec3(minAABB.x, minAABB.y, maxAABB.z) - centerAABB,
                            glm::vec3(maxAABB.x, minAABB.y, maxAABB.z) - centerAABB,
                            glm::vec3(minAABB.x, maxAABB.y, maxAABB.z) - centerAABB,
                            glm::vec3(maxAABB.x, maxAABB.y, maxAABB.z) - centerAABB
                        };

                        float lowestY = 999999.0f;
                        for(int k = 0; k < 8; ++k) {
                            glm::vec4 transCorner = transform * glm::vec4(corners[k], 1.0f);
                            if(transCorner.y < lowestY) {
                                lowestY = transCorner.y;
                            }
                        }

                        // 3. Assemblage final
                        glm::mat4 pieceModel = glm::mat4(1.0f);
                        // A. Translation pour poser la base sur la tuile à Y = 0.05
                        glm::vec2 offset = glm::vec2(0.0f);
                        if (_isChaosMode && _pieceOffsets.count(piece.id)) {
                            offset = _pieceOffsets[piece.id];
                        }
                        pieceModel = glm::translate(pieceModel, glm::vec3((float)j + 0.5f + offset.x, 0.05f - lowestY, (float)i + 0.5f + offset.y));
                        // B. Ajout des rotations et scales
                        pieceModel *= transform;
                        // C. Centrage sur la géométrie locale (on se remet à 0,0,0)
                        pieceModel = glm::translate(pieceModel, -centerAABB);

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pieceModel));

                        // Coloration + Texture des pièces
                        bool isPieceHighlighted = isSelected || isHovered;
                        glm::vec3 pieceColor;

                        if (isPieceHighlighted) {
                            // Highlight en couleur pure (pas de texture)
                            pieceColor = (piece.color == Color::Blanc) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.15f, 0.15f, 0.15f);
                            if (isSelected) {
                                pieceColor = glm::mix(pieceColor, glm::vec3(1.0f, 1.0f, 0.3f), 0.5f); // Glow Jaune
                            } else if (isHovered) {
                                pieceColor = glm::mix(pieceColor, glm::vec3(0.6f, 0.9f, 1.0f), 0.3f); // Glow Bleu ciel
                            }
                            glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));
                            glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 0);
                        } else {
                            // Texture + squareColor blanc pour ne pas teinter
                            unsigned int pieceTexID = (piece.color == Color::Blanc) ? _pieceTexWhite : _pieceTexBlack;
                            pieceColor = glm::vec3(1.0f);
                            glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));
                            if (pieceTexID != 0) {
                                glActiveTexture(GL_TEXTURE0);
                                glBindTexture(GL_TEXTURE_2D, pieceTexID);
                                glUniform1i(glGetUniformLocation(_boardShader->ID, "diffuseTex"), 0);
                                glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 1);
                            } else {
                                pieceColor = (piece.color == Color::Blanc) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.15f, 0.15f, 0.15f);
                                glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));
                                glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 0);
                            }
                        }

                        for(const auto& mesh : model3D._meshes) {
                            mesh.bind();
                            // Les .obj de pieces n'ont pas de texture propre, on skippe bindTexture
                            // pour ne pas écraser notre texture manuelle
                            glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
                            mesh.unbind();
                        }

                    }
                }
            }
        }
    }
}

    // ====== RENDU DE LA PIECE EN ANIMATION ======
    if (_currentAnim.active) {
        std::string modelName = "pawn";
        switch (_currentAnim.piece.type) {
            case PieceType::Pion: modelName = "pawn"; break;
            case PieceType::Tour: modelName = "rook"; break;
            case PieceType::Cavalier: modelName = "knight"; break;
            case PieceType::Fou: modelName = "bishop"; break;
            case PieceType::Reine: modelName = "queen"; break;
            case PieceType::Roi: modelName = "king"; break;
        }

        if (_assetManager.hasModel(modelName)) {
            const Model& model3D = *_assetManager.getModel(modelName);

            // Interpolation position (X, Z)
            float t = std::min(_currentAnim.progress, 1.0f);
            float currentX = glm::mix((float)_currentAnim.startY + 0.5f, (float)_currentAnim.endY + 0.5f, t);
            float currentZ = glm::mix((float)_currentAnim.startX + 0.5f, (float)_currentAnim.endX + 0.5f, t);

            // Interpolation Hauteur (Saut parabolique)
            // Parabole basique: 4 * h * t * (1 - t) où h = hauteur max
            float jumpHeight = 2.5f; 
            float currentY = 0.05f + (4.0f * jumpHeight * t * (1.0f - t));

            // -- Phase 3: Loi Géométrique --
            // Rotation 360 degres * (1 + le nombre d'echecs/vrilles bonus de la geom)
            float rotationAngle = t * glm::radians(360.0f * (1 + _currentAnimFlips));

            glm::mat4 animModel = glm::mat4(1.0f);
            
            // 1. Translation a la position (Y integre le saut)
            animModel = glm::translate(animModel, glm::vec3(currentX, currentY, currentZ));

            // 2. Rotation d'animation globale au monde axe x
            animModel = glm::rotate(animModel, rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));

            // 3. Application de l'orientation de base selon la couleur
            float baseRot = (_currentAnim.piece.color == Color::Blanc) ? 180.0f : 0.0f;
            if (_currentAnim.piece.type == PieceType::Cavalier) {
                baseRot += 180.0f;
            }
            animModel = glm::rotate(animModel, glm::radians(baseRot), glm::vec3(0.0f, 1.0f, 0.0f));
            
            // 4. Orientation du modèle (.obj file a x = forward, z = top)
            animModel = glm::rotate(animModel, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Modeles obj couchés

            // 5. Centrage AABB
            glm::vec3 aabbMin = model3D.getAABBMin();
            glm::vec3 aabbMax = model3D.getAABBMax();
            glm::vec3 center = (aabbMin + aabbMax) * 0.5f;
            animModel = glm::scale(animModel, glm::vec3(0.1f));
            animModel = glm::translate(animModel, -center);
            
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(animModel));

            // Coloration glow bleu très fort pendant le vol
            glm::vec3 pieceColor = (_currentAnim.piece.color == Color::Blanc) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.15f, 0.15f, 0.15f);
            pieceColor = glm::mix(pieceColor, glm::vec3(0.3f, 1.0f, 0.8f), 0.7f); // Magic glow
            glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));

            for(const auto& mesh : model3D._meshes) {
                mesh.bind();
                mesh.bindTexture(_boardShader->ID);
                glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
                mesh.unbind();
            }
        }
    }

    glBindVertexArray(_squareVAO);

    // ====== RENDU DES BORDURES DE L'ECHIQUIER ======
    glm::vec3 borderColor(0.3f, 0.15f, 0.05f); // Bois sombre
    glUniform3fv(colorLoc, 1, glm::value_ptr(borderColor));

    struct BorderDef { glm::vec3 pos; glm::vec3 scale; };
    BorderDef borders[4] = {
        { glm::vec3(-0.1f, 0.1f, 4.0f), glm::vec3(0.2f, 0.3f, 8.4f) }, // Gauche
        { glm::vec3( 8.1f, 0.1f, 4.0f), glm::vec3(0.2f, 0.3f, 8.4f) }, // Droite
        { glm::vec3( 4.0f, 0.1f, -0.1f), glm::vec3(8.0f, 0.3f, 0.2f) }, // Haut
        { glm::vec3( 4.0f, 0.1f,  8.1f), glm::vec3(8.0f, 0.3f, 0.2f) }  // Bas
    };

    for(int i=0; i<4; ++i) {
        glm::mat4 bModel = glm::mat4(1.0f);
        bModel = glm::translate(bModel, borders[i].pos);
        bModel = glm::scale(bModel, borders[i].scale);
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(bModel));
        glUniform1i(glGetUniformLocation(_boardShader->ID, "hasTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    // ====== RENDU DES GRADINS & SPECTATEURS (Poisson + Permutation) ======
    if (_isChaosMode && _assetManager.hasModel("seats") && _assetManager.hasModel("spectator")) {
        const Model& seatModel = *_assetManager.getModel("seats");
        const Model& specModel = *_assetManager.getModel("spectator");
        
        // Couleur neutre / bleue pour les sièges
        glm::vec3 seatColor(0.2f, 0.4f, 0.7f);
        glUniform3fv(colorLoc, 1, glm::value_ptr(seatColor));
        for(const auto& mesh : seatModel._meshes) {
            mesh.bind();
            mesh.bindTexture(_boardShader->ID);

            for (const auto& sTrans : _seatTransforms) {
                glm::mat4 m = glm::scale(sTrans, glm::vec3(0.02f)); // Ajustement d'echelle empirique
                m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation pour ne pas faire face au sol
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
            }
            mesh.unbind();
        }

        glm::vec3 specColor(1.0f, 1.0f, 1.0f);
        glUniform3fv(colorLoc, 1, glm::value_ptr(specColor));
        glm::vec3 specAabbMin = specModel.getAABBMin();
        glm::vec3 specAabbMax = specModel.getAABBMax();
        glm::vec3 specCenter = (specAabbMin + specAabbMax) * 0.5f;

        for(const auto& mesh : specModel._meshes) {
            mesh.bind();
            mesh.bindTexture(_boardShader->ID);
            
            for (const auto& s : _spectators) {
                int gradinIndex = s.seatIndex / _spectatorSeatLocalTransforms.size();
                int localSeatIndex = s.seatIndex % _spectatorSeatLocalTransforms.size();

                if (gradinIndex >= _seatTransforms.size()) continue;

                glm::mat4 baseTrans = _seatTransforms[gradinIndex];
                glm::mat4 localTrans = _spectatorSeatLocalTransforms[localSeatIndex];

                // On combine
                glm::mat4 thisSpecModel = baseTrans * localTrans;

                float baseHeightOffset = 0.5f; 
                
                float time = (float)ImGui::GetTime();
                float jumpSpeed = 6.0f; 
                float jumpHeight = 0.5f; 
                
                float jumpAnimOscillation = std::abs(sin(time * jumpSpeed + (float)s.seatIndex * 0.5f)); 
                
                float finalOffset = baseHeightOffset + (jumpAnimOscillation * jumpHeight);
                thisSpecModel = glm::translate(thisSpecModel, glm::vec3(0.0f, finalOffset, 0.0f));

                thisSpecModel = glm::scale(thisSpecModel, glm::vec3(0.02f)); // Echelle du Pikmin
                
                thisSpecModel = glm::translate(thisSpecModel, -specCenter);

                
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(thisSpecModel));
                glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
            }
            mesh.unbind();
        }
    }

    glBindVertexArray(0);

    // ====== RENDU DU NUAGE DE PARTICULES (Binomiale) ======
    _particleSystem.updateAndRender(deltaTime, _isChaosMode, engine, _enchantedPieces, *_boardShader, _squareVAO);

    // Rendu de la Skybox en dernier pour optimisation avec LEQUAL
    _skybox.draw(_skyboxShader->ID, _camera.getViewMatrix(), _camera.getProjectionMatrix());

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer3D::triggerAnimation(int startX, int startY, int endX, int endY, Piece p) {
    _currentAnim.active = true;
    _currentAnim.startX = startX;
    _currentAnim.startY = startY;
    _currentAnim.endX = endX;
    _currentAnim.endY = endY;
    _currentAnim.progress = 0.0f;
    _currentAnim.piece = p;
    
    // Probabilité p = 0.4 de "succès" (donc d'arrêter de vriller en l'air).
    if (_isChaosMode) {
        _currentAnimFlips = RandomGen::geometric(0.4);
    } else {
        _currentAnimFlips = 0;
    }
}

