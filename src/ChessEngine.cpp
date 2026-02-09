#include "../include/ChessEngine.hpp"
#include <cmath>
#include "Piece.hpp"

ChessEngine::ChessEngine()
{
    reset();
}

void ChessEngine::reset()
{
    for (int i = 0; i < 8; ++i)
        for (int j = 0; j < 8; ++j)
            plateau[i][j] = std::nullopt;

    current_player = Color::Blanc;
    message        = "Blanc à toi de jouer !";

    for (int i = 0; i < 8; i++)
    {
        plateau[1][i] = Piece{.type = PieceType::Pion, .color = Color::Noir};
        plateau[6][i] = Piece{.type = PieceType::Pion, .color = Color::Blanc};
    }
    // Positions initiales
    plateau[0][0] = plateau[0][7] = Piece{.type = PieceType::Tour, .color = Color::Noir};
    plateau[7][0] = plateau[7][7] = Piece{.type = PieceType::Tour, .color = Color::Blanc};
    plateau[0][1] = plateau[0][6] = Piece{.type = PieceType::Cavalier, .color = Color::Noir};
    plateau[7][1] = plateau[7][6] = Piece{.type = PieceType::Cavalier, .color = Color::Blanc};
    plateau[0][2] = plateau[0][5] = Piece{.type = PieceType::Fou, .color = Color::Noir};
    plateau[7][2] = plateau[7][5] = Piece{.type = PieceType::Fou, .color = Color::Blanc};
    plateau[0][3]                 = Piece{.type = PieceType::Reine, .color = Color::Noir};
    plateau[7][3]                 = Piece{.type = PieceType::Reine, .color = Color::Blanc};
    plateau[0][4]                 = Piece{.type = PieceType::Roi, .color = Color::Noir};
    plateau[7][4]                 = Piece{.type = PieceType::Roi, .color = Color::Blanc};
}

bool ChessEngine::noMansLand(int fromX, int fromY, int toX, int toY)
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

bool ChessEngine::canPionMove(int fromX, int fromY, int toX, int toY)
{
    Piece& p     = plateau[fromX][fromY].value();
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

bool ChessEngine::canIMove(int fromX, int fromY, int toX, int toY)
{
    if (!plateau[fromX][fromY].has_value())
        return false;
    Piece& p = plateau[fromX][fromY].value();
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
    plateau[toX][toY]     = plateau[fromX][fromY];
    plateau[fromX][fromY] = std::nullopt;
    current_player        = (current_player == Color::Blanc) ? Color::Noir : Color::Blanc;
    message               = (current_player == Color::Blanc ? "Blanc à toi de jouer !" : "Noir à toi de jouer !");
}