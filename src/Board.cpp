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
    bool isBottom{ isBottomSide(color) };
    PieceColor othColor = PieceManager::getOtherColor(color);
    RowCol_pair krowcol_pair{ seats_->getKingSeat(isBottom)->rowCol_pair() },
        orowcol_pair{ seats_->getKingSeat(!isBottom)->rowCol_pair() };
    int fcol{ krowcol_pair.second };
    if (fcol == orowcol_pair.second) {
        int krow{ krowcol_pair.first }, orow{ orowcol_pair.first },
            lrow{ isBottom ? krow : orow }, urow{ isBottom ? orow : krow };
        bool killed{ true };
        for (int row = lrow + 1; row < urow; ++row)
            if (seats_->getSeat(row, fcol)->piece()) { // 空棋子则继续循环
                killed = false; // 全部是空棋子，则将帅对面
                break;
            }
        if (killed)
            return true;
    }
    // '获取某方可杀将棋子全部可走的位置
    for (auto& frowcol_p : seats_->getLiveRowCols(othColor, BLANKNAME, BLANKCOL, true)) {
        auto mvRowCols = seats_->getMoveRowCols(isBottom, frowcol_p);
        if (!mvRowCols.empty() && find(mvRowCols.begin(), mvRowCols.end(), krowcol_pair) != mvRowCols.end()) // 对方强子可走位置有本将位置
            return true;
    }
    return false;
}

bool Board::isDied(PieceColor color) const
{
    for (auto& frowcol_p : seats_->getLiveRowCols(color))
        if (!getCanMoveRowCols(frowcol_p).empty()) // 本方还有棋子可以走
            return false;
    return true;
}

PRowCol_pair Board::getPRowCol_pair(const wstring& str) const
{
    assert(str.size() == 4);
    RowCol_pair frowcol_pair, trowcol_pair;
    RowCol_pair_vector rowcol_pv{};
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ PieceManager::getColorFromZh(str.back()) };
    bool isBottom{ isBottomSide(color) };
    int index{}, movDir{ PieceManager::getMovNum(isBottom, str.at(2)) };
    wchar_t name{ str.front() };

    if (PieceManager::isPiece(name)) { // 首字符为棋子名
        rowcol_pv = seats_->getLiveRowCols(color, name,
            PieceManager::getCurIndex(isBottom,
                PieceManager::getNumIndex(color, str.at(1)), BOARDCOLNUM));
        assert(rowcol_pv.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (rowcol_pv.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = str.at(1);
        rowcol_pv = (PieceManager::isPawn(name)
                ? seats_->getSortPawnLiveRowCols(isBottom, color, name)
                : seats_->getLiveRowCols(color, name));
        index = PieceManager::getPreIndex(rowcol_pv.size(), isBottom, str.front());
    }

    assert(index <= static_cast<int>(rowcol_pv.size()) - 1);
    frowcol_pair = rowcol_pv.at(index);

    int numIndex{ PieceManager::getNumIndex(color, str.back()) },
        toCol{ PieceManager::getCurIndex(isBottom, numIndex, BOARDCOLNUM) };
    if (PieceManager::isLineMove(name))
        trowcol_pair = (movDir == 0
                ? make_pair(frowcol_pair.first, toCol)
                : make_pair(frowcol_pair.first + movDir * (numIndex + 1), frowcol_pair.second));
    else { // 斜线走子：仕、相、马
        int colAway{ abs(toCol - frowcol_pair.second) }; //  相距1或2列
        trowcol_pair = make_pair(
            frowcol_pair.first + movDir * (PieceManager::isAdvisor(name) || PieceManager::isBishop(name) ? colAway : (colAway == 1 ? 2 : 1)),
            toCol);
    }

    //assert(str == getZHStr(make_pair(frowcol_pair, trowcol_pair), RecFormat::PGN_ZH));
    return make_pair(frowcol_pair, trowcol_pair);
}

//中文纵线着法
const wstring Board::getZHStr(PRowCol_pair prowcol_pair) const
{
    wostringstream wos{};
    const SPiece& fromPiece{ seats_->getSeat(prowcol_pair.first)->piece() };
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->name() };
    int fromRow{ prowcol_pair.first.first }, fromCol{ prowcol_pair.first.second },
        toRow{ prowcol_pair.second.first }, toCol{ prowcol_pair.second.second };
    bool isSameRow{ fromRow == toRow }, isBottom{ isBottomSide(color) };
    auto rowcol_pv = seats_->getLiveRowCols(color, name, fromCol);

    if (rowcol_pv.size() > 1 && PieceManager::isStronge(name)) {
        if (PieceManager::isPawn(name))
            rowcol_pv = seats_->getSortPawnLiveRowCols(isBottom, color, name);
        wos << PieceManager::getPreChar(rowcol_pv.size(), isBottom,
                   distance(rowcol_pv.begin(), find(rowcol_pv.begin(), rowcol_pv.end(), prowcol_pair.first)))
            << name;
    } else //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wos << name << PieceManager::getColChar(color, isBottom, fromCol);
    wos << PieceManager::getMovChar(isSameRow, isBottom, toRow > fromRow)
        << (PieceManager::isLineMove(name) && !isSameRow
                   ? PieceManager::getNumChar(color, abs(fromRow - toRow)) // 非同一行
                   : PieceManager::getColChar(color, isBottom, toCol));

    //assert(getPRowCol_pair(wos.str(), RecFormat::PGN_ZH) == prowcol_pair);
    return wos.str();
}

