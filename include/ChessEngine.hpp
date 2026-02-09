#pragma once
#include <optional>
#include <string>
#include <vector>
#include "Piece.hpp"

class ChessEngine {
public:
    std::optional<Piece> plateau[8][8];
    Color                current_player = Color::Blanc;
    std::string          message;

    ChessEngine();
    void reset();
    bool noMansLand(int fromX, int fromY, int toX, int toY);
    bool canPionMove(int fromX, int fromY, int toX, int toY);
    bool canIMove(int fromX, int fromY, int toX, int toY);
    void executeMove(int fromX, int fromY, int toX, int toY);
};