#pragma once
#include <optional>

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
        if (type == PieceType::Tour)
            return (color == Color::Blanc) ? "T" : "t";
        if (type == PieceType::Cavalier)
            return (color == Color::Blanc) ? "C" : "c";
        if (type == PieceType::Fou)
            return (color == Color::Blanc) ? "F" : "f";
        if (type == PieceType::Reine)
            return (color == Color::Blanc) ? "Q" : "q";
        if (type == PieceType::Roi)
            return (color == Color::Blanc) ? "K" : "k";
        return "";
    }
};