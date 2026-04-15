#include "Renderer3D.hpp"
#include "../include/ChessEngine.hpp"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include "RandomGen.hpp"

std::string loadShaderSource(const char* filepath)
{
    std::ifstream     file(filepath);
    std::stringstream buffer;
    if (file.is_open())
    {
        buffer << file.rdbuf();
        return buffer.str();
    }
    std::cerr << "ERREUR::SHADER::FICHIER_NON_TROUVE: " << filepath << std::endl;
    return "";
}

Renderer3D::Renderer3D() {}

Renderer3D::~Renderer3D()
{
    glDeleteVertexArrays(1, &_squareVAO);
    glDeleteBuffers(1, &_squareVBO);
    glDeleteFramebuffers(1, &_fbo);
    glDeleteTextures(1, &_textureColorBuffer);
    glDeleteRenderbuffers(1, &_rbo);
    glDeleteProgram(_shaderProgram);
    glDeleteProgram(_skyboxShaderProgram);
}

unsigned int Renderer3D::compileShader(unsigned int type, const char* source)
{
    unsigned int id = glCreateShader(type);
    glShaderSource(id, 1, &source, nullptr);
    glCompileShader(id);

    int  success;
    char infoLog[512];
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(id, 512, nullptr, infoLog);
        std::cerr << "ERREUR::SHADER::COMPILATION_FAILED\n"
                  << infoLog << std::endl;
    }
    return id;
}

