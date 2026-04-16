#include "../include/ChessEngine.hpp"
#include <cmath>
#include "Piece.hpp"
#include "RandomGen.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <future>

ChessEngine::ChessEngine()
{
    reset(false);
}

void ChessEngine::reset(bool chaos)
{
    isChaosMode = chaos;
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            plateau[i][j] = std::nullopt;

    current_player = Color::Blanc;
    message        = "Blanc à toi de jouer !";

    int nextId = 1;

    for (int i = 0; i < 8; i++)
    {
        plateau[1][i] = Piece{.type = PieceType::Pion, .color = Color::Noir, .id = nextId++};
        plateau[6][i] = Piece{.type = PieceType::Pion, .color = Color::Blanc, .id = nextId++};
    }
    // Positions initiales
    plateau[0][0] = plateau[0][7] = Piece{.type = PieceType::Tour, .color = Color::Noir, .id = nextId++};
    plateau[7][0] = plateau[7][7] = Piece{.type = PieceType::Tour, .color = Color::Blanc, .id = nextId++};
    
    // Assignation ID unique a cause de la struct initializer
    if (plateau[0][7].has_value()) plateau[0][7]->id = nextId++;
    if (plateau[7][7].has_value()) plateau[7][7]->id = nextId++;

    plateau[0][1] = plateau[0][6] = Piece{.type = PieceType::Cavalier, .color = Color::Noir, .id = nextId++};
    plateau[7][1] = plateau[7][6] = Piece{.type = PieceType::Cavalier, .color = Color::Blanc, .id = nextId++};
    if (plateau[0][6].has_value()) plateau[0][6]->id = nextId++;
    if (plateau[7][6].has_value()) plateau[7][6]->id = nextId++;

    plateau[0][2] = plateau[0][5] = Piece{.type = PieceType::Fou, .color = Color::Noir, .id = nextId++};
    plateau[7][2] = plateau[7][5] = Piece{.type = PieceType::Fou, .color = Color::Blanc, .id = nextId++};
    if (plateau[0][5].has_value()) plateau[0][5]->id = nextId++;
    if (plateau[7][5].has_value()) plateau[7][5]->id = nextId++;

    plateau[0][3]                 = Piece{.type = PieceType::Reine, .color = Color::Noir, .id = nextId++};
    plateau[7][3]                 = Piece{.type = PieceType::Reine, .color = Color::Blanc, .id = nextId++};
    plateau[0][4]                 = Piece{.type = PieceType::Roi, .color = Color::Noir, .id = nextId++};
    plateau[7][4]                 = Piece{.type = PieceType::Roi, .color = Color::Blanc, .id = nextId++};

    // Initialisation continue de l'espérance de vie étudiée (Loi de Weibull)
    if (isChaosMode) {
        for (int i = 0; i < 8; ++i) {
            for (int j = 0; j < 8; ++j) {
                if (plateau[i][j].has_value()) {
                    plateau[i][j]->maxLifespan = RandomGen::weibull(15.0, 2.0);
                    std::cout << "Piece " << plateau[i][j]->id << " a une esperence de vie de " << plateau[i][j]->maxLifespan << std::endl;
                }
            }
        }
    }
}

bool ChessEngine::estEnEchec(Color joueur) const {
    // on parcourt tout le tableau pour trouver le roi. Une fois trouver on reparcourt le tableau et on test pour chaque pièce adverse la possibilité de l'atteindre
    int kingX = -1, kingY = -1;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].has_value() && 
                plateau[i][j]->type == PieceType::Roi && 
                plateau[i][j]->color == joueur) {
                kingX = i;
                kingY = j;
                break;
            }
        }
    }

    Color adversaire = (joueur == Color::Blanc) ? Color::Noir : Color::Blanc;
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].has_value() && plateau[i][j]->color == adversaire) {
                if (canIMove(i, j, kingX, kingY)) {
                    return true;
                }
            }
        }
    }
    return false;
}

bool ChessEngine::estEnEchecEtMat(Color joueur) const {
    if (!estEnEchec(joueur)) return false; // Pas mat s'il n'y a même pas échec

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (plateau[i][j].has_value() && plateau[i][j]->color == joueur) {
                
                for (int destX = 0; destX < 8; destX++) {
                    for (int destY = 0; destY < 8; destY++) {
                        if (canIMove(i, j, destX, destY)) {
                            
                            // On crée une copie locale du moteur pour simuler le coup
                            ChessEngine simulation = *this; 
                            
                            simulation.plateau[destX][destY] = simulation.plateau[i][j];
                            simulation.plateau[i][j] = std::nullopt;

                            if (!simulation.estEnEchec(joueur)) {
                                return false; 
                            }
                        }
                    }
                }
            }
        }
    }
    return true;
}

