#include "Renderer3D.hpp"
#include "../include/ChessEngine.hpp"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

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

    // unsigned int indices[] = {0, 1, 3, 1, 2, 3};

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
}

// ================= RENDU =================
void Renderer3D::render(int width, int height, const ChessEngine& engine)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo); // FIX : Pas de &
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(1.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_shaderProgram);

    unsigned int lightDirLoc = glGetUniformLocation(_shaderProgram, "uLightDir");
    unsigned int viewPosLoc  = glGetUniformLocation(_shaderProgram, "uViewPos");

    glUniform3f(lightDirLoc, -0.5f, -1.0f, -0.5f);

    // On envoie la position de la caméra (extraite de ta matrice _view ou définie manuellement)
    glUniform3f(viewPosLoc, _camPos.x, _camPos.y, _camPos.z);

    _projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);

    unsigned int viewLoc  = glGetUniformLocation(_shaderProgram, "view");
    unsigned int projLoc  = glGetUniformLocation(_shaderProgram, "projection");
    unsigned int modelLoc = glGetUniformLocation(_shaderProgram, "model");
    unsigned int colorLoc = glGetUniformLocation(_shaderProgram, "squareColor");

    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(_view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(_projection));

    glBindVertexArray(_squareVAO);

    for (int i = 0; i < 8; i++)
    {
        for (int j = 0; j < 8; j++)
        {
            // Dans ta boucle de rendu pour le sol
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, glm::vec3((float)j + 0.5f, 0.0f, (float)i + 0.5f));

            // Ici on écrase le cube sur l'axe Y (0.1f) pour en faire une plaque
            model = glm::scale(model, glm::vec3(0.95f, 0.1f, 0.95f));

            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glm::vec3 color = ((i + j) % 2 == 0) ? glm::vec3(0.9f, 0.9f, 0.9f) : glm::vec3(0.2f, 0.2f, 0.2f);
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));

            // glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            glDrawArrays(GL_TRIANGLES, 0, 36);

            // Logique d'état du jeu pour les pièces
            if (engine.plateau[i][j].has_value()) {
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
                        const Mesh& mesh = it->second;

                        // ====== CALCUL DE LA BOUNDING BOX ET CENTRAGE ======
                        glm::vec3 minAABB = mesh.getAABBMin();
                        glm::vec3 maxAABB = mesh.getAABBMax();
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
                        pieceModel = glm::translate(pieceModel, glm::vec3((float)j + 0.5f, 0.05f - lowestY, (float)i + 0.5f));
                        // B. Ajout des rotations et scales
                        pieceModel *= transform;
                        // C. Centrage sur la géométrie locale (on se remet à 0,0,0)
                        pieceModel = glm::translate(pieceModel, -centerAABB);

                        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(pieceModel));

                        // Coloration
                        glm::vec3 pieceColor = (piece.color == Color::Blanc) ? glm::vec3(0.8f, 0.8f, 0.8f) : glm::vec3(0.2f, 0.15f, 0.1f);
                        glUniform3fv(colorLoc, 1, glm::value_ptr(pieceColor));

                        mesh.bind();
                        glDrawArrays(GL_TRIANGLES, 0, mesh.getVertexCount());
                        mesh.unbind();
                    }
                }
                
                glBindVertexArray(_squareVAO); // Re-bind square VAO pour les tuiles suivantes
            }
        }
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Renderer3D::updateViewMatrix()
{
    float yawRad   = glm::radians(_yaw);
    float pitchRad = glm::radians(_pitch);

    glm::vec3 camPos;
    camPos.x = _target.x + _distance * cos(pitchRad) * cos(yawRad);
    camPos.y = _target.y + _distance * sin(pitchRad);
    camPos.z = _target.z + _distance * cos(pitchRad) * sin(yawRad);

    _camPos.x = _target.x + _distance * cos(pitchRad) * cos(yawRad);
    _camPos.y = _target.y + _distance * sin(pitchRad);
    _camPos.z = _target.z + _distance * cos(pitchRad) * sin(yawRad);

    _view = glm::lookAt(_camPos, _target, glm::vec3(0.0f, 1.0f, 0.0f));
}

void Renderer3D::updateCamera()
{
    ImGuiIO& io = ImGui::GetIO();

    // Sensibilité de la rotation
    float sensitivity = 0.5f;

    // On récupère le mouvement relatif de la souris (MouseDelta)
    _yaw += io.MouseDelta.x * sensitivity;
    _pitch -= io.MouseDelta.y * sensitivity;

    // Limites pour éviter de "retourner" la caméra
    if (_pitch > 89.0f)
        _pitch = 89.0f;
    if (_pitch < 10.0f)
        _pitch = 10.0f;

    // On recalcule la matrice de vue (View Matrix)
    updateViewMatrix();
}