#include <imgui.h>
#include <iostream>
#include <string>
#include "quick_imgui/quick_imgui.hpp"

enum class Color { Blanc,
                   Noir };
enum class PieceType { Pion,
                       Tour,
                       Cavalier,
                       Fou,
                       Reine,
                       Roi,
                       None };

struct Piece {
    PieceType type;
    Color     color;

    const char* getIcon() const
    {
        if (type == PieceType::Pion)
            return (color == Color::Blanc) ? "P" : "p";
        else if (type == PieceType::Tour)
            return (color == Color::Blanc) ? "T" : "t";
        else if (type == PieceType::Cavalier)
            return (color == Color::Blanc) ? "C" : "c";
        else if (type == PieceType::Fou)
            return (color == Color::Blanc) ? "F" : "f";
        else if (type == PieceType::Reine)
            return (color == Color::Blanc) ? "Q" : "q";
        else if (type == PieceType::Roi)
            return (color == Color::Blanc) ? "K" : "k";
        return "";
    }
};

Color current_player = Color::Blanc;

void setupStartingPosition(std::optional<Piece> board[8][8])
{
    // vider le plateau
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            board[i][j] = std::nullopt;
    // Définir le joueur courant
    current_player = Color::Blanc;
    // On place les pions
    for (int i = 0; i < 8; i++)
    {
        board[1][i] = Piece{.type = PieceType::Pion, .color = Color::Noir};
        board[6][i] = Piece{.type = PieceType::Pion, .color = Color::Blanc};
    }

    board[0][0] = board[0][7] = Piece{.type = PieceType::Tour, .color = Color::Noir};
    board[7][0] = board[7][7] = Piece{.type = PieceType::Tour, .color = Color::Blanc};
    board[0][1] = board[0][6] = Piece{.type = PieceType::Cavalier, .color = Color::Noir};
    board[7][1] = board[7][6] = Piece{.type = PieceType::Cavalier, .color = Color::Blanc};
    board[0][2] = board[0][5] = Piece{.type = PieceType::Fou, .color = Color::Noir};
    board[7][2] = board[7][5] = Piece{.type = PieceType::Fou, .color = Color::Blanc};
    board[0][3]               = Piece{.type = PieceType::Reine, .color = Color::Noir};
    board[7][3]               = Piece{.type = PieceType::Reine, .color = Color::Blanc};
    board[0][4]               = Piece{.type = PieceType::Roi, .color = Color::Noir};
    board[7][4]               = Piece{.type = PieceType::Roi, .color = Color::Blanc};
}

bool noMansLand(int fromX, int fromY, int toX, int toY, std::optional<Piece> plateau[8][8])
{
    int stepX = (toX - fromX) == 0 ? 0 : (toX - fromX) / std::abs(toX - fromX);
    int stepY = (toY - fromY) == 0 ? 0 : (toY - fromY) / std::abs(toY - fromY);

    int x = fromX + stepX;
    int y = fromY + stepY;

    while (x != toX || y != toY)
    {
        if (plateau[x][y].has_value())
            return false;
        x += stepX;
        y += stepY;
    }
    return true;
}

bool canPionMove(int fromX, int fromY, int toX, int toY, std::optional<Piece> plateau[8][8])
{
    Piece& p         = plateau[fromX][fromY].value();
    int    direction = (p.color == Color::Blanc) ? -1 : 1;
    int    startRow  = (p.color == Color::Blanc) ? 6 : 1;

    int dx = toX - fromX;
    int dy = toY - fromY;

    if (dx == direction && dy == 0 && !plateau[toX][toY].has_value())
    {
        return true;
    }

    // déplacement 2 cases
    if (fromX == startRow && dx == 2 * direction && dy == 0 && !plateau[toX][toY].has_value() && !plateau[fromX + direction][fromY].has_value())
    {
        return true;
    }

    // capture
    if (dx == direction && std::abs(dy) == 1 && plateau[toX][toY].has_value() && plateau[toX][toY]->color != p.color)
    {
        return true;
    }

    return false;
}

bool canIMove(int fromX, int fromY, int toX, int toY, std::optional<Piece> plateau[8][8])
{
    Piece& p = plateau[fromX][fromY].value();

    if (plateau[toX][toY].has_value() && plateau[toX][toY]->color == p.color)
    {
        return false;
    }

    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (p.type)
    {
    case PieceType::Cavalier:
        return (std::abs(dx) * std::abs(dy) == 2);

    case PieceType::Tour:
        if (dx != 0 && dy != 0)
            return false;
        return noMansLand(fromX, fromY, toX, toY, plateau);

    case PieceType::Fou:
        if (std::abs(dx) != std::abs(dy))
            return false;
        return noMansLand(fromX, fromY, toX, toY, plateau);

    case PieceType::Reine:
        if (dx != 0 && dy != 0 && std::abs(dx) != std::abs(dy))
            return false;
        return noMansLand(fromX, fromY, toX, toY, plateau);

    case PieceType::Roi:
        return std::abs(dx) <= 1 && std::abs(dy) <= 1;

    case PieceType::Pion:
        return canPionMove(fromX, fromY, toX, toY, plateau);
    }
}

