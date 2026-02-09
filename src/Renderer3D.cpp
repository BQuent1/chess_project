#include "Renderer3D.hpp"
#include <glad/glad.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

// Utilitaire unique pour charger les shaders
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
        0.5f, 0.0f, 0.5f,
        0.5f, 0.0f, -0.5f,
        -0.5f, 0.0f, -0.5f,
        -0.5f, 0.0f, 0.5f
    };
    unsigned int indices[] = {0, 1, 3, 1, 2, 3};

    glGenVertexArrays(1, &_squareVAO);
    glGenBuffers(1, &_squareVBO);
    unsigned int EBO;
    glGenBuffers(1, &EBO);

    glBindVertexArray(_squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, _squareVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
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
    _projection = glm::perspective(glm::radians(45.0f), (float)width / (float)height, 0.1f, 100.0f);
    _view       = glm::lookAt(glm::vec3(4.0f, 10.0f, 12.0f), glm::vec3(4.0f, 0.0f, 4.0f), glm::vec3(0.0f, 1.0f, 0.0f));
}

// ================= RENDU =================
void Renderer3D::render(int width, int height)
{
    glBindFramebuffer(GL_FRAMEBUFFER, _fbo); // FIX : Pas de &
    glViewport(0, 0, width, height);
    glEnable(GL_DEPTH_TEST);

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(_shaderProgram);
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
            glm::mat4 model = glm::mat4(1.0f);
            model           = glm::translate(model, glm::vec3((float)j + 0.5f, 0.0f, (float)i + 0.5f));
            glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

            glm::vec3 color = ((i + j) % 2 == 0) ? glm::vec3(0.9f, 0.9f, 0.9f) : glm::vec3(0.2f, 0.2f, 0.2f);
            glUniform3fv(colorLoc, 1, glm::value_ptr(color));

            glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        }
    }

    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}