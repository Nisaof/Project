#pragma once
#include <string>
#include <map>
#include <vector>
#include <memory> 
#include "ConfigReader.hpp"

class Piece {
public:
    std::string type;
    int color;
    int x;
    int y;
    Movement movement;
    SpecialAbilities special_abilities;
    bool hasMoved;

    Piece(const std::string& type,
          int color,
          int x, int y,
          const Movement& movement,
          const SpecialAbilities& special_abilities,
          bool hasMoved = false) :
        type(type),
        color(color),
        x(x),
        y(y),
        movement(movement),
        special_abilities(special_abilities),
        hasMoved(hasMoved) {}

    // en son
    void move(int newX, int newY);
    bool canUsePortal() const;
    std::string getType() const;
};