int main()
{
    float value{0.f};

    int                  selectedX = -1, selectedY = -1;
    std::optional<Piece> plateau[8][8];
    std::string          titre_fenetre = (current_player == Color::Blanc ? "Blanc à toi de jouer !" : "Noir à toi de jouer !");

    quick_imgui::loop(
        "Chess",
        {
            .init = [&]() { setupStartingPosition(plateau); },
            .loop =
                [&]() {
                    ImGui::ShowDemoWindow(); // This opens a window which shows tons of examples of what you can do with ImGui. You should check it out! Also, you can use the "Item Picker" in the top menu of that demo window: then click on any widget and it will show you the corresponding code directly in your IDE!

                    ImGui::Begin("Echec");

                    ImGui::Separator();
                    ImGui::TextColored(ImVec4{1, 1, 0, 1}, "%s", titre_fenetre.c_str());
                    ImGui::Separator();
                    if (ImGui::Button("Rejouer"))
                    {
                        setupStartingPosition(plateau);
                        selectedX = -1;
                        selectedY = -1;
                    }

                    // construire une grille de 8x8
                    for (int i = 0; i < 8; i++)
                    {
                        for (int j = 0; j < 8; j++)
                        {
                            int stylesCount = 0;

                            bool isSelected = (selectedX == i && selectedY == j);
                            if (isSelected)
                            {
                                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{0.8f, 0.8f, 0.0f, 1.0f});
                            }
                            else
                            {
                                ImVec4 color = ((i + j) % 2 == 0) ? ImVec4{0.8f, 0.8f, 0.8f, 1.f} : ImVec4{0.2f, 0.2f, 0.2f, 1.f};
                                ImGui::PushStyleColor(ImGuiCol_Button, color);
                            }
                            stylesCount++;

                            if (plateau[i][j].has_value())
                            {
                                if (plateau[i][j]->color == Color::Blanc)
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{1.0f, 1.0f, 1.0f, 1.0f}); // Blanc
                                else
                                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4{0.0f, 0.0f, 0.0f, 1.0f}); // Noir
                                stylesCount++;
                            }

                            const char* label = plateau[i][j].has_value() ? plateau[i][j]->getIcon() : "";

                            ImGui::PushID(i * 8 + j);
                            if (ImGui::Button(label, ImVec2{50.f, 50.f}))
                            {
                                titre_fenetre = (current_player == Color::Blanc ? "Blanc à toi de jouer !" : "Noir à toi de jouer !");
                                if (selectedX == -1 && selectedY == -1)
                                {
                                    // test si case pas vide
                                    if (plateau[i][j].has_value())
                                    {
                                        if (plateau[i][j]->color == current_player)
                                        {
                                            selectedX = i;
                                            selectedY = j;
                                            std::cout << "piece selectionnee en (" << i << ", " << j << ")\n";
                                        }
                                        else
                                        {
                                            titre_fenetre = "Ce n'est pas votre tour!";
                                        }
                                    }
                                }
                                // une pièce est déjà sélectionnée
                                else
                                {
                                    if (selectedX == i && selectedY == j)
                                    {
                                        selectedX = -1;
                                        selectedY = -1;
                                    }
                                    else
                                    {
                                        if (canIMove(selectedX, selectedY, i, j, plateau))
                                        {
                                            plateau[i][j]                 = plateau[selectedX][selectedY];
                                            plateau[selectedX][selectedY] = std::nullopt;
                                            selectedX                     = -1;
                                            selectedY                     = -1;

                                            std::cout << "Piece deplacee vers (" << i << ", " << j << ")\n";
                                            current_player = (current_player == Color::Blanc) ? Color::Noir : Color::Blanc;
                                            titre_fenetre  = (current_player == Color::Blanc ? "Blanc à toi de jouer !" : "Noir à toi de jouer !");
                                        }
                                        else
                                        {
                                            titre_fenetre = "Deplacement non autorise !";
                                            selectedX     = -1;
                                            selectedY     = -1;
                                        }
                                    }
                                }
                            }
                            ImGui::PopID();

                            ImGui::PopStyleColor(stylesCount);

                            if (j < 7)
                                ImGui::SameLine();
                        }
                    }

                    ImGui::End();

                    // Example window showing some ImGui widgets
                    ImGui::Begin("Example");

                    ImGui::SliderFloat("My Value", &value, 0.f, 3.f);

                    if (ImGui::Button("1", ImVec2{50.f, 50.f}))
                        std::cout << "Clicked button 1\n";
                    ImGui::SameLine(); // Draw the next ImGui widget on the same line as the previous one. Otherwise it would be below it

                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{1.f, 0.f, 0.f, 1.f}); // Changes the color of all buttons until we call ImGui::PopStyleColor(). There is also ImGuiCol_ButtonActive and ImGuiCol_ButtonHovered

                    ImGui::PushID(2); // When some ImGui items have the same label (for exemple the next two buttons are labeled "Yo") ImGui needs you to specify an ID so that it can distinguish them. It can be an int, a pointer, a string, etc.
                                      // You will definitely run into this when you create a button for each of your chess pieces, so remember to give them an ID!
                    if (ImGui::Button("Yo", ImVec2{50.f, 50.f}))
                        std::cout << "Clicked button 2\n";
                    ImGui::PopID(); // Then pop the id you pushed after you created the widget

                    ImGui::SameLine();
                    ImGui::PushID(3);
                    if (ImGui::Button("Yo", ImVec2{50.f, 50.f}))
                        std::cout << "Clicked button 3\n";
                    ImGui::PopID();

                    ImGui::PopStyleColor();

                    ImGui::End();
                },
        }
    );
}