//#pragma once
#ifndef CHESSTYPE_H
#define CHESSTYPE_H

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <direct.h>
#include <fstream>
#include <functional>
#include <io.h>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace PieceSpace {
class Piece;
class Pieces;
class PieceManager;
}

namespace SeatSpace {
class Seat;
class Seats;
class SeatManager;
}

namespace BoardSpace {
class Board;
}

namespace ChessManualSpace {
class ChessManual;
}

using namespace std;
using namespace PieceSpace;
using namespace SeatSpace;
using namespace BoardSpace;
using namespace ChessManualSpace;

enum class PieceColor {
    RED,
    BLACK
};

enum class PieceKind {
    KING,
    ADVISOR,
    BISHOP,
    KNIGHT,
    ROOK,
    CANNON,
    PAWN
};

enum class ChangeType {
    EXCHANGE,
    ROTATE,
    SYMMETRY
};

enum class RecFormat {
    XQF,
    BIN,
    JSON,
    PGN_ICCS,
    PGN_ZH,
    PGN_CC
};

typedef shared_ptr<PieceSpace::Piece> SPiece;

typedef shared_ptr<SeatSpace::Seat> SSeat;
typedef pair<const SSeat&, const SSeat&> SSeat_pair;
typedef vector<SSeat> SSeat_vector;

typedef pair<int, int> RowCol_pair;
typedef pair<pair<int, int>, pair<int, int>> PRowCol_pair;
typedef vector<RowCol_pair> RowCol_pair_vector;
typedef vector<PRowCol_pair> PRowCol_pair_vector;

typedef shared_ptr<BoardSpace::Board> SBoard;
typedef shared_ptr<ChessManualSpace::ChessManual> SChessManual;

constexpr auto BLANKNAME = L'\x0';
constexpr auto BLANKCOL = -1;
constexpr auto PIECENUM = 32;
constexpr auto BOARDROWNUM = 10;
constexpr auto BOARDCOLNUM = 9;
constexpr auto SEATNUM = BOARDROWNUM * BOARDCOLNUM;

#endif
