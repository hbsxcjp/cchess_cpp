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
    PieceColor getSideColor(bool isBottom) const;
    bool isKilled(bool isBottom) const;
    bool isDied(bool isBottom) const;

    // 棋子可放置的位置
    RowCol_pair_vector getPutRowCols(PieceColor bottomColor, const SPiece& piece) const;
    // 某位置棋子可移动的位置（未排除被将军的情况）
    RowCol_pair_vector getMoveRowCols(PieceColor bottomColor, const RowCol_pair& rowcol_pair) const;
    // 取得棋盘上活的棋子
    RowCol_pair_vector getLiveRowCols(PieceColor color, wchar_t name = BLANKNAME,
        int col = BLANKCOL, bool getStronge = false) const;

    PRowCol_pair getPRowCol_pair(PieceColor bottomColor, const wstring& str) const;
    const wstring getZHStr(PieceColor bottomColor, PRowCol_pair prowcol_pair) const;

    const SPiece doneMove(PRowCol_pair prowcol_pair) const;
    void undoMove(PRowCol_pair prowcol_pair, const SPiece& eatPie) const;

    void setBoardPieces(const vector<SPiece>& boardPieces);
    void changeSide(const ChangeType ct, const shared_ptr<PieceSpace::Pieces>& pieces);
    const wstring getPieceChars() const;
    const wstring toString() const;

private:
    SSeat_vector allSeats_{};

    const SSeat& __getKingSeat(bool isBottom) const;

    RowCol_pair_vector __getRowCols(const SSeat_vector& seats) const;
    SSeat_vector __getSeats(const RowCol_pair_vector& rowcol_pv) const;

    // 排除同颜色棋子，fseat为空则无需排除
    SSeat_vector __getMoveSeats(const SSeat_vector& seats, const SSeat& fseat) const;
    // 取得棋盘上活的棋子
    SSeat_vector __getLiveSeats(PieceColor color, wchar_t name = BLANKNAME,
        int col = BLANKCOL, bool getStronge = false) const;
    // '多兵排序'
    SSeat_vector __getSortPawnLiveSeats(bool isBottom, PieceColor color, wchar_t name) const;

    SSeat_vector __getKingMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector __getAdvisorMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector __getBishopMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector __getKnightMoveSeats(bool isBottom, const SSeat& fseat) const;
    SSeat_vector __getRookMoveSeats(const SSeat& fseat) const;
    SSeat_vector __getCannonMoveSeats(const SSeat& fseat) const;
    SSeat_vector __getPawnMoveSeats(bool isBottom, const SSeat& fseat) const;

    RowCol_pair_vector __getNonObs_MoveSeats(bool isBottom, const SSeat& fseat,
        PRowCol_pair_vector (*getObs_MoveRowCols)(bool, const RowCol_pair&)) const;
    SSeat_vector __getRook_MoveSeats(const SSeat& fseat) const;
    SSeat_vector __getCannon_MoveSeats(const SSeat& fseat) const;
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
