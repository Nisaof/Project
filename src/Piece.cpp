#include "Piece.hpp"
#include "ConfigReader.hpp"
#include <vector>
#include <memory>

// Piece sınıfının kurucu fonksiyonu
/*Piece::Piece(const std::string& type,
             int color,
             int x, int y,
             const Movement& movement,
             const SpecialAbilities& abilities,
             bool hasMoved)
    : type(type), color(color), x(x), y(y), movement(movement), special_abilities(abilities), hasMoved(hasMoved) {}
*/
void Piece::move(int newX, int newY) {
    x = newX;
    y = newY;
    hasMoved = true;
}

bool Piece::canUsePortal() const {
    if (special_abilities.custom_abilities.find("portal_master") != special_abilities.custom_abilities.end()) {
        return special_abilities.custom_abilities.at("portal_master");
    }
    return false;
}

/*std::string Piece::getType() const {
    return type;
}*/

// ConfigReader'dan gelen taş bilgileriyle Piece nesneleri oluşturur
std::vector<std::shared_ptr<Piece>> createPiecesFromConfig(const ConfigReader& configReader) {
    std::vector<std::shared_ptr<Piece>> pieces;
 
    // Standart ve özel taş konfigürasyonlarını al
    auto pieceConfigs = configReader.getConfig().pieces;
    auto customPieceConfigs = configReader.getConfig().custom_pieces;

    // Standart taşlar
    for (const auto& pc : pieceConfigs) {
        for (const auto& colorEntry : pc.positions) {
            int color = (colorEntry.first == "white") ? 0 : 1;
            for (const auto& pos : colorEntry.second) {
                int x = pos.x;
                int y = pos.y;
                bool hasMoved = false;
                auto piecePtr = std::make_shared<Piece>(pc.type, color, x, y, pc.movement, pc.special_abilities, hasMoved);
                pieces.push_back(piecePtr);
            }
        }
    }
    // Özel taşlar
    for (const auto& pc : customPieceConfigs) {
        for (const auto& colorEntry : pc.positions) {
            int color = (colorEntry.first == "white") ? 0 : 1;
            for (const auto& pos : colorEntry.second) {
                int x = pos.x;
                int y = pos.y;
                bool hasMoved = false;
                auto piecePtr = std::make_shared<Piece>(pc.type, color, x, y, pc.movement, pc.special_abilities, hasMoved);
                pieces.push_back(piecePtr);
            }
        }
    }
    return pieces;
}


