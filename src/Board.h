//#pragma once
#ifndef BOARD_H
#define BOARD_H

#include "ChessType.h"

namespace BoardSpace {

class Board {
public:
    Board(const wstring& pieceChars = wstring{});

    bool isBottomSide(PieceColor color) const { return bottomColor_ == color; }
    bool isKilled(PieceColor color) const;
    bool isDied(PieceColor color) const;

    PRowCol_pair getPRowCol_pair(const wstring& str) const;
    const wstring getZHStr(PRowCol_pair prowcol_pair) const;

    const RowCol_pair_vector getPutRowCols(const SPiece& piece) const;
    const RowCol_pair_vector getCanMoveRowCols(RowCol_pair rowcol_pair) const;
    const RowCol_pair_vector getLiveRowCols(PieceColor color) const;

    const SPiece doneMove(PRowCol_pair prowcol_pair) const;
    void undoMove(PRowCol_pair prowcol_pair, const SPiece& eatPie) const;

    void setPieces(const wstring& pieceChars);
    void changeSide(const ChangeType ct);

    const wstring getPieceChars() const;
    const wstring toString() const;

private:
    PieceColor bottomColor_;
    shared_ptr<Pieces> pieces_;
    shared_ptr<Seats> seats_;

    void __setBottomSide();
};

const wstring FENplusToFEN(const wstring& FENplus);
const wstring FENToFENplus(const wstring& FEN, PieceColor color);
const wstring pieCharsToFEN(const wstring& pieceChars); // 便利函数，下同
const wstring FENTopieChars(const wstring& fen);

const string getExtName(const RecFormat fmt);
RecFormat getRecFormat(const string& ext);

const wstring testBoard();
}
#endif