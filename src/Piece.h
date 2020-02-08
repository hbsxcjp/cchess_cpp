//#pragma once
#ifndef PIECE_H
#define PIECE_H
 
#include "ChessType.h"

namespace PieceSpace {

// 棋子类
class Piece {

public:
    explicit Piece(wchar_t ch);

    wchar_t ch() const { return ch_; }
    wchar_t name() const { return name_; }
    PieceColor color() const { return color_; }
    PieceKind kind() const { return kind_; }
    //const SSeat seat() const { return seat_.lock(); }

    //void setSeat(const SSeat& seat = nullptr) { seat_ = weak_ptr<Seat>(seat); }
    const wstring toString() const;

private:
    const wchar_t ch_;
    const wchar_t name_;
    const PieceColor color_;
    const PieceKind kind_;
};

// 一副棋子类
class Pieces {

public:
    Pieces();

    const SPiece& getOtherPiece(const SPiece& piece) const;
    const vector<SPiece> getBoardPieces(const wstring& pieceChars) const;

    const wstring toString() const;

private:
    vector<SPiece> allPieces_;
 };

// 棋子管理类
class PieceManager {

public:
    static wchar_t getName(wchar_t ch)
    {
        const map<int, int> chIndex_nameIndex{
            { 0, 0 }, { 1, 2 }, { 2, 4 }, { 3, 6 }, { 4, 7 }, { 5, 8 }, { 6, 9 },
            { 7, 1 }, { 8, 3 }, { 9, 5 }, { 10, 6 }, { 11, 7 }, { 12, 8 }, { 13, 10 }
        };
        return nameChars_[chIndex_nameIndex.at(chChars_.find(ch))];
    }

    static wchar_t getPrintName(const Piece& piece)
    {
        wchar_t name{ piece.name() };
        if (piece.color() == PieceColor::BLACK) {
            switch (piece.kind()) {
            case PieceKind::ROOK:
                name = L'車';
                break;
            case PieceKind::KNIGHT:
                name = L'馬';
                break;
            case PieceKind::CANNON:
                name = L'砲';
                break;
            default:
                break;
            }
        }
        return name;
    }

    static PieceColor getColor(wchar_t ch)
    {
        return islower(ch) ? PieceColor::BLACK : PieceColor::RED;
    }

    static PieceColor getOtherColor(PieceColor color)
    {
        return color == PieceColor::RED ? PieceColor::BLACK : PieceColor::RED;
    }

    static PieceColor getColorFromZh(wchar_t numZh)
    {
        return numChars_.at(PieceColor::RED).find(numZh) != wstring::npos
            ? PieceColor::RED
            : PieceColor::BLACK;
    }

    static PieceKind getKindFromCh(wchar_t ch)
    {
        switch (toupper(ch)) {
        case L'K':
            return PieceKind::KING;
        case L'A':
            return PieceKind::ADVISOR;
        case L'B':
            return PieceKind::BISHOP;
        case L'N':
            return PieceKind::KNIGHT;
        case L'R':
            return PieceKind::ROOK;
        case L'C':
            return PieceKind::CANNON;
        default: // L'P'
            break;
        }
        return PieceKind::PAWN;
    }

    static bool isKing(wchar_t name)
    {
        return nameChars_.substr(0, 2).find(name) != wstring::npos;
    }
    static bool isAdvisor(wchar_t name)
    {
        return nameChars_.substr(2, 2).find(name) != wstring::npos;
    }
    static bool isBishop(wchar_t name)
    {
        return nameChars_.substr(4, 2).find(name) != wstring::npos;
    }
    static bool isKnight(wchar_t name)
    {
        return nameChars_[6] == name;
    }
    static bool isRook(wchar_t name)
    {
        return nameChars_[7] == name;
    }
    static bool isCannon(wchar_t name)
    {
        return nameChars_[8] == name;
    }
    static bool isStronge(wchar_t name)
    {
        return nameChars_.substr(6, 5).find(name) != wstring::npos;
    }
    static bool isLineMove(wchar_t name)
    {
        return isKing(name) || nameChars_.substr(7, 4).find(name) != wstring::npos;
    }
    static bool isPawn(wchar_t name)
    {
        return nameChars_.substr(nameChars_.size() - 2, 2).find(name) != wstring::npos;
    }
    static bool isPiece(wchar_t name)
    {
        return nameChars_.find(name) != wstring::npos;
    };

    static const wstring getPiecesChars() { return piecesChar_; }
    static const wstring getZhChars()
    {
        return (preChars_ + nameChars_ + movChars_
            + numChars_.at(PieceColor::RED) + numChars_.at(PieceColor::BLACK));
    }
    static const wstring getICCSChars()
    {
        return wstring(L"0123456789") + ICCSChars_;
    }
    static const wstring FirstFEN() { return FirstFEN_; }

    static int getRowFromICCSChar(wchar_t ch) { return ch - '0'; } // 0:48
    static int getColFromICCSChar(wchar_t ch) { return ICCSChars_.find(ch); }
    static wchar_t getColICCSChar(int col) { return ICCSChars_[col]; }
    static wchar_t nullChar() { return nullChar_; };

    // 宽字符与数字序号的转换
    static int getMovNum(bool isBottom, wchar_t movChar)
    {
        return (-1 + movChars_.find(movChar)) * (isBottom ? 1 : -1);
    }
    static wchar_t getMovChar(bool isSameRow, bool isBottom, bool isLowToUp)
    {
        return movChars_[isSameRow ? 1 : (isBottom == isLowToUp ? 2 : 0)];
    }
    static wchar_t getNumChar(PieceColor color, int num)
    {
        return numChars_.at(color)[num - 1];
    }
    static int getNumIndex(PieceColor color, wchar_t numChar)
    {
        return 0 + numChars_.at(color).find(numChar);
    }
    static int getCurIndex(bool isBottom, int index, int len)
    {
        return isBottom ? len - 1 - index : index;
    }
    static int getPreIndex(int seatsLen, bool isBottom, wchar_t preChar)
    {
        return getCurIndex(isBottom, __getPreChars(seatsLen).find(preChar), seatsLen);
    }
    static wchar_t getPreChar(int seatsLen, bool isBottom, int index)
    {
        return __getPreChars(seatsLen)[getCurIndex(isBottom, index, seatsLen)];
    }
    static wchar_t getColChar(PieceColor color, bool isBottom, int colIndex)
    {
        return numChars_.at(color)[getCurIndex(isBottom, colIndex, BOARDCOLNUM)];
    }

private:
    static const wstring __getPreChars(int length)
    {
        return (length == 2 ? (wstring{ preChars_ }).erase(1, 1) //L"前后"
                            : (length == 3 ? preChars_ //L"前中后"
                                           : numChars_.at(PieceColor::RED).substr(0, 5))); //L"一二三四五");
    }

    static const wstring piecesChar_;
    static const wstring chChars_;
    static const wstring preChars_;
    static const wstring nameChars_;
    static const wstring movChars_;
    static const map<PieceColor, wstring> numChars_;
    static constexpr wchar_t nullChar_{ L'_' };
    static const wstring ICCSChars_;
    static const wstring FirstFEN_;
};
}

#endif