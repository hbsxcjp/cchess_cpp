//#pragma once
#ifndef CHESSMANUAL_H
#define CHESSMANUAL_H
// 中国象棋棋盘布局类型 by-cjp

#include "ChessType.h"

namespace ChessManualSpace {

class ChessManual {
    class Move;

private:
    typedef shared_ptr<Move> SMove;

    // 着法节点类
    class Move : public enable_shared_from_this<Move> {
    public:
        Move() = default;

        int frowcol() const;
        int trowcol() const;
        SSeat_pair getSeat_pair() const { return seat_pair_; }
        const wstring iccs() const;
        const wstring zh() const { return zhStr_; }
        const wstring& remark() const { return remark_; }
        const SPiece& eatPie() const { return eatPie_; }
        const SMove& next() const { return next_; }
        const SMove& other() const { return other_; }
        const SMove prev() const { return prev_.lock(); }

        SMove& addNext();
        SMove& addOther(); 
        SMove& addNext(const SSeat_pair& seat_pair, const wstring& remark);
        SMove& addOther(const SSeat_pair& seat_pair, const wstring& remark);

        void setSeatPairRemark(const SSeat_pair& seat_pair, const wstring& remark);
        void setSeatPair(const SSeat_pair& seat_pair) { seat_pair_ = seat_pair; }
        void setRemark(const wstring& remark) { 
            remark_ = remark; 
            }
        void setPrev(const weak_ptr<Move>& prev) { prev_ = prev; }
        void setZhStr(const wstring& zhStr) { zhStr_ = zhStr; }

        vector<SMove> getPrevMoves();
        void done();
        void undo();

        void cutNext() { next_ = nullptr; }
        void cutOther() { other_ && (other_ = other_->other_); }

        const wstring toString() const;

        int nextNo() const { return nextNo_; }
        int otherNo() const { return otherNo_; }
        int CC_ColNo() const { return CC_ColNo_; }

        void setNextNo(int nextNo) { nextNo_ = nextNo; }
        void setOtherNo(int otherNo) { otherNo_ = otherNo; }
        void setCC_ColNo(int CC_ColNo) { CC_ColNo_ = CC_ColNo; }

    private:
        pair<SSeat, SSeat> seat_pair_{};
        wstring remark_{}; // 注释
        weak_ptr<Move> prev_{};

        wstring zhStr_{}; // 中文着法描述
        SPiece eatPie_{};
        SMove next_{}, other_{};

        int nextNo_{ 0 }, otherNo_{ 0 }, CC_ColNo_{ 0 }; // CC_ColNo_:图中列位置（需在ChessManual::setMoves确定）
    };

public:
    ChessManual();
    ChessManual(const string& infilename);

    SMove& addNextMove(SMove& move, int frowcol, int trowcol, const wstring& remark) const;
    SMove& addOtherMove(SMove& move, int frowcol, int trowcol, const wstring& remark) const;
    SMove& addNextMove(SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const;
    SMove& addOtherMove(SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const;

    void reset(); // 重置为常规的下棋初始状态，不需手工布子
    void read(const string& infilename);
    void write(const string& outfilename);

    void go();
    void back();
    void backTo(const SMove& move);
    void goOther();
    void goInc(int inc);

    void changeSide(ChangeType ct);

    int getMovCount() const { return movCount_; }
    int getRemCount() const { return remCount_; }
    int getRemLenMax() const { return remLenMax_; }
    int getMaxRow() const { return maxRow_; }
    int getMaxCol() const { return maxCol_; }

    const wstring toString();

private:
    void __setFENplusFromFEN(const wstring& FEN, PieceColor color);
    void __setBoardFromInfo();

    void __setMoveFromRowcol(const SMove& move, int frowcol, int trowcol, const wstring& remark) const;
    void __setMoveFromStr(const SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const;
    void __setMoveZhStrAndNums();

    const wstring __moveInfo() const;

    void __readXQF(istream& is);

    void __readBIN(istream& is);
    void __writeBIN(ostream& os) const;

    void __readJSON(istream& is);
    void __writeJSON(ostream& os) const;

    void __readInfo_PGN(wistream& wis);
    void __writeInfo_PGN(wostream& wos) const;

    void __readMove_PGN_ICCSZH(wistream& wis, RecFormat fmt);
    void __writeMove_PGN_ICCSZH(wostream& wos, RecFormat fmt) const;

    void __readMove_PGN_CC(wistream& wis);
    void __writeMove_PGN_CC(wostream& wos) const;

    map<wstring, wstring> info_;
    shared_ptr<Board> board_;
    SMove rootMove_, currentMove_;
    int movCount_{ 0 }, remCount_{ 0 }, remLenMax_{ 0 }, maxRow_{ 0 }, maxCol_{ 0 };
};
 
void transDir(const string& dirfrom, const RecFormat fmt);
void testTransDir(int fd, int td, int ff, int ft, int tf, int tt);

const wstring testChessmanual();

}

#endif