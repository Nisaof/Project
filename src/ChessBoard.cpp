// ChessBoard.cpp
#include "ChessBoard.hpp"
#include "MoveValidator.hpp"
#include "PortalSystem.hpp"
#include "GameManager.hpp"
#include <iostream>
#include <stdexcept>
#include <cctype>

ChessBoard::ChessBoard(int size, const std::string& display_format) 
    : board_size(size), board_display_format(display_format) {}

int ChessBoard::getBoardSize() const {
  return board_size;
}

std::string ChessBoard::posToString(const Position& pos) const {
  return std::to_string(pos.x) + "," + std::to_string(pos.y);
}

bool ChessBoard::isInBounds(const Position& pos) const {
  return pos.x >= 0 && pos.x < board_size && pos.y >= 0 && pos.y < board_size;
}

const ChessBoard::Square& ChessBoard::getSquare(const Position& pos) const {
  if (!isInBounds(pos)) {
    throw std::out_of_range("Tahta sınırlarının dışı.");
  }
  static const Square empty_square;
  auto it = board.find(posToString(pos));
  return it != board.end() ? it->second : empty_square;
}

void ChessBoard::placePiece(const std::string& piece, bool is_white, int x, int y) {
  if (!isInBounds({x, y})) {
    throw std::invalid_argument("Geçersiz pozisyon.");
  }
  std::string pos_str = posToString({x, y});
  if (piece.empty()) {
    board.erase(pos_str);
  } else {
    board[pos_str] = Square(piece, is_white);
  }
}

void ChessBoard::initializeBoard(const std::vector<PieceConfig>& piece_configs) {
  board.clear();
  for (const auto& config : piece_configs) {
    if (config.positions.find("white") != config.positions.end()) {
      for (const auto& pos : config.positions.at("white")) {
        if (isInBounds(pos)) {
          placePiece(config.type, true, pos.x, pos.y);
        }
      }
    }
    if (config.positions.find("black") != config.positions.end()) {
      for (const auto& pos : config.positions.at("black")) {
        if (isInBounds(pos)) {
          placePiece(config.type, false, pos.x, pos.y);
        }
      }
    }
  }
}

void ChessBoard::movePiece(const Position& start, const Position& end, 
                          MoveValidator& validator, PortalSystem& portal_system, 
                          GameManager& game_manager) {
    if (!isInBounds(start) || !isInBounds(end)) {
        throw std::invalid_argument("Geçersiz pozisyon.");
    }

    const auto& start_square = getSquare(start);

    if (!validator.isValidMove(start_square.piece, start, end, start_square.is_white, 
                              *this, portal_system)) {
        throw std::invalid_argument("Geçersiz hareket.");
    }

    std::string captured_piece = getSquare(end).piece;
    bool captured_piece_color = getSquare(end).is_white;

    
    std::string start_str = posToString(start);
    std::string end_str = posToString(end);
    if (board.find(start_str) == board.end()) {
        throw std::invalid_argument("Başlangıç pozisyonunda taş yok.");
    }
    board[end_str] = board[start_str];
    board.erase(start_str);


    if (validator.toLowerCase(start_square.piece) == "pawn") {
        bool is_promotion_rank = (start_square.is_white && end.y == 7) || 
                               (!start_square.is_white && end.y == 0);
        if (is_promotion_rank) {
            handlePawnPromotion(end, start_square.is_white);
        }
    }

    // En passant 
    if (validator.toLowerCase(start_square.piece) == "pawn" && 
        abs(end.x - start.x) == 1 && getSquare(end).is_empty()) {
        Position captured_pawn_pos = {end.x, start.y};
        if (!getSquare(captured_pawn_pos).is_empty()) {
            std::string captured_str = posToString(captured_pawn_pos);
            captured_piece = board[captured_str].piece;
            captured_piece_color = board[captured_str].is_white;
            board.erase(captured_str);
            std::cout << "\nEn passantla piyon alındı." << std::endl;
        }
    }

    // Rok 
    if (validator.toLowerCase(start_square.piece) == "king" && 
        abs(end.x - start.x) == 2) {
        handleCastling(start, end);
    }

    // undo için
    game_manager.addToMoveHistory({start, end, start_square.piece, start_square.is_white, 
                                  captured_piece, captured_piece_color});

    // Portal kontrolü 
    for (const auto& portal : portal_system.getPortals()) {
        if (end.x == portal.positions.entry.x && end.y == portal.positions.entry.y) {
            // ışınlandı
            Position portal_exit = portal.positions.exit;
            
            
            if (!portal_system.isPortalInCooldown(end, portal_exit) &&
                portal_system.validatePortalMove(start_square.piece, end, portal_exit, 
                                               start_square.is_white, *this)) {
                std::cout << "\n!!Portal!!" << std::endl;
                
                
                std::string exit_str = posToString(portal_exit);
                board[exit_str] = board[end_str];
                board.erase(end_str);
                
                // stacke
                game_manager.addToMoveHistory({end, portal_exit, start_square.piece, 
                                             start_square.is_white, "", false});
                
                
                portal_system.handlePortalMove(end, portal_exit, *this);
                break;
            }
        }
    }

   
    portal_system.updateCooldowns();
}
//DÖNNNNN
void ChessBoard::handlePawnPromotion(const Position& pos, bool is_white) {
    std::string promoted_piece;
    std::cout << "\nPiyon terfi ediyor! Seçenekler: Queen, Rook, Bishop, Knight" << std::endl;
    std::cout << "Terfi etmek istediğiniz taşı seçin: ";
    std::cin >> promoted_piece;

    // İlk harfi büyük, geri kalanı küçük yap
    promoted_piece[0] = std::toupper(promoted_piece[0]);
    for (size_t i = 1; i < promoted_piece.length(); ++i) {
        promoted_piece[i] = std::tolower(promoted_piece[i]);
    }

    // Geçerli bir seçim mi kontrol et
    std::vector<std::string> valid_pieces = {"Queen", "Rook", "Bishop", "Knight"};
    while (std::find(valid_pieces.begin(), valid_pieces.end(), promoted_piece) == valid_pieces.end()) {
        std::cout << "Geçersiz seçim. Lütfen tekrar deneyin: ";
        std::cin >> promoted_piece;
        promoted_piece[0] = std::toupper(promoted_piece[0]);
        for (size_t i = 1; i < promoted_piece.length(); ++i) {
            promoted_piece[i] = std::tolower(promoted_piece[i]);
        }
    }

    // Piyonu terfi et
    std::string pos_str = posToString(pos);
    board[pos_str] = Square(promoted_piece, is_white);
    std::cout << (is_white ? "Beyaz" : "Siyah") << " piyon " << promoted_piece << " olarak terfi etti!" << std::endl;
}