// ================= INITIALISATION =================
void Renderer3D::init(int width, int height)
{
    // 1. Shaders
    std::string vShaderStr  = loadShaderSource("../../assets/shaders/board_square.vert");
    std::string fShaderStr  = loadShaderSource("../../assets/shaders/board_square.frag");
    const char* vShaderCode = vShaderStr.c_str();
    const char* fShaderCode = fShaderStr.c_str();

    unsigned int vertex   = compileShader(GL_VERTEX_SHADER, vShaderCode);
    unsigned int fragment = compileShader(GL_FRAGMENT_SHADER, fShaderCode);

    _shaderProgram = glCreateProgram();
    glAttachShader(_shaderProgram, vertex);
    glAttachShader(_shaderProgram, fragment);
    glLinkProgram(_shaderProgram);
    glDeleteShader(vertex);
    glDeleteShader(fragment);

    // Skybox Shader
    std::string skyboxVShaderStr = loadShaderSource("../../assets/shaders/skybox.vert");
    std::string skyboxFShaderStr = loadShaderSource("../../assets/shaders/skybox.frag");
    const char* skyboxVShaderCode = skyboxVShaderStr.c_str();
    const char* skyboxFShaderCode = skyboxFShaderStr.c_str();

    unsigned int skyboxVertex = compileShader(GL_VERTEX_SHADER, skyboxVShaderCode);
    unsigned int skyboxFragment = compileShader(GL_FRAGMENT_SHADER, skyboxFShaderCode);

    _skyboxShaderProgram = glCreateProgram();
    glAttachShader(_skyboxShaderProgram, skyboxVertex);
    glAttachShader(_skyboxShaderProgram, skyboxFragment);
    glLinkProgram(_skyboxShaderProgram);
    glDeleteShader(skyboxVertex);
    glDeleteShader(skyboxFragment);

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
    float vertices[] = {
        // positions          // normales
        // Face Arrière
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, 0.0f, -1.0f,
        // Face Avant
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, 0.0f, 1.0f,
        // Face Gauche
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, -1.0f, 0.0f, 0.0f,
        // Face Droite
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 1.0f, 0.0f, 0.0f,
        // Face Bas
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, 0.5f, 0.0f, -1.0f, 0.0f,
        -0.5f, -0.5f, -0.5f, 0.0f, -1.0f, 0.0f,
        // Face Haut
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, 0.5f, 0.0f, 1.0f, 0.0f,
        -0.5f, 0.5f, -0.5f, 0.0f, 1.0f, 0.0f
    };


    glGenVertexArrays(1, &_squareVAO);
    glGenBuffers(1, &_squareVBO);

    glBindVertexArray(_squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    // glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // normales
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

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
    updateViewMatrix();
    _projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    // _view       = glm::lookAt(glm::vec3(4.0f, 10.0f, 12.0f), glm::vec3(4.0f, 0.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));

    loadModels();

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
    _pieceModels["pawn"].loadFromObj("../../assets/models/pawn_model.obj");
    _pieceModels["knight"].loadFromObj("../../assets/models/knight_model.obj");
    _pieceModels["bishop"].loadFromObj("../../assets/models/bishop_model.obj");
    _pieceModels["rook"].loadFromObj("../../assets/models/rook_model.obj");
    _pieceModels["queen"].loadFromObj("../../assets/models/queen_model.obj");
    _pieceModels["king"].loadFromObj("../../assets/models/king_model.obj");

    // Decors / Spectateurs
    _pieceModels["spectator"].loadFromGLTF("../../assets/models/spectator_model/clean_mii.glb");
    _pieceModels["seats"].loadFromObj("../../assets/models/normal_stadium_seats_v1_L1.123cda2b5658-a3c1-4927-86c0-def7dd9a8010/16949_Normal_Stadium_Seats_v1_NEW.obj");
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

    glUseProgram(_shaderProgram);

    unsigned int viewLoc = glGetUniformLocation(_shaderProgram, "view");
    unsigned int projLoc = glGetUniformLocation(_shaderProgram, "projection");
    unsigned int modelLoc = glGetUniformLocation(_shaderProgram, "model");
    unsigned int colorLoc = glGetUniformLocation(_shaderProgram, "squareColor");
    unsigned int viewPosLoc = glGetUniformLocation(_shaderProgram, "uViewPos");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(_view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(_projection));
    glUniform3fv(viewPosLoc, 1, glm::value_ptr(_camPos));

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
    glUniform3fv(glGetUniformLocation(_shaderProgram, "ambientColor"), 1, glm::value_ptr(ambianceColor));

    // 2. Directional Light (Lune ou Soleil global)
    glUniform3f(glGetUniformLocation(_shaderProgram, "dirLight.direction"), -0.2f, -1.0f, -0.3f);
    glUniform3f(glGetUniformLocation(_shaderProgram, "dirLight.color"), 1.0f, 1.0f, 1.0f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "dirLight.intensity"), 0.6f);

    // 3. Point Light Mobile (Orbite autour du plateau central X=4, Z=4)
    float orbRadius = 6.0f;
    float orbSpeed = 0.5f;
    glm::vec3 pointPos(4.0f + cos(time * orbSpeed) * orbRadius, 3.0f, 4.0f + sin(time * orbSpeed) * orbRadius);
    glUniform3fv(glGetUniformLocation(_shaderProgram, "pointLight.position"), 1, glm::value_ptr(pointPos));
    
    // Couleur de la lumière mobile qui pulse légerement
    float pulse = (sin(time * 2.0f) + 1.0f) * 0.5f;
    glm::vec3 pointColor = glm::mix(glm::vec3(1.0f, 0.5f, 0.0f), glm::vec3(1.0f, 0.8f, 0.2f), pulse); // Feu / Magie
    glUniform3fv(glGetUniformLocation(_shaderProgram, "pointLight.color"), 1, glm::value_ptr(pointColor));
    glUniform1f(glGetUniformLocation(_shaderProgram, "pointLight.intensity"), 1.0f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "pointLight.constant"), 1.0f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "pointLight.linear"), 0.09f);
    glUniform1f(glGetUniformLocation(_shaderProgram, "pointLight.quadratic"), 0.032f);

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
            
            bool isHovered = (_hoveredX == i && _hoveredY == j);
            bool isSelected = (selectedX == i && selectedY == j);

            if (isSelected) {
                // Highlight jaune pour la case sélectionnée
                tileColor = glm::mix(tileColor, glm::vec3(0.8f, 0.8f, 0.2f), 0.6f);
            } else if (isHovered) {
                // Highlight leger pour le survol
                tileColor = glm::mix(tileColor, glm::vec3(0.5f, 0.8f, 1.0f), 0.4f);
            }

            glUniform3fv(colorLoc, 1, glm::value_ptr(tileColor));

            glUniform1i(glGetUniformLocation(_shaderProgram, "hasTexture"), 0);
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
                    auto it = _pieceModels.find(modelStr);
                    if (it != _pieceModels.end()) {
                        const Model& model3D = it->second;

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

                        // Coloration
                        glm::vec3 pieceColor = (piece.color == Color::Blanc) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.15f, 0.15f, 0.15f);
                        
                        // -- Phase 3: Loi Binomiale (Surbrillance Magique) --
                        if (_isChaosMode && _enchantedPieces.count(piece.id) > 0) {
                            pieceColor = glm::mix(pieceColor, glm::vec3(0.8f, 0.2f, 1.0f), 0.5f); // Violet magique
                        }

                        // Survol et Sélection de la PIÈCE
                        if (isSelected) {
                            pieceColor = glm::mix(pieceColor, glm::vec3(1.0f, 1.0f, 0.3f), 0.5f); // Glow Jaune
                        } else if (isHovered) {
                            pieceColor = glm::mix(pieceColor, glm::vec3(0.6f, 0.9f, 1.0f), 0.3f); // Glow Bleu ciel
                        }

                        glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));

                        for(const auto& mesh : model3D._meshes) {
                            mesh.bind();
                            mesh.bindTexture(_shaderProgram);
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

        if (_pieceModels.count(modelName) > 0) {
            const Model& model3D = _pieceModels[modelName];

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
                mesh.bindTexture(_shaderProgram);
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
        glUniform1i(glGetUniformLocation(_shaderProgram, "hasTexture"), 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
    
    // ====== RENDU DES GRADINS & SPECTATEURS (Poisson + Permutation) ======
    if (_isChaosMode && _pieceModels.count("seats") > 0 && _pieceModels.count("spectator") > 0) {
        const Model& seatModel = _pieceModels["seats"];
        const Model& specModel = _pieceModels["spectator"];
        
        // Couleur neutre / bleue pour les sièges
        glm::vec3 seatColor(0.2f, 0.4f, 0.7f);
        glUniform3fv(colorLoc, 1, glm::value_ptr(seatColor));
        for(const auto& mesh : seatModel._meshes) {
            mesh.bind();
            mesh.bindTexture(_shaderProgram);

            for (const auto& sTrans : _seatTransforms) {
                glm::mat4 m = glm::scale(sTrans, glm::vec3(0.02f)); // Ajustement d'echelle empirique
                m = glm::rotate(m, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f)); // Rotation pour ne pas faire face au sol
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(m));
                glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
            }
            mesh.unbind();
        }

        // Couleur neutre pour que la texture du modèle soit visible sans filtre
        glm::vec3 specColor(1.0f, 1.0f, 1.0f);
        glUniform3fv(colorLoc, 1, glm::value_ptr(specColor));
        glm::vec3 specAabbMin = specModel.getAABBMin();
        glm::vec3 specAabbMax = specModel.getAABBMax();
        glm::vec3 specCenter = (specAabbMin + specAabbMax) * 0.5f;

        for(const auto& mesh : specModel._meshes) {
            mesh.bind();
            mesh.bindTexture(_shaderProgram);
            
            for (const auto& s : _spectators) {
                // Trouver la transformation globale de ce siège:
                int gradinIndex = s.seatIndex / _spectatorSeatLocalTransforms.size();
                int localSeatIndex = s.seatIndex % _spectatorSeatLocalTransforms.size();

                // S'il y a plus de sièges demandés que de gradins existants, on sécurise
                if (gradinIndex >= _seatTransforms.size()) continue;

                glm::mat4 baseTrans = _seatTransforms[gradinIndex];
                glm::mat4 localTrans = _spectatorSeatLocalTransforms[localSeatIndex];

                // On combine
                glm::mat4 thisSpecModel = baseTrans * localTrans;

                // --- AJUSTEMENT HAUTEUR ET ANIMATION (Sautillement) ---
                // 1. Hauteur de base (ajuste cette valeur pour monter le modèle sur le siège)
                float baseHeightOffset = 0.5f; 
                
                // 2. Animation de saut (basée sur le temps global)
                // Chaque spectateur saute à un rythme légèrement décalé selon son index
                float time = (float)ImGui::GetTime();
                float jumpSpeed = 6.0f; // Vitesse des rebonds
                float jumpHeight = 0.5f; // Hauteur max du saut
                
                // Utilise abs(sin) pour un effet de rebond sur le siège
                float jumpAnimOscillation = std::abs(sin(time * jumpSpeed + (float)s.seatIndex * 0.5f)); 
                
                float finalOffset = baseHeightOffset + (jumpAnimOscillation * jumpHeight);
                thisSpecModel = glm::translate(thisSpecModel, glm::vec3(0.0f, finalOffset, 0.0f));
                // ------------------------------------------------------

                // Ajustement de la taille, orientation vers l'echiquier (+Z du modele)
                thisSpecModel = glm::scale(thisSpecModel, glm::vec3(0.02f)); // Echelle du Pikmin
                
                // Centrage de la géométrie du Pikmin
                thisSpecModel = glm::translate(thisSpecModel, -specCenter);

                
                glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(thisSpecModel));
                glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
            }
            mesh.unbind();
        }
    }

    glBindVertexArray(0);

    // Rendu de la Skybox en dernier pour optimisation avec LEQUAL
    _skybox.draw(_skyboxShaderProgram, _view, _projection);

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

