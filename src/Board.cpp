#include "Board.h"
#include "Piece.h"
#include "Seat.h"

namespace BoardSpace {

/* ===== Board start. ===== */
Board::Board(const wstring& pieceChars)
    : bottomColor_{ PieceColor::RED }
    , pieces_{ make_shared<Pieces>() }
    , seats_{ make_shared<Seats>() } // make_shared:动态分配内存，初始化对象并指向它
{
    setPieces(pieceChars);
}

bool Board::isKilled(PieceColor color) const
{
    return seats_->isKilled(color == bottomColor_);
}

bool Board::isDied(PieceColor color) const
{
    return seats_->isDied(color == bottomColor_);
}

PRowCol_pair Board::getPRowCol_pair(const wstring& str) const
{
    return seats_->getPRowCol_pair(bottomColor_, str);
}

//中文纵线着法
const wstring Board::getZHStr(PRowCol_pair prowcol_pair) const
{
    return seats_->getZHStr(bottomColor_, prowcol_pair);
}

const RowCol_pair_vector Board::getPutRowCols(const SPiece& piece) const
{
    return seats_->getPutRowCols(bottomColor_, piece);
}

const RowCol_pair_vector Board::getMoveRowCols(RowCol_pair rowcol_pair) const
{
    return seats_->getMoveRowCols(bottomColor_, rowcol_pair);
}

const RowCol_pair_vector Board::getLiveRowCols(PieceColor color) const
{
    return seats_->getLiveRowCols(color);
}

const SPiece Board::doneMove(PRowCol_pair prowcol_pair) const
{
    return seats_->doneMove(prowcol_pair);
}

void Board::undoMove(PRowCol_pair prowcol_pair, const SPiece& eatPie) const
{
    seats_->undoMove(prowcol_pair, eatPie);
}

void Board::setPieces(const wstring& pieceChars)
{
    if (pieceChars.empty())
        return;
    seats_->setBoardPieces(pieces_->getBoardPieces(pieceChars));
    __setBottomSide();
}

void Board::changeSide(const ChangeType ct)
{
    seats_->changeSide(ct, pieces_);
    __setBottomSide();
}

const wstring Board::getPieceChars() const
{
    return seats_->getPieceChars();
}

const wstring Board::toString() const
{ // 文本空棋盘
    wstring textBlankBoard{ L"┏━┯━┯━┯━┯━┯━┯━┯━┓\n"
                            "┃　│　│　│╲│╱│　│　│　┃\n"
                            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                            "┃　│　│　│╱│╲│　│　│　┃\n"
                            "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┴─┴─┴─┴─┴─┴─┴─┨\n"
                            "┃　　　　　　　　　　　　　　　┃\n"
                            "┠─┬─┬─┬─┬─┬─┬─┬─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─┼─╬─┼─╬─┼─╬─┼─┨\n"
                            "┃　│　│　│　│　│　│　│　┃\n"
                            "┠─╬─┼─┼─┼─┼─┼─╬─┨\n"
                            "┃　│　│　│╲│╱│　│　│　┃\n"
                            "┠─┼─┼─┼─╳─┼─┼─┼─┨\n"
                            "┃　│　│　│╱│╲│　│　│　┃\n"
                            "┗━┷━┷━┷━┷━┷━┷━┷━┛\n" }; // 边框粗线
    wostringstream wos{};
    /*// Pieces test
    wos << L"pieces_:\n"
        << pieces_->toString() << L"\n";
    // RowCols test
    wos << L"seats_:\n"
        << seats_->toString() << L"\n";
    //*/

    int index{ 0 };
    auto pieceChars = seats_->getPieceChars();
    for (const auto& rowcol_p : SeatManager::getAllRowCols()) {
        auto ch = pieceChars[index++];
        if (ch != PieceManager::nullChar())
            textBlankBoard[(BOARDROWNUM - 1 - rowcol_p.first) * 2 * (BOARDCOLNUM * 2) + rowcol_p.second * 2]
                = PieceManager::getPrintName(ch);
    }
    wos << textBlankBoard;
    return wos.str();
}

void Board::__setBottomSide()
{
    bottomColor_ = seats_->getSideColor(true);
}
/* ===== Board end. ===== */

const wstring FENplusToFEN(const wstring& FENplus)
{
    return FENplus.substr(0, FENplus.find(L' '));
}

const wstring FENToFENplus(const wstring& FEN, PieceColor color)
{
    return (FEN + L" " + (color == PieceColor::RED ? L"r" : L"b") + L" - - 0 1");
}

const wstring pieCharsToFEN(const wstring& pieceChars)
{
    assert(pieceChars.size() == SEATNUM);
    wstring fen{};
    wregex linerg{ LR"(.{9})" };
    for (wsregex_token_iterator lineIter{
             pieceChars.begin(), pieceChars.end(), linerg, 0 },
         end{};
         lineIter != end; ++lineIter) {
        wostringstream wos{};
        int num{ 0 };
        for (auto wch : (*lineIter).str()) {
            if (wch != PieceManager::nullChar()) {
                if (num) {
                    wos << num;
                    num = 0;
                }
                wos << wch;
            } else
                ++num;
        }
        if (num)
            wos << num;
        fen.insert(0, wos.str()).insert(0, L"/");
    }
    fen.erase(0, 1);

    //assert(FENTopieChars(fen) == pieceChars);
    return fen;
}

const wstring FENTopieChars(const wstring& fen)
{
    wstring pieceChars{};
    wregex linerg{ LR"(/)" };
    for (wsregex_token_iterator lineIter{ fen.begin(), fen.end(), linerg, -1 };
         lineIter != wsregex_token_iterator{}; ++lineIter) {
        wostringstream wos{};
        for (auto wch : wstring{ *lineIter })
            wos << (isdigit(wch)
                    ? wstring(wch - L'0', PieceManager::nullChar())
                    : wstring{ wch }); // ASCII: 0:48
        pieceChars.insert(0, wos.str());
    }

    assert(fen == pieCharsToFEN(pieceChars));
    return pieceChars;
}

const string getExtName(const RecFormat fmt)
{
    switch (fmt) {
    case RecFormat::XQF:
        return ".xqf";
    case RecFormat::BIN:
        return ".bin";
    case RecFormat::JSON:
        return ".json";
    case RecFormat::PGN_ICCS:
        return ".pgn_iccs";
    case RecFormat::PGN_ZH:
        return ".pgn_zh";
    case RecFormat::PGN_CC:
        return ".pgn_cc";
    default:
        return ".pgn_cc";
    }
}

RecFormat getRecFormat(const string& ext)
{
    if (ext == ".xqf")
        return RecFormat::XQF;
    else if (ext == ".bin")
        return RecFormat::BIN;
    else if (ext == ".json")
        return RecFormat::JSON;
    else if (ext == ".pgn_iccs")
        return RecFormat::PGN_ICCS;
    else if (ext == ".pgn_zh")
        return RecFormat::PGN_ZH;
    else if (ext == ".pgn_cc")
        return RecFormat::PGN_CC;
    else
        return RecFormat::PGN_CC;
}

const wstring testBoard()
{
    wostringstream wos{};
    Board board{};
    for (auto& fen : { PieceManager::FirstFEN(),
             wstring{ L"5a3/4ak2r/6R2/8p/9/9/9/B4N2B/4K4/3c5" } }) {
        auto pieceChars = FENTopieChars(fen);

        board.setPieces(pieceChars);
        wos << "     fen:" << fen
            << "\nchar_FEN:" << pieCharsToFEN(pieceChars)
            << "\ngetChars:" << pieceChars
            << "\nboardGet:" << board.getPieceChars() << L'\n';
        //*
        auto __printCanMoveRowCols = [&](void) {
            wos << L'\n' << board.toString() << flush;

            for (auto color : { PieceColor::RED, PieceColor::BLACK }) {
                auto rowcols = board.getLiveRowCols(color);
                wos << L"isBottomSide: " << boolalpha << board.isBottomSide(color) << L'\n'
                    << getRowColsStr(rowcols) << L'\n';
                for (auto& rowcol_pair : rowcols)
                    wos << L"From:" << rowcol_pair.first << rowcol_pair.second << L" CanMovtTo: "
                        << getRowColsStr(board.getMoveRowCols(rowcol_pair)) << L'\n';
            }
        };
        __printCanMoveRowCols();
        //*/
        //*
        for (const auto chg : {
                 ChangeType::EXCHANGE, ChangeType::ROTATE, ChangeType::SYMMETRY }) { //
            board.changeSide(chg);
            __printCanMoveRowCols();
        }
        //*/
        wos << L"\n";
    }
    return wos.str();
}
}
