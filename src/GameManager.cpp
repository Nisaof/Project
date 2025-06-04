#include "GameManager.hpp"
#include "ChessBoard.hpp"
#include "MoveValidator.hpp"
#include "PortalSystem.hpp"
#include <stdexcept>
#include <iostream>

GameManager::GameManager(ChessBoard& board, MoveValidator& validator, PortalSystem& portal_system)
    : chess_board(board), validator(validator), portal_system(portal_system) {
}

bool GameManager::isInCheck(bool is_white_turn) const {
    
    Position king_position;
    bool king_found = false;

    //şah kısmı
    for (int y = 0; y < chess_board.getBoardSize() && !king_found; ++y) {
        for (int x = 0; x < chess_board.getBoardSize() && !king_found; ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (validator.toLowerCase(square.piece) == "king" && square.is_white == is_white_turn) {
                king_position = {x, y};
                king_found = true;
            }
        }
    }

    if (!king_found) {
        return false;
    }

    // Tehdit 
    const std::vector<std::string> threatening_pieces = {"queen", "rook", "bishop", "knight", "pawn"};
    
    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            // Sadece rakip taşları kontrol edicek
            if (!square.is_empty() && square.is_white != is_white_turn) {
                std::string piece_lower = validator.toLowerCase(square.piece);
                // Sadece tehditler
                if (std::find(threatening_pieces.begin(), threatening_pieces.end(), piece_lower) != threatening_pieces.end()) {
                    Position start = {x, y};
                    // Taşın şahı tehdit ediyor mu
                    if (validator.isValidMove(square.piece, start, king_position, !is_white_turn, 
                                           chess_board, portal_system)) {
                        return true;
                    }
                }
            }
        }
    }

    return false;
}

bool GameManager::isCheckmate(bool is_white_turn) {
    // Önce şah mı
    if (!isInCheck(is_white_turn)) {
        return false;
    }

    // Tüm taşları ve olası hamleleri 
    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (!square.is_empty() && square.is_white == is_white_turn) {
                Position start = {x, y};
                
                // Tüm olası hedef kareleri 
                for (int dy = 0; dy < chess_board.getBoardSize(); ++dy) {
                    for (int dx = 0; dx < chess_board.getBoardSize(); ++dx) {
                        Position end = {dx, dy};
                        if (start.x == end.x && start.y == end.y) continue;
                        
                        // Hamle geçerli mi 
                        if (validator.isValidMove(square.piece, start, end, is_white_turn,
                                               chess_board, portal_system)) {
                            // Geçici tahta oluştur 
                            ChessBoard temp_board = chess_board;
                            try {
                                // Hamleyi yap
                                auto captured_piece = temp_board.getSquare(end);
                                temp_board.placePiece(square.piece, square.is_white, end.x, end.y);
                                temp_board.placePiece("", false, start.x, start.y);
                                
                                // Yeni durumda şah devam ediyor mu kontrol et
                                GameManager temp_manager(temp_board, validator, portal_system);
                                if (!temp_manager.isInCheck(is_white_turn)) {
                                    return false; // Kurtarıcı hamle var
                                }
                            } catch (const std::exception& e) {
                                continue;
                            }
                        }
                    }
                }
            }
        }
    }

    return true; // kurtarıcı yok
}

bool GameManager::isStalemate(bool is_white_turn) const {
    if (isInCheck(is_white_turn)) {
        return false;
    }

    for (int y = 0; y < chess_board.getBoardSize(); ++y) {
        for (int x = 0; x < chess_board.getBoardSize(); ++x) {
            const auto& square = chess_board.getSquare({x, y});
            if (!square.is_empty() && square.is_white == is_white_turn) {
                Position start = {x, y};
                for (int dy = -chess_board.getBoardSize(); dy <= chess_board.getBoardSize(); ++dy) {
                    for (int dx = -chess_board.getBoardSize(); dx <= chess_board.getBoardSize(); ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        Position end = {x + dx, y + dy};
                        if (chess_board.isInBounds(end) && 
                            validator.isValidMove(square.piece, start, end, is_white_turn, 
                                                 chess_board, portal_system)) {
                            return false;
                        }
                    }
                }
            }
        }
    }

    return true;
}

void GameManager::addToMoveHistory(const Move& move) {
    move_history.push(move);
}

void GameManager::undoMove() {
    if (move_history.empty()) {
        std::cout << "No moves to undo." << std::endl;
        return;
    }

    Move last_move = move_history.top();
    move_history.pop();

    try {
        // Önce taşı geri alacak.
        chess_board.placePiece(last_move.moved_piece, last_move.moved_piece_color, 
                             last_move.start.x, last_move.start.y);
        
        // Eğer yenen bir taş varsa onu geri koyucak.
        if (!last_move.captured_piece.empty()) {
            chess_board.placePiece(last_move.captured_piece, last_move.captured_piece_color, 
                                 last_move.end.x, last_move.end.y);
        } else {
            // Yenen taş yoksa hedef kareyi boşaltıcak.
            chess_board.placePiece("", false, last_move.end.x, last_move.end.y);
        }

        std::cout << "Move undone: " << last_move.moved_piece << " from " 
                  << last_move.end.x << "," << last_move.end.y << " to "
                  << last_move.start.x << "," << last_move.start.y << std::endl;

        portal_system.updateCooldowns();
    } catch (const std::exception& e) {
        // Hata durumunda hamleyi geri alıcak ve hatayı bildiricek
        move_history.push(last_move);
        std::cerr << "Error undoing move: " << e.what() << std::endl;
    }
}