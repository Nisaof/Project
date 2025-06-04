//#ifndef GAME_MANAGER_HPP
#define GAME_MANAGER_HPP
#include <stack>
#include "ConfigReader.hpp"


class ChessBoard;
class MoveValidator;
class PortalSystem;

class GameManager {
public:
    struct Move {
    Position start;
    Position end;
    std::string moved_piece;
    bool moved_piece_color; // true white, false black
    std::string captured_piece;
    bool captured_piece_color;
};


    GameManager(ChessBoard& board, MoveValidator& validator, PortalSystem& portal_system);
    bool isInCheck(bool is_white_turn) const;
    bool isCheckmate(bool is_white_turn);
    bool isStalemate(bool is_white_turn) const;
    void addToMoveHistory(const Move& move);
    void undoMove(); 

private:
    ChessBoard& chess_board;
    MoveValidator& validator;
    PortalSystem& portal_system; 
    std::stack<Move> move_history;
};

//#endif