const RowCol_pair_vector Board::getPutRowCols(const SPiece& piece) const
{
    return seats_->getPutRowCols(isBottomSide(piece->color()), piece);
}

const RowCol_pair_vector Board::getCanMoveRowCols(RowCol_pair rowcol_pair) const
{
    auto& fseat = seats_->getSeat(rowcol_pair);
    assert(fseat->piece());
    PieceColor color{ fseat->piece()->color() };
    auto rowcol_pv = seats_->getMoveRowCols(isBottomSide(color), rowcol_pair);
    auto pos = remove_if(rowcol_pv.begin(), rowcol_pv.end(),
        [&](const RowCol_pair& rowcol_p) {
            // 移动棋子后，检测是否会被对方将军
            auto& tseat = seats_->getSeat(rowcol_p);
            auto eatPiece = fseat->movTo(tseat);
            bool killed{ isKilled(color) };
            tseat->movTo(fseat, eatPiece);
            return killed;
        });
    return RowCol_pair_vector{ rowcol_pv.begin(), pos };
}

const RowCol_pair_vector Board::getLiveRowCols(PieceColor color) const
{
    return seats_->getLiveRowCols(color);
}

const SPiece Board::doneMove(PRowCol_pair prowcol_pair) const
{
    return seats_->getSeat(prowcol_pair.first)->movTo(seats_->getSeat(prowcol_pair.second));
}

void Board::undoMove(PRowCol_pair prowcol_pair, const SPiece& eatPie) const
{
    seats_->getSeat(prowcol_pair.second)->movTo(seats_->getSeat(prowcol_pair.first), eatPie);
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
    for (auto color : { PieceColor::BLACK, PieceColor::RED })
        for (auto& frowcol_p : seats_->getLiveRowCols(color))
            textBlankBoard[(BOARDROWNUM - 1 - frowcol_p.first) * 2 * (BOARDCOLNUM * 2) + frowcol_p.second * 2]
                = PieceManager::getPrintName(*seats_->getSeat(frowcol_p)->piece());
    wos << textBlankBoard;
    return wos.str();
}

void Board::__setBottomSide()
{
    bottomColor_ = seats_->getKingSeat(true)->piece()->color();
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
                        << getRowColsStr(board.getCanMoveRowCols(rowcol_pair)) << L'\n';
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
