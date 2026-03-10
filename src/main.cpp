#include <imgui.h>
#include "../include/ChessEngine.hpp"
#include "Renderer3D.hpp"
#include "quick_imgui/quick_imgui.hpp"

int main()
{
    ChessEngine engine;
    Renderer3D  renderer3D;

    int   selectedX = -1, selectedY = -1;
    float expValue       = 0.0f;
    float bernoulliValue = 0.0f;

    quick_imgui::loop("Chess Project", {.init = [&]() {
            // Initialisation du renderer avec une taille par défaut
            // (Il s'adaptera tout seul après)
            renderer3D.init(800, 600); }, .loop = [&]() {
                                            // ... (TA FENÊTRE "ECHEC" 2D EST ICI, NE CHANGE RIEN) ...
                                            ImGui::Begin("Echec");
                                            ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", engine.message.c_str());

                                            if (ImGui::Button("Rejouer"))
                                            {
                                                engine.reset();
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
                                                        if (selectedX == -1)
                                                        {
                                                            if (engine.plateau[i][j] && engine.plateau[i][j]->color == engine.current_player)
                                                            {
                                                                selectedX = i;
                                                                selectedY = j;
                                                            }
                                                        }
                                                        else
                                                        {
                                                            if (selectedX == i && selectedY == j)
                                                                selectedX = selectedY = -1;
                                                            else if (engine.canIMove(selectedX, selectedY, i, j))
                                                            {
                                                                engine.executeMove(selectedX, selectedY, i, j);
                                                                selectedX = selectedY = -1;
                                                            }
                                                            else
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

                                            ImGui::Begin("Aleatoire");
                                            ImGui::Text("Clique pour un nombre aléatoire grace a un generateur exponentiel");
                                            if (ImGui::Button("Generer exponentiel"))
                                            {
                                                float randomValue = static_cast<float>(rand()) / static_cast<float>(RAND_MAX);
                                                expValue          = -log(1.0f - randomValue) * 5.0f;
                                            }
                                            if (ImGui::Button("Generer bernoulli"))
                                            {
                                                bernoulliValue = (rand() % 2) == 0 ? 0.0f : 1.0f;
                                            }

                                            ImGui::Text("Valeur aléatoire exponentielle : %.2f", expValue);
                                            ImGui::Text("Valeur bernoulli : %.2f", bernoulliValue);
                                            ImGui::End();


                                            ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoMove; // Empêche le déplacement au clic

                                            ImGui::Begin("Vue 3D", nullptr, window_flags);

                                            ImVec2 viewportSize = ImGui::GetContentRegionAvail();
                                            renderer3D.render((int)viewportSize.x, (int)viewportSize.y);
                                            // On dessine l'image
                                            ImGui::Image(renderer3D.getTextureID(), viewportSize, ImVec2(0, 1), ImVec2(1, 0));

                                            if (ImGui::IsItemHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
                                            {
                                                renderer3D.updateCamera();
                                            }

                                            ImGui::End(); }});
}