void ChessBoard::handleCastling(const Position& king_start, const Position& king_end) {
    // Kısa rok mu uzun rok mu?
    bool is_kingside = king_end.x > king_start.x;
    
    // Kalenin başlangıç ve bitiş pozisyonları
    int rook_start_x = is_kingside ? 7 : 0;
    int rook_end_x = is_kingside ? 5 : 3;
    Position rook_start = {rook_start_x, king_start.y};
    Position rook_end = {rook_end_x, king_start.y};
    
    // Kaleyi hareket ettir
    std::string rook_start_str = posToString(rook_start);
    std::string rook_end_str = posToString(rook_end);
    board[rook_end_str] = board[rook_start_str];
    board.erase(rook_start_str);
    
    std::cout << "\nRok yapıldı!" << std::endl;
}

void ChessBoard::printBoard() const {
  if (board_display_format == "simple") {
    
    for (int y = 0; y < board_size; ++y) {
      for (int x = 0; x < board_size; ++x) {
        const Square& square = getSquare({x, y});
        if (square.is_empty()) {
          std::cout << ". ";
        } else {
          char symbol = square.piece[0];
          if (square.is_white) {
            symbol = std::toupper(symbol);
          }
          std::cout << symbol << " ";
        }
      }
      std::cout << "\n";
    }
  } else {
    // Sütun (a,h) satır (1,8)
    std::cout << "   ";
    for (int x = 0; x < board_size; ++x) {
      std::cout << (char)('a' + x) << "  ";
    }
    std::cout << std::endl;

    for (int y = board_size - 1; y >= 0; --y) {
      std::cout << (y + 1) << "  ";
      for (int x = 0; x < board_size; ++x) {
        Position pos{x, y};
        std::string key = posToString(pos);
        if (board.find(key) != board.end() && !board.at(key).is_empty()) {
          const auto& square = board.at(key);
          std::string piece_short;
          if (square.piece == "King") piece_short = square.is_white ? "WK" : "BK";
          else if (square.piece == "Queen") piece_short = square.is_white ? "WQ" : "BQ";
          else if (square.piece == "Rook") piece_short = square.is_white ? "WR" : "BR";
          else if (square.piece == "Bishop") piece_short = square.is_white ? "WB" : "BB";
          else if (square.piece == "Knight") piece_short = square.is_white ? "WA" : "BA";
          else if (square.piece == "Pawn") piece_short = square.is_white ? "WP" : "BP";
          else piece_short = square.is_white ? "XX" : "xx";
          std::cout << piece_short << " ";
        } else {
          std::cout << " . ";
        }
      }
      std::cout << (y + 1) << std::endl;
    }

    std::cout << "   ";
    for (int x = 0; x < board_size; ++x) {
      std::cout << (char)('a' + x) << "  ";
    }
    std::cout << std::endl;
  }
}

//Koordinat sistemine dönüşüm 
Position ChessBoard::notationToPosition(const std::string& notation) const {
    if (notation.length() != 2) {
        throw std::invalid_argument("Geçersiz.");
    }
    
    int x = std::tolower(notation[0]) - 'a';
    int y = notation[1] - '1';
    
    return Position{x, y};
}

std::string ChessBoard::positionToNotation(const Position& pos) const {
    if (!isInBounds(pos)) {
        throw std::invalid_argument("Geçersiz pozisyon.");
    }
    
    char file = 'a' + pos.x;
    char rank = '1' + pos.y;
    
    return std::string(1, file) + std::string(1, rank);
}