bool ChessEngine::noMansLand(int fromX, int fromY, int toX, int toY) const
{
    int stepX = (toX - fromX) == 0 ? 0 : (toX - fromX) / std::abs(toX - fromX);
    int stepY = (toY - fromY) == 0 ? 0 : (toY - fromY) / std::abs(toY - fromY);
    int x     = fromX + stepX;
    int y     = fromY + stepY;
    while (x != toX || y != toY)
    {
        if (plateau[x][y].has_value())
            return false;
        x += stepX;
        y += stepY;
    }
    return true;
}

bool ChessEngine::canPionMove(int fromX, int fromY, int toX, int toY) const
{
    const Piece& p = plateau[fromX][fromY].value();
    int    dir   = (p.color == Color::Blanc) ? -1 : 1;
    int    start = (p.color == Color::Blanc) ? 6 : 1;
    int    dx    = toX - fromX;
    int    dy    = toY - fromY;

    if (dx == dir && dy == 0 && !plateau[toX][toY].has_value())
        return true;
    if (fromX == start && dx == 2 * dir && dy == 0 && !plateau[toX][toY].has_value() && !plateau[fromX + dir][fromY].has_value())
        return true;
    if (dx == dir && std::abs(dy) == 1 && plateau[toX][toY].has_value() && plateau[toX][toY]->color != p.color)
        return true;
    return false;
}

bool ChessEngine::canIMove(int fromX, int fromY, int toX, int toY) const
{
    if (!plateau[fromX][fromY].has_value())
        return false;
    const Piece& p = plateau[fromX][fromY].value();
    if (plateau[toX][toY].has_value() && plateau[toX][toY]->color == p.color)
        return false;

    int dx = toX - fromX;
    int dy = toY - fromY;

    switch (p.type)
    {
    case PieceType::Cavalier: return (std::abs(dx) * std::abs(dy) == 2);
    case PieceType::Tour: return (dx == 0 || dy == 0) && noMansLand(fromX, fromY, toX, toY);
    case PieceType::Fou: return (std::abs(dx) == std::abs(dy)) && noMansLand(fromX, fromY, toX, toY);
    case PieceType::Reine: return (dx == 0 || dy == 0 || std::abs(dx) == std::abs(dy)) && noMansLand(fromX, fromY, toX, toY);
    case PieceType::Roi: return std::abs(dx) <= 1 && std::abs(dy) <= 1;
    case PieceType::Pion: return canPionMove(fromX, fromY, toX, toY);
    default: return false;
    }
}

void ChessEngine::executeMove(int fromX, int fromY, int toX, int toY)
{
    float distSq = (toX - fromX)*(toX - fromX) + (toY - fromY)*(toY - fromY);
    float dist = std::sqrt(distSq);

    plateau[toX][toY]     = plateau[fromX][fromY];
    plateau[fromX][fromY] = std::nullopt;

    if (plateau[toX][toY].has_value()) {
        plateau[toX][toY]->traveledDistance += dist;
        if (isChaosMode) {
            if (plateau[toX][toY]->traveledDistance >= plateau[toX][toY]->maxLifespan) {
                plateau[toX][toY] = std::nullopt;
                message = "Une piece est morte de fatigue ! ";
            }
        }
    }


    current_player        = (current_player == Color::Blanc) ? Color::Noir : Color::Blanc;
    if(estEnEchec(current_player)) {
        message = "Le roi est en echec !";
        message += (current_player == Color::Blanc ? "Blanc a toi de jouer !" : "Noir a toi de jouer !");
    }
    
    if (message.find("fatigue") == std::string::npos) {
        message               = (current_player == Color::Blanc ? "Blanc a toi de jouer !" : "Noir a toi de jouer !");
    } else {
        message += (current_player == Color::Blanc ? "A Blanc !" : "A Noir !");
    }

    if(estEnEchecEtMat(current_player)) {
        message = "Le roi est en echec et mat ! La partie est terminée";
        message += (current_player == Color::Blanc ? " Les Noirs ont gagné !" : " Les Blancs ont gagné !");

        // retourner au menu après 5 secondes sans bloquer la fenêtre
        std::thread([this]() {
            std::this_thread::sleep_for(std::chrono::seconds(5));
            this->reset(this->isChaosMode);
        }).detach();
    }
}
