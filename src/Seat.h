//#pragma once
#ifndef SEAT_H
#define SEAT_H

#include "ChessType.h"

namespace SeatSpace {

// 棋子位置类
class Seat : public enable_shared_from_this<Seat> {

public:
    explicit Seat(int row, int col);

    int row() const { return row_; }
    int col() const { return col_; }
    RowCol_pair rowCol_pair() { return make_pair(row_, col_); }
    const SPiece& piece() const { return piece_; }

    bool isSameColor(const SSeat& seat) const;
    void setPiece(const SPiece& piece = nullptr) const { piece_ = piece; }
    const SPiece movTo(const SSeat& tseat, const SPiece& eatPiece = nullptr) const;

    const wstring toString() const;

private:
    const int row_, col_;
    mutable SPiece piece_{};
};

// 棋盘位置类
class Seats {
public:
    Seats();

    const SSeat& getSeat(int row, int col) const;
    const SSeat& getSeat(RowCol_pair rowcol_pair) const;

    const SSeat& getKingSeat(bool isBottom) const;

    // 棋子可放置的位置
    RowCol_pair_vector getPutRowCols(bool isBottom, const SPiece& piece) const;
    // 某位置棋子可移动的位置（未排除被将军的情况）
    RowCol_pair_vector getMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;
    // 取得棋盘上活的棋子
    RowCol_pair_vector getLiveRowCols(PieceColor color, wchar_t name = BLANKNAME,
        int col = BLANKCOL, bool getStronge = false) const;
    // '多兵排序'
    RowCol_pair_vector getSortPawnLiveRowCols(bool isBottom, PieceColor color, wchar_t name) const;

    void setBoardPieces(const vector<SPiece>& boardPieces);
    void changeSide(const ChangeType ct, const shared_ptr<PieceSpace::Pieces>& pieces);
    const wstring getPieceChars() const;
    const wstring toString() const;

private:
    SSeat_vector allSeats_{};

    RowCol_pair_vector __getKingMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getAdvisorMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getBishopMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getKnightMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getRookMoveRowCols(const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getCannonMoveRowCols(const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getPawnMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const;

    // 排除同颜色棋子，fseat为空则无需排除
    RowCol_pair_vector __getMoveRowCols(const RowCol_pair_vector& rowcol_pv, const RowCol_pair& rowcol_pair) const;

    RowCol_pair_vector __getNonObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair,
        PRowCol_pair_vector getObs_MoveRowCols(bool, const RowCol_pair&)) const;
    RowCol_pair_vector __getRook_MoveRowCols(const RowCol_pair& rowcol_pair) const;
    RowCol_pair_vector __getCannon_MoveRowCols(const RowCol_pair& rowcol_pair) const;
};

// 棋盘位置管理类
class SeatManager {
public:
    static bool isBottom(int row) { return row < RowLowUpIndex_; };
    static int getIndex_rc(int row, int col) { return row * BOARDCOLNUM + col; }
    static int getRowCol(RowCol_pair rowcol_pair) { return rowcol_pair.first * 10 + rowcol_pair.second; }
    static RowCol_pair getRowCol_pair(int rowcol) { return make_pair(rowcol / 10, rowcol % 10); }
    static RowCol_pair getRotate(RowCol_pair rowcol_pair) { return make_pair(BOARDROWNUM - 1 - rowcol_pair.first, BOARDCOLNUM - 1 - rowcol_pair.second); }
    static RowCol_pair getSymmetry(RowCol_pair rowcol_pair) { return make_pair(rowcol_pair.first, BOARDCOLNUM - 1 - rowcol_pair.second); }

    static RowCol_pair_vector getAllRowCols();
    static RowCol_pair_vector getKingRowCols(bool isBottom);
    static RowCol_pair_vector getAdvisorRowCols(bool isBottom);
    static RowCol_pair_vector getBishopRowCols(bool isBottom);
    static RowCol_pair_vector getPawnRowCols(bool isBottom);

    static RowCol_pair_vector getKingMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair);
    static RowCol_pair_vector getAdvisorMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair);
    static PRowCol_pair_vector getBishopObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair);
    static PRowCol_pair_vector getKnightObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair);
    static vector<RowCol_pair_vector> getRookCannonMoveRowCol_Lines(const RowCol_pair& rowcol_pair);
    static RowCol_pair_vector getPawnMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair);

private:
    static constexpr int RowLowIndex_{ 0 }, RowLowMidIndex_{ 2 }, RowLowUpIndex_{ 4 },
        RowUpLowIndex_{ 5 }, RowUpMidIndex_{ 7 }, RowUpIndex_{ 9 },
        ColLowIndex_{ 0 }, ColMidLowIndex_{ 3 }, ColMidUpIndex_{ 5 }, ColUpIndex_{ 8 };
};

const wstring getRowColsStr(const RowCol_pair_vector& rowcols);
}

#endif
