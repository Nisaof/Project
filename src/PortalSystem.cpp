#include "PortalSystem.hpp"
#include <algorithm>
#include <iostream>

PortalSystem::PortalSystem(const std::vector<PortalConfig>& portals) : portals_(portals) {
    for (const auto& portal : portals_) {
        cooldowns_[portal.id] = 0;
    }
}

bool PortalSystem::isPortalMove(const Position& start, const Position& end) const {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            return true;
        }
    }
    return false;
}

bool PortalSystem::validatePortalMove(const std::string& piece, const Position& start, 
                                     const Position& end, bool is_white_turn, 
                                     const ChessBoard& board) const {
    const auto& square = board.getSquare(start);
    if (square.is_empty() || square.piece != piece || square.is_white != is_white_turn) {
        return false;
    }

    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            
            // Önce cooldown kontrolü
            if (isPortalInCooldown(start, end)) {
                return false;
            }
            
            // Renklere göre kurallara bkıyor
            std::string color = is_white_turn ? "white" : "black";
            auto it = std::find(portal.properties.allowed_colors.begin(), 
                               portal.properties.allowed_colors.end(), color);
            if (it == portal.properties.allowed_colors.end()) {
                std::cout << "\nPortal Hatası: Bu portal " << color << " taşlar için kullanılamaz!" << std::endl;
                return false;
            }
            
            return true;
        }
    }
    return false;
}

void PortalSystem::handlePortalMove(const Position& start, const Position& end, ChessBoard& board) {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            auto square = board.getSquare(start);
            if (!square.is_empty()) {
                board.placePiece(square.piece, square.is_white, end.x, end.y);
                board.placePiece("", false, start.x, start.y);
                
                // cooldown sayısı
                cooldowns_[portal.id] = portal.properties.cooldown;
                
                //queueya ekliyor
                for (int i = 0; i < portal.properties.cooldown; i++) {
                    cooldown_queue_.push(portal.id);
                }

                /*
                std::cout << "\n--- Portal Kullanıldı ---";
                std::cout << "\nPortal: " << portal.id;
                std::cout << "\nCooldown: " << cooldowns_[portal.id] << " tur";
                std::cout << "\nDurum: Bu portal " << cooldowns_[portal.id] 
                         << " tur boyunca hiçbir taş tarafından kullanılamaz.";
                std::cout << "\nGiriş: (" << start.x << "," << start.y << ")";
                std::cout << "\nÇıkış: (" << end.x << "," << end.y << ")" << std::endl;*/
            }
            break;
        }
    }
}

bool PortalSystem::isPortalInCooldown(const Position& start, const Position& end) const {
    for (const auto& portal : portals_) {
        if (start.x == portal.positions.entry.x && start.y == portal.positions.entry.y &&
            end.x == portal.positions.exit.x && end.y == portal.positions.exit.y) {
            auto cooldown_it = cooldowns_.find(portal.id);
            if (cooldown_it != cooldowns_.end() && cooldown_it->second > 0) {
                std::cout << "\nPortal " << portal.id << "cooldownda! "
                          << "Kalan tur: " << cooldown_it->second << " tur" << std::endl;
                std::cout << "Bu portal şu anda hiçbir taş tarafından kullanılamaz." << std::endl;
                return true;
            }
            return false;
        }
    }
    return false;
}

void PortalSystem::updateCooldowns() {
    if (cooldown_queue_.empty()) {
        return;
    }

    // Her turda bir portal'ın cooldown'ını azalt
    std::string portal_id = cooldown_queue_.front();
    cooldown_queue_.pop();
    
    auto it = cooldowns_.find(portal_id);
    if (it != cooldowns_.end() && it->second > 0) {
        it->second--;
        if (it->second == 0) {
            std::cout << "\nPortal " << portal_id << " artık kullanıma hazır!" << std::endl;
        }
    }
    
    // Cooldown durumlarını göster
    bool has_cooldowns = false;
    for (const auto& pair : cooldowns_) {
        if (pair.second > 0) {
            if (!has_cooldowns) {
                std::cout << "\n--- PORTAL COOLDOWN DURUMLARI ---";
                has_cooldowns = true;
            }
            std::cout << "\n" << pair.first << " -> Kalan bekleme süresi: " << pair.second << " tur";
        }
    }
    if (has_cooldowns) {
        std::cout << "\n!!" << std::endl;
    }
}