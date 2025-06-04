#include "ChessBoard.hpp"
#include "ConfigReader.hpp"
#include "MoveValidator.hpp"
#include "PortalSystem.hpp"
#include "GameManager.hpp"
#include <iostream>
#include <string>
#include <sstream>
#include <cctype>

// Konum dizesini ayrıştırma (ör. "a1" -> Position{0, 0})
bool parsePosition(const std::string& pos_str, Position& pos, int board_size) {
  if (pos_str.length() < 2) {
    std::cerr << "Geçersiz pozisyon\n";
    return false;
  }
  char col = std::tolower(pos_str[0]);
  std::string row_str = pos_str.substr(1);
  if (col < 'a' || col >= 'a' + board_size) {
    std::cerr << "Geçersiz pozisyon\n";
    return false;
  }
  try {
    int row = std::stoi(row_str) - 1; // Kullanıcı 1-8 girer pc 0-7 kullanır
    if (row < 0 || row >= board_size) {
      std::cerr << "Geçersiz pozisyon\n";
      return false;
    }
    pos = {col - 'a', row};
    return true;
  } catch (...) {
    std::cerr << "Geçersiz pozisyon\n";
    return false;
  }
}

// Komut satırı girişi
bool processMoveCommand(const std::string& command, ChessBoard& board, 
                        MoveValidator& validator, PortalSystem& portal_system, 
                        GameManager& game_manager, bool is_white_turn) {
  std::istringstream iss(command);
  std::string cmd, start_str, end_str, piece;
  iss >> cmd >> start_str >> end_str >> piece;
  if (cmd != "move" || start_str.empty() || end_str.empty() || piece.empty()) {
    std::cout << "Geçersiz komut. Örnek: move a1 b2 king\n";
    return false;
  }

  Position start, end;
  if (!parsePosition(start_str, start, board.getBoardSize()) || 
      !parsePosition(end_str, end, board.getBoardSize())) {
    std::cout << "Geçersiz pozisyon. Örnek: a1, b2 (sınırlar içinde)\n";
    return false;
  }

  // Tahta üzerindeki taşın rengini kontrol et
  const auto& start_square = board.getSquare(start);
  if (start_square.is_empty()) {
    std::cout << "Başlangıç pozisyonunda taş yok.\n";
    return false;
  }

  // Taşın sırayla uyumlu olduğunu kontrol et
  if (start_square.is_white != is_white_turn) {
    std::cout << (is_white_turn ? "Beyaz" : "Siyah") << " oyuncunun sırası. "
              << (start_square.is_white ? "Beyaz" : "Siyah") << " taş seçildi.\n";
    return false;
  }

  // Taş türünün girişle uyumlu olduğunu kontrol et
  std::string piece_lower = validator.toLowerCase(piece);
  std::string square_piece_lower = validator.toLowerCase(start_square.piece);
  if (piece_lower != square_piece_lower) {
    std::cout << "Başlangıç pozisyonundaki taş (" << start_square.piece
              << ") ile belirtilen taş (" << piece << ") uyuşmuyor.\n";
    return false;
  }

  // Hareketi doğrula ve uygula
  if (validator.isValidMove(piece, start, end, start_square.is_white, board, portal_system)) {
    board.movePiece(start, end, validator, portal_system, game_manager);
    std::cout << "Hareket başarılı: " << start_str << " -> " << end_str << "\n";
    board.printBoard();
    return true;
  } else {
    std::cout << "Geçersiz hareket: " << piece << " için " << start_str << " -> " << end_str << "\n";
    return false;
  }
}

int main(int argc, char* argv[]) {
  if (!std::cin.good()) {
    std::cerr << "Giriş hatası\n";
    return 1;
  }

  std::string config_file = (argc > 1) ? argv[1] : "data/chess_pieces.json";
  ConfigReader config_reader;
  if (!config_reader.loadFromFile(config_file)) {
    std::cerr << "Yapılandırma dosyası yüklenemedi\n";
    return 1;
  }

  std::string display_format = (argc > 2 && std::string(argv[2]) == "simple") ? "simple" : "detailed";
  int board_size = config_reader.getConfig().game_settings.board_size;
  
  if (board_size <= 0 || board_size > 26) {
    std::cerr << "Geçersiz tahta boyutu\n";
    return 1;
  }

  ChessBoard board(board_size, display_format);
  board.initializeBoard(config_reader.getConfig().pieces);
  MoveValidator validator;
  PortalSystem portal_system(config_reader.getConfig().portals);
  GameManager game_manager(board, validator, portal_system);

  std::cout << "Başlangıç tahtası:\n";
  board.printBoard();
  std::cout << "Komutlar: move <başlangıç> <hedef> <taş> (ör. move a1 b2 king), undo, quit\n";

  bool is_white_turn = true;
  std::string command;
  while (true) {
    std::cout << (is_white_turn ? "Beyaz" : "Siyah") << " oyuncunun sırası > ";
    std::cout.flush();
    
    if (!std::getline(std::cin, command)) {
      break;
    }

    if (command == "quit") {
      std::cout << "Oyun sona erdi.\n";
      break;
    }

    if (command == "undo") {
      game_manager.undoMove();
      board.printBoard();
      is_white_turn = !is_white_turn;
      continue;
    }

    if (!command.empty()) {
      if (processMoveCommand(command, board, validator, portal_system, game_manager, is_white_turn)) {
        if (game_manager.isCheckmate(!is_white_turn)) {
          std::cout << (is_white_turn ? "Beyaz" : "Siyah") << " şah mat yaptı! Oyun bitti.\n";
          break;
        }
        if (game_manager.isStalemate(!is_white_turn)) {
          std::cout << "Oyun berabere bitti.\n";
          break;
        }
        is_white_turn = !is_white_turn;
      }
    } else {
      std::cout << "Boş komut. Örnek: move a1 b2 king\n";
    }
  }

  return 0;
}