void Renderer3D::updateViewMatrix()
{
    float yawRad   = glm::radians(_yaw);
    float pitchRad = glm::radians(_pitch);

    if (!_isFpsMode) {
        // Trackball mode
        _camPos.x = _target.x + _distance * cos(pitchRad) * cos(yawRad);
        _camPos.y = _target.y + _distance * sin(pitchRad);
        _camPos.z = _target.z + _distance * cos(pitchRad) * sin(yawRad);
        
        _view = glm::lookAt(_camPos, _target, glm::vec3(0.0f, 1.0f, 0.0f));
    } else {
        // FPS mode (POV of a piece)
        // Mettre la camera au dessus de la pièce
        _camPos = _fpsPos + glm::vec3(0.0f, 1.0f, 0.0f);
        glm::vec3 front;
        front.x = cos(yawRad) * cos(pitchRad);
        front.y = sin(pitchRad);
        front.z = sin(yawRad) * cos(pitchRad);
        front = glm::normalize(front);
        
        _view = glm::lookAt(_camPos, _camPos + front, glm::vec3(0.0f, 1.0f, 0.0f));
    }
}

void Renderer3D::updateCamera()
{
    ImGuiIO& io = ImGui::GetIO();

    // Sensibilité de la rotation
    float sensitivity = 0.5f;

    if (!_isFpsMode) {
        // On récupère le mouvement relatif de la souris (MouseDelta)
        _yaw += io.MouseDelta.x * sensitivity;
        _pitch -= io.MouseDelta.y * sensitivity;

        // Limites pour éviter de "retourner" la caméra et ne pas passer sous l'échiquier
        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < 1.0f) _pitch = 1.0f;
    } else {
        _yaw += io.MouseDelta.x * sensitivity;
        _pitch -= io.MouseDelta.y * sensitivity;

        if (_pitch > 89.0f) _pitch = 89.0f;
        if (_pitch < -89.0f) _pitch = -89.0f; // Can look up and down freely
    }

    // On recalcule la matrice de vue (View Matrix)
    updateViewMatrix();
}

void Renderer3D::updateRaycast(float mouseX, float mouseY, int screenWidth, int screenHeight)
{
    // 1. Normalized Device Coordinates (NDC)
    // Invert Y since screen goes down, but NDC goes up
    float x = (2.0f * mouseX) / screenWidth - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / screenHeight;
    float z = 1.0f;
    glm::vec3 ray_nds = glm::vec3(x, y, z);

    // 2. Clip Coordinates
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0, 1.0);

    // 3. Eye/Camera Coordinates
    glm::mat4 invProj = glm::inverse(_projection);
    glm::vec4 ray_eye = invProj * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0, 0.0);

    // 4. World Coordinates
    glm::mat4 invView = glm::inverse(_view);
    glm::vec3 ray_wor = glm::vec3(invView * ray_eye);
    ray_wor = glm::normalize(ray_wor);

    // 5. Intersection Plane (Y=0, the chessboard floor)
    // Ray equation: P = _camPos + t * ray_wor
    // We want P.y = 0 -> _camPos.y + t * ray_wor.y = 0 -> t = -_camPos.y / ray_wor.y
    _hoveredX = -1;
    _hoveredY = -1;

    if (abs(ray_wor.y) > 0.001f) {
        float t = -_camPos.y / ray_wor.y;
        if (t >= 0.0f) {
            glm::vec3 intersection = _camPos + t * ray_wor;
            
            // On map la 3D (X,Z) aux indices d'échiquier (j, i)
            // L'échiquier fait 8x8. X de 0 à 8, Z de 0 à 8
            if (intersection.x >= 0.0f && intersection.x < 8.0f &&
                intersection.z >= 0.0f && intersection.z < 8.0f) {
                _hoveredY = (int)floor(intersection.x); // j
                _hoveredX = (int)floor(intersection.z); // i
            }
        }
    }
}