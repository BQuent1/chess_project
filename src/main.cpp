#include <imgui.h>
#include "../include/ChessEngine.hpp"
#include "Renderer3D.hpp"
#include "quick_imgui/quick_imgui.hpp"
#include <GLFW/glfw3.h>

int main()
{
    ChessEngine engine;
    Renderer3D  renderer3D;

    int   selectedX = -1, selectedY = -1;
    float expValue       = 0.0f;
    float bernoulliValue = 0.0f;
    bool fpsModeActive = false;

    enum class ScreenState { MainMenu, Playing };
    ScreenState currentScreen = ScreenState::MainMenu;

    quick_imgui::loop("Chess Project", {.init = [&]() {
            // Initialisation du renderer avec une taille par défaut
            // (Il s'adaptera tout seul après)
            renderer3D.init(800, 600); }, .loop = [&]() {
                                            if (currentScreen == ScreenState::MainMenu) {
                                                ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
                                                ImGui::Begin("Menu Principal", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoMove);
                                                
                                                ImGui::Text("Chess Project");
                                                ImGui::Separator();
                                                ImGui::Spacing();

                                                if (ImGui::Button("Mode Classique", ImVec2(200, 50))) {
                                                    engine.reset(false);
                                                    renderer3D.setChaosMode(false);
                                                    currentScreen = ScreenState::Playing;
                                                }

                                                ImGui::Spacing();

                                                if (ImGui::Button("Mode Chaos", ImVec2(200, 50))) {
                                                    engine.reset(true);
                                                    renderer3D.setChaosMode(true);
                                                    currentScreen = ScreenState::Playing;
                                                }

                                                ImGui::End();
                                                
                                                int width, height;
                                                glfwGetFramebufferSize(glfwGetCurrentContext(), &width, &height);
                                                renderer3D.render(width, height, engine, -1, -1);
                                                return;
                                            }

                                            ImGui::Begin("Echec");
                                            ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", engine.message.c_str());

                                            if (ImGui::Button("Rejouer"))
                                            {
                                                engine.reset(engine.isChaosMode);
                                                renderer3D.resetChaosState();
                                                selectedX = selectedY = -1;
                                            }
                                            ImGui::SameLine();
                                            if (ImGui::Button("Menu Principal"))
                                            {
                                                currentScreen = ScreenState::MainMenu;
                                                renderer3D.resetChaosState();
                                                selectedX = selectedY = -1;
                                            }

                                            for (int i = 0; i < 8; i++)
                                            {
                                                for (int j = 0; j < 8; j++)
                                                {
                                                    int  styles     = 0;
                                                    bool isSelected = (selectedX == i && selectedY == j);
                                                    bool isPossible = (selectedX != -1 && engine.canIMove(selectedX, selectedY, i, j));

                                                    // Gestion des couleurs
                                                    if (isSelected)
                                                        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.8f, 0.0f, 1.0f});
                                                    else if (isPossible)
                                                    {
                                                        ImVec4 col = engine.plateau[i][j] ? ImVec4{0.8f, 0.2f, 0.2f, 0.7f} : ImVec4{0.2f, 0.8f, 0.2f, 0.7f};
                                                        ImGui::PushStyleColor(ImGuiCol_Button, col);
                                                    }
                                                    else
                                                    {
                                                        ImVec4 col = ((i + j) % 2 == 0) ? ImVec4{0.8f, 0.8f, 0.8f, 1.f} : ImVec4{0.2f, 0.2f, 0.2f, 1.f};
                                                        ImGui::PushStyleColor(ImGuiCol_Button, col);
                                                    }
                                                    styles++;

                                                    if (engine.plateau[i][j])
                                                    {
                                                        ImVec4 txt = (engine.plateau[i][j]->color == Color::Blanc) ? ImVec4{1, 1, 1, 1} : ImVec4{0, 0, 0, 1};
                                                        ImGui::PushStyleColor(ImGuiCol_Text, txt);
                                                        styles++;
                                                    }

                                                    ImGui::PushID(i * 8 + j);
                                                    const char* icone = engine.plateau[i][j] ? engine.plateau[i][j]->getIcon() : "";
                                                    if (ImGui::Button(icone, ImVec2{50.f, 50.f}))
                                                    {
                                                        
                                                        if (selectedX == -1) // on n'a pas encore sélectionné de pièce
                                                        {
                                                            if (engine.plateau[i][j] && engine.plateau[i][j]->color == engine.current_player)
                                                            {
                                                                selectedX = i;
                                                                selectedY = j;
                                                                if (fpsModeActive) {
                                                                    glm::vec3 piecePos((float)selectedY + 0.5f, 0.5f, (float)selectedX + 0.5f);
                                                                    renderer3D.setFpsMode(true, piecePos);
                                                                }
                                                            }
                                                        }
                                                        else // on a déjà sélectionné une pièce
                                                        {
                                                            if (selectedX == i && selectedY == j) // si on reclique sur la case sélectionnée
                                                                selectedX = selectedY = -1;
                                                            else if (engine.canIMove(selectedX, selectedY, i, j)) // si le mouvement est possible
                                                            {
                                                                if (engine.plateau[selectedX][selectedY].has_value()) { // si la case est occupée par une pièce
                                                                    renderer3D.triggerAnimation(selectedX, selectedY, i, j, engine.plateau[selectedX][selectedY].value());
                                                                    if (fpsModeActive) {
                                                                        glm::vec3 piecePos((float)j + 0.5f, 0.5f, (float)i + 0.5f);
                                                                        renderer3D.setFpsMode(true, piecePos);
                                                                    }
                                                                }
                                                                engine.executeMove(selectedX, selectedY, i, j);
                                                                if (fpsModeActive) {
                                                                    glm::vec3 piecePos((float)j + 0.5f, 0.5f, (float)i + 0.5f);
                                                                    renderer3D.setFpsMode(true, piecePos);
                                                                }
                                                                selectedX = selectedY = -1;
                                                            }
                                                            else // si le mouvement n'est pas possible
                                                            {
                                                                selectedX = selectedY = -1;
                                                            }
                                                        }
                                                    }
                                                    ImGui::PopID();
                                                    ImGui::PopStyleColor(styles);
                                                    if (j < 7)
                                                        ImGui::SameLine();
                                                }
                                            }

                                            ImGui::End();

                                            
                                            ImGui::Begin("Controles Camera");
                                            if (ImGui::Checkbox("Mode FPS (Vue de piece)", &fpsModeActive)) {
                                                if (fpsModeActive && selectedX != -1 && selectedY != -1) {
                                                    // On se place au centre de la case
                                                    glm::vec3 piecePos((float)selectedY + 0.5f, 0.5f, (float)selectedX + 0.5f);
                                                    renderer3D.setFpsMode(true, piecePos);
                                                    renderer3D.updateViewMatrix();
                                                } else {
                                                    if (fpsModeActive) {
                                                        // Fallback if no piece selected
                                                        renderer3D.setFpsMode(true, glm::vec3(4.0f, 0.5f, 4.0f));
                                                    } else {
                                                        renderer3D.setFpsMode(false);
                                                    }
                                                }
                                                
                                            }
                                            
                                            if (fpsModeActive && selectedX != -1 && selectedY != -1) {
                                                // Dynamic update if selection changes while in FPS
                                                glm::vec3 piecePos((float)selectedY + 0.5f, 0.5f, (float)selectedX + 0.5f);
                                                renderer3D.setFpsMode(true, piecePos);
                                            }
                                            ImGui::End();

                                            if (engine.isChaosMode) {
                                                ImGui::Begin("Temps de Vie des Pieces", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
                                                for (int i = 0; i < 8; i++) {
                                                    for (int j = 0; j < 8; j++) {
                                                        if (engine.plateau[i][j].has_value()) {
                                                            const Piece& p = engine.plateau[i][j].value();
                                                            float percentage = 100.0f * (p.traveledDistance / p.maxLifespan);
                                                            if (percentage > 100.0f) percentage = 100.0f;
                                                            ImVec4 color = p.color == Color::Blanc ? ImVec4(1.0f, 1.0f, 1.0f, 1.0f) : ImVec4(0.6f, 0.6f, 0.6f, 1.0f);
                                                            ImGui::TextColored(color, "%s %s (ID %d) : %.1f / %.1f u (%.0f%% fatigue)",
                                                                (p.color == Color::Blanc) ? "Blanc" : "Noir",
                                                                p.getIcon(),
                                                                p.id,
                                                                p.traveledDistance,
                                                                p.maxLifespan,
                                                                percentage);
                                                        }
                                                    }
                                                }
                                                ImGui::End();
                                            }

                                            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove; // Empêche le déplacement au clic

                                            ImGui::Begin("Vue 3D", nullptr, window_flags);

                                            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                                            ImVec2 windowPos = ImGui::GetCursorScreenPos(); // Position absolue du coin haut gauche du viewport
                                            ImVec2 mousePos = ImGui::GetMousePos();
                                            
                                            // Mouse relative to the viewport
                                            float relativeMouseX = mousePos.x - windowPos.x;
                                            float relativeMouseY = mousePos.y - windowPos.y;

                                            // Only raycast if mouse is within the viewport
                                            if (relativeMouseX >= 0 && relativeMouseX < viewportSize.x && 
                                                relativeMouseY >= 0 && relativeMouseY < viewportSize.y) {
                                                renderer3D.updateRaycast(relativeMouseX, relativeMouseY, (int)viewportSize.x, (int)viewportSize.y);
                                            }

                                            renderer3D.render((int)viewportSize.x, (int)viewportSize.y, engine, selectedX, selectedY);
                                            // On dessine l'image
                                            ImGui::Image(renderer3D.getTextureID(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

                                            // --- GESTION DU CLIC VIA RAYCAST SUR LA VUE 3D ---
                                            static bool isDraggingCamera = false;
                                            
                                            // Activer le drag de la caméra uniquement si on clique en survolant la zone
                                            if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                                                isDraggingCamera = true;
                                            }
                                            // Arrêter le drag si on relâche le bouton, même en dehors de la fenêtre
                                            if (!ImGui::IsMouseDown(ImGuiMouseButton_Right)) {
                                                isDraggingCamera = false;
                                            }

                                            if (isDraggingCamera) {
                                                renderer3D.updateCamera();
                                            }

                                            // Click Interactions (Sélection locale)
                                            if (ImGui::IsItemHovered())
                                            {
                                                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) // Left click selects pieces
                                                {
                                                    int hX = renderer3D.getHoveredX();
                                                    int hY = renderer3D.getHoveredY();

                                                    if (hX != -1 && hY != -1) {
                                                        // Selection Logic matching the 2D UI
                                                        if (selectedX == -1) {
                                                            if (engine.plateau[hX][hY] && engine.plateau[hX][hY]->color == engine.current_player) {
                                                                selectedX = hX;
                                                                selectedY = hY;
                                                                if (fpsModeActive) {
                                                                    glm::vec3 piecePos((float)selectedY + 0.5f, 0.5f, (float)selectedX + 0.5f);
                                                                    renderer3D.setFpsMode(true, piecePos);
                                                                }
                                                            }
                                                        } else {
                                                            if (selectedX == hX && selectedY == hY) {
                                                                selectedX = selectedY = -1; // Deselect
                                                            } else if (engine.canIMove(selectedX, selectedY, hX, hY)) {
                                                                if (engine.plateau[selectedX][selectedY].has_value()) {
                                                                    renderer3D.triggerAnimation(selectedX, selectedY, hX, hY, engine.plateau[selectedX][selectedY].value());
                                                                    if (fpsModeActive) {
                                                                        glm::vec3 piecePos((float)selectedY + 0.5f, 0.5f, (float)selectedX + 0.5f);
                                                                        renderer3D.setFpsMode(true, piecePos);
                                                                    }
                                                                }
                                                                engine.executeMove(selectedX, selectedY, hX, hY);
                                                                if (fpsModeActive) {
                                                                    glm::vec3 piecePos((float)hY + 0.5f, 0.5f, (float)hX + 0.5f);
                                                                    renderer3D.setFpsMode(true, piecePos);
                                                                }
                                                                selectedX = selectedY = -1;
                                                            } else {
                                                                selectedX = selectedY = -1; // Deselect on invalid move
                                                            }
                                                        }
                                                    }
                                                }
                                            }

                                            ImGui::End(); }});
}