#include <imgui.h>
#include "../include/ChessEngine.hpp"
#include "Renderer3D.hpp"
#include "quick_imgui/quick_imgui.hpp"

int main()
{
    ChessEngine engine;
    Renderer3D  renderer3D; // <--- Nouvelle instance

    int selectedX = -1, selectedY = -1;

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

            ImGui::Begin("Vue 3D");
            
            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            int width = (int)viewportPanelSize.x;
            int height = (int)viewportPanelSize.y;

            if (width > 0 && height > 0) {
                renderer3D.render(width, height);

                // 2. On affiche la texture résultante dans ImGui
                // On inverse l'axe Y (uv0=(0,1), uv1=(1,0)) car OpenGL a l'origine en bas à gauche
                ImGui::Image(renderer3D.getTextureID(), viewportPanelSize, ImVec2(0, 1), ImVec2(1, 0));
            }

            ImGui::End(); }});
}