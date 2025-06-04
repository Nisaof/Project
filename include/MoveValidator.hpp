// MoveValidator.hpp
#ifndef MOVE_VALIDATOR_HPP
#define MOVE_VALIDATOR_HPP
#include "ChessBoard.hpp"
#include "PortalSystem.hpp"
#include <string>
#include <vector>
#include <queue>
#include <unordered_set>

class MoveValidator {
public:
  MoveValidator() = default;
  bool isValidMove(const std::string& piece, const Position& start, const Position& end,
                   bool is_white, const ChessBoard& board, const PortalSystem& portal_system) const;

  std::string toLowerCase(const std::string& str) const;
private:
  
 
  std::vector<Position> getMoveEdges(const std::string& piece_lower, const Position& pos, 
                                     bool is_white, const ChessBoard& board) const;
  
  bool bfsValidateMove(const std::string& piece_lower, const Position& start, 
                       const Position& end, bool is_white, const ChessBoard& board, 
                       const PortalSystem& portal_system) const;
  
  // Special hareketler
  bool validateCastling(const Position& start, const Position& end, 
                       bool is_white, const ChessBoard& board) const;
  bool isEnPassantMove(const Position& start, const Position& end,
                      bool is_white, const ChessBoard& board) const;
};

#endif