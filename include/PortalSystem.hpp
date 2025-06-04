#ifndef PORTAL_SYSTEM_HPP
#define PORTAL_SYSTEM_HPP
#include "ChessBoard.hpp"
#include "ConfigReader.hpp"
#include <queue>
#include <unordered_map>
#include <string>

class PortalSystem {
public:
    PortalSystem(const std::vector<PortalConfig>& portals);
    bool isPortalMove(const Position& start, const Position& end) const;
    bool validatePortalMove(const std::string& piece, const Position& start, 
                           const Position& end, bool is_white_turn, const ChessBoard& board) const;
    void handlePortalMove(const Position& start, const Position& end, ChessBoard& board);
    void updateCooldowns();
    bool isPortalInCooldown(const Position& start, const Position& end) const;
    const std::vector<PortalConfig>& getPortals() const { return portals_; }

private:
    std::vector<PortalConfig> portals_;
    std::queue<std::string> cooldown_queue_;
    std::unordered_map<std::string, int> cooldowns_;
};

#endif