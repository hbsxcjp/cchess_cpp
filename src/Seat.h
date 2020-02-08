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
    int rowcol() const { return row_ * 10 + col_; }
    const SPiece& piece() const { return piece_; }

    bool isSameColor(const SSeat& seat) const;
    void setPiece(const SPiece& piece = nullptr) { piece_ = piece; }
    const SPiece movTo(SSeat& tseat, const SPiece& eatPiece = nullptr);

    const wstring toString() const;

private:
    const int row_, col_;
    SPiece piece_{};
};

// 棋盘位置类
class Seats {
public:
    Seats();

    const SSeat& getSeat(int row, int col) const;
    const SSeat& getSeat(int rowcol) const;
    const SSeat& getSeat(RowCol_pair rowcol_pair) const;

    const SSeat& getKingSeat(bool isBottom) const;

    // 棋子可放置的位置
    SSeat_vector getPutSeats(bool isBottom, const SPiece& piece) const;
    // 某位置棋子可移动的位置（未排除被将军的情况）
    SSeat_vector getMoveSeats(bool isBottom, const SSeat& fseat) const;
    // 取得棋盘上活的棋子
    SSeat_vector getLiveSeats(PieceColor color, wchar_t name = BLANKNAME,
        int col = BLANKCOL, bool getStronge = false) const;
    // '多兵排序'
    SSeat_vector getSortPawnLiveSeats(bool isBottom, PieceColor color, wchar_t name) const;

    void setBoardPieces(const vector<SPiece>& boardPieces);
    void changeSide(const ChangeType ct, const shared_ptr<PieceSpace::Pieces>& pieces);
    const wstring getPieceChars() const;
    const wstring toString() const;

private:
    SSeat_vector allSeats_{};

    SSeat_vector getAllSeats() const;
    SSeat_vector getKingSeats(bool isBottom) const;
    SSeat_vector getAdvisorSeats(bool isBottom) const;
    SSeat_vector getBishopSeats(bool isBottom) const;
    SSeat_vector getPawnSeats(bool isBottom) const;

    SSeat_vector getKingMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector getAdvisorMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector getBishopMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector getKnightMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector getRookMoveSeats(const SSeat& fseat) const;
    SSeat_vector getCannonMoveSeats(const SSeat& fseat) const;
    SSeat_vector getPawnMoveSeats(bool isBottom, const SSeat& fseat) const;

    // 排除同颜色棋子，fseat为空则无需排除
    SSeat_vector __getMoveSeats(const RowCol_pair_vector& rowcols, const SSeat& fseat = nullptr) const;

    const RowCol_pair_vector __getNonObs_MoveRowcols(bool isBottom, const SSeat& fseat,
        const PRowCol_pair_vector getObs_MoveRowcols(bool, int, int)) const;
    const RowCol_pair_vector __getRook_MoveRowcols(const SSeat& fseat) const;
    const RowCol_pair_vector __getCannon_MoveRowcols(const SSeat& fseat) const;
};

// 棋盘位置管理类
class SeatManager {
public:
    static bool isBottom(int row) { return row < RowLowUpIndex_; };
    static int getIndex_rc(int row, int col) { return row * BOARDCOLNUM + col; }
    static int getIndex_rc(int rowcol) { return getIndex_rc(rowcol / 10, rowcol % 10); }
    static int getRotate(int rowcol) { return (BOARDROWNUM - rowcol / 10 - 1) * 10 + (BOARDCOLNUM - rowcol % 10 - 1); }
    static int getSymmetry(int rowcol) { return rowcol + BOARDCOLNUM - rowcol % 10 * 2 - 1; }

    static void movBack(SSeat& fseat, SSeat& tseat, const SPiece& eatPiece);

    static const RowCol_pair_vector getRowCols(const SSeat_vector& seats);

    static const RowCol_pair_vector getAllRowcols();
    static const RowCol_pair_vector getKingRowcols(bool isBottom);
    static const RowCol_pair_vector getAdvisorRowcols(bool isBottom);
    static const RowCol_pair_vector getBishopRowcols(bool isBottom);
    static const RowCol_pair_vector getPawnRowcols(bool isBottom);

    static const RowCol_pair_vector getKingMoveRowcols(bool isBottom, int frow, int fcol);
    static const RowCol_pair_vector getAdvisorMoveRowcols(bool isBottom, int frow, int fcol);
    static const PRowCol_pair_vector getBishopObs_MoveRowcols(bool isBottom, int frow, int fcol);
    static const PRowCol_pair_vector getKnightObs_MoveRowcols(bool isBottom, int frow, int fcol);
    static const vector<RowCol_pair_vector> getRookCannonMoveRowcol_Lines(int frow, int fcol);
    static const RowCol_pair_vector getPawnMoveRowcols(bool isBottom, int frow, int fcol);

private:
    static constexpr int RowLowIndex_{ 0 }, RowLowMidIndex_{ 2 }, RowLowUpIndex_{ 4 },
        RowUpLowIndex_{ 5 }, RowUpMidIndex_{ 7 }, RowUpIndex_{ 9 },
        ColLowIndex_{ 0 }, ColMidLowIndex_{ 3 }, ColMidUpIndex_{ 5 }, ColUpIndex_{ 8 };
};

const wstring getSeatsStr(const SSeat_vector& seats);
const wstring getRowColsStr(const RowCol_pair_vector& rowcols);
}

#endif
