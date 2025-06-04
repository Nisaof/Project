// ChessBoard.hpp
#ifndef CHESS_BOARD_HPP
#define CHESS_BOARD_HPP
#include "ConfigReader.hpp"
#include <unordered_map>
#include <string>
#include <vector>

class MoveValidator;
class PortalSystem;
class GameManager;

class ChessBoard {
public:
  struct Square {
    std::string piece;
    bool is_white;
    //piece""
    Square() : piece(""), is_white(false) {}
    Square(const std::string& p, bool w) : piece(p), is_white(w) {}
    bool is_empty() const { return piece.empty(); }
  };

  ChessBoard(int size, const std::string& display_format = "detailed"); 
  int getBoardSize() const;
  void initializeBoard(const std::vector<PieceConfig>& piece_configs);
  void placePiece(const std::string& piece, bool is_white, int x, int y);
  void printBoard() const;
  bool isInBounds(const Position& pos) const;
  const Square& getSquare(const Position& pos) const;
  void movePiece(const Position& start, const Position& end, MoveValidator& validator, 
                 PortalSystem& portal_system, GameManager& game_manager);
  
  // special hareketler
  Position notationToPosition(const std::string& notation) const;
  std::string positionToNotation(const Position& pos) const;
  void handlePawnPromotion(const Position& pos, bool is_white);
  void handleCastling(const Position& king_start, const Position& king_end);

private:
  std::unordered_map<std::string, Square> board;
  int board_size;
  std::string board_display_format; 
  std::string posToString(const Position& pos) const;
};

#endif