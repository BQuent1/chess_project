#include "ParticleSystem.hpp"
#include "RandomGen.hpp"
#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

void ParticleSystem::updateAndRender(float deltaTime, bool isChaosMode, const ChessEngine& engine, 
                                     const std::set<int>& enchantedPieces, 
                                     const Shader& shader, unsigned int vao) {
    if (!isChaosMode) return;

    glBindVertexArray(vao);
    
    // Emettre des particules pour chaque pièce enchantée toujours en vie
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (engine.plateau[i][j].has_value()) {
                const Piece& p = engine.plateau[i][j].value();
                if (enchantedPieces.count(p.id) > 0) {
                    if (RandomGen::uniformContinuous(0.0, 1.0) < 0.2) { // 20% de chances par frame
                        Particle part;
                        part.position = glm::vec3(j + 0.5f + RandomGen::uniformContinuous(-0.3, 0.3), 
                                                  0.05f, 
                                                  i + 0.5f + RandomGen::uniformContinuous(-0.3, 0.3));
                        part.velocity = glm::vec3(RandomGen::uniformContinuous(-0.2, 0.2),
                                                  RandomGen::uniformContinuous(0.5, 1.2), // vers le haut
                                                  RandomGen::uniformContinuous(-0.2, 0.2));
                        part.maxLife = RandomGen::uniformContinuous(0.5, 1.5);
                        part.life = part.maxLife;
                        part.pieceId = p.id;
                        _particles.push_back(part);
                    }
                }
            }
        }
    }
    
    // Mettre à jour et dessiner les particules
    shader.setInt("hasTexture", 0);
    glm::vec3 partColor = glm::vec3(0.8f, 0.2f, 1.0f); // Violet magique
    
    for (auto it = _particles.begin(); it != _particles.end();) {
        it->life -= deltaTime;
        if (it->life <= 0.0f) {
            it = _particles.erase(it);
        } else {
            it->position += it->velocity * deltaTime;
            
            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, it->position);
            float scale = 0.05f * (it->life / it->maxLife);
            model = glm::scale(model, glm::vec3(scale));
            
            shader.setMat4("model", model);
            shader.setVec3("squareColor", partColor);
            
            glDrawArrays(GL_TRIANGLES, 0, 36);
            
            ++it;
        }
    }
    glBindVertexArray(0);
}
