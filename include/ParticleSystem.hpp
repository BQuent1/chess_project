#pragma once
#include <vector>
#include <set>
#include <glm/glm.hpp>
#include "Shader.hpp"
#include "../include/ChessEngine.hpp"

struct Particle {
    glm::vec3 position;
    glm::vec3 velocity;
    float life;
    float maxLife;
    int pieceId;
};

class ParticleSystem {
private:
    std::vector<Particle> _particles;

public:
    ParticleSystem() = default;
    ~ParticleSystem() = default;

    void updateAndRender(float deltaTime, bool isChaosMode, const ChessEngine& engine, 
                         const std::set<int>& enchantedPieces, 
                         const Shader& shader, unsigned int vao);
};
