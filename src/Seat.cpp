#include "Seat.h"
#include "Piece.h"

namespace SeatSpace {

/* ===== Seat start. ===== */
Seat::Seat(int row, int col)
    : row_{ row }
    , col_{ col }
{
}

bool Seat::isSameColor(const SSeat& seat) const
{
    auto& piece = seat->piece();
    return piece && piece->color() == piece_->color();
}

const SPiece Seat::movTo(const SSeat& tseat, const SPiece& eatPiece) const
{
    auto tpiece = tseat->piece();
    tseat->setPiece(this->piece());
    setPiece(eatPiece);
    return tpiece;
}

const wstring Seat::toString() const
{
    wostringstream wos{};
    wos << row_ << col_ << L'&' << (piece_ ? piece_->name() : L'_'); //<< boolalpha << setw(2) <<
    return wos.str();
}
/* ===== Seat end. ===== */

/* ===== Seats start. ===== */
Seats::Seats()
{
    for (auto& rowcol_pair : SeatManager::getAllRowCols())
        allSeats_.push_back(make_shared<Seat>(rowcol_pair.first, rowcol_pair.second));
}

inline const SSeat& Seats::getSeat(int row, int col) const
{
    return allSeats_.at(SeatManager::getIndex_rc(row, col));
}

inline const SSeat& Seats::getSeat(RowCol_pair rowcol_pair) const
{
    return getSeat(rowcol_pair.first, rowcol_pair.second);
}

PieceColor Seats::getSideColor(bool isBottom) const { return __getKingSeat(isBottom)->piece()->color(); }

bool Seats::isKilled(bool isBottom) const
{
    auto &kingSeat{ __getKingSeat(isBottom) }, &otherSeat{ __getKingSeat(!isBottom) };
    PieceColor kingColor = kingSeat->piece()->color(),
               otherColor = PieceManager::getOtherColor(kingColor);
    int kcol{ kingSeat->col() };
    if (kcol == otherSeat->col()) {
        int krow{ kingSeat->row() }, orow{ otherSeat->row() },
            lrow{ isBottom ? krow : orow }, urow{ isBottom ? orow : krow };
        bool killed{ true };
        for (int row = lrow + 1; row < urow; ++row)
            if (getSeat(row, kcol)->piece()) { // 有棋子
                killed = false;
                break;
            }
        if (killed)
            return true; // 全部是空棋子，则将帅对面
    }
    // '获取某方可杀将棋子全部可走的位置
    auto krowcol_pair = kingSeat->rowCol_pair();
    for (auto& frowcol_p : getLiveRowCols(otherColor, BLANKNAME, BLANKCOL, true)) {
        auto mvRowCols = getMoveRowCols(kingColor, frowcol_p);
        if (!mvRowCols.empty() && find(mvRowCols.begin(), mvRowCols.end(), krowcol_pair) != mvRowCols.end()) // 对方强子可走位置有本将位置
            return true;
    }
    return false;
}

bool Seats::isDied(bool isBottom) const
{
    auto sideColor = getSideColor(isBottom);
    for (auto& frowcol_p : getLiveRowCols(sideColor))
        if (!getMoveRowCols(sideColor, frowcol_p).empty()) // 本方还有棋子可以走
            return false;
    return true;
}

RowCol_pair_vector Seats::getPutRowCols(PieceColor sideColor, const SPiece& piece) const
{
    bool isBottom = sideColor == piece->color();
    switch (piece->kind()) {
    case PieceKind::KING:
        return SeatManager::getKingRowCols(isBottom);
    case PieceKind::ADVISOR:
        return SeatManager::getAdvisorRowCols(isBottom);
    case PieceKind::BISHOP:
        return SeatManager::getBishopRowCols(isBottom);
    case PieceKind::PAWN:
        return SeatManager::getPawnRowCols(isBottom);
    default: // KNIGHT ROOK CANNON
        break;
    }
    return SeatManager::getAllRowCols();
}

RowCol_pair_vector Seats::getMoveRowCols(PieceColor sideColor, const RowCol_pair& rowcol_pair) const
{
    // 该位置需有棋子，由调用者board来保证
    auto& fseat = getSeat(rowcol_pair);
    auto& piece = fseat->piece();
    assert(fseat->piece());
    PieceColor color{ piece->color() };
    bool isBottom = sideColor == color;
    SSeat_vector seats;
    switch (piece->kind()) {
    case PieceKind::ROOK:
        seats = __getRookMoveSeats(fseat);
        break;
    case PieceKind::KNIGHT:
        seats = __getKnightMoveSeats(isBottom, fseat);
        break;
    case PieceKind::CANNON:
        seats = __getCannonMoveSeats(fseat);
        break;
    case PieceKind::BISHOP:
        seats = __getBishopMoveSeats(isBottom, fseat);
        break;
    case PieceKind::ADVISOR:
        seats = __getAdvisorMoveSeats(isBottom, fseat);
        break;
    case PieceKind::PAWN:
        seats = __getPawnMoveSeats(isBottom, fseat);
        break;
    case PieceKind::KING:
        seats = __getKingMoveSeats(isBottom, fseat);
        break;
    default:
        break;
    };

    auto pos = remove_if(seats.begin(), seats.end(),
        [&](const SSeat& tseat) {
            // 移动棋子后，检测是否会被对方将军
            auto eatPiece = fseat->movTo(tseat);
            bool killed{ isKilled(isBottom) };
            tseat->movTo(fseat, eatPiece);
            return killed;
        });
    return __getRowCols(SSeat_vector{ seats.begin(), pos });
}

RowCol_pair_vector Seats::getLiveRowCols(PieceColor color, wchar_t name, int col, bool getStronge) const
{
    return __getRowCols(__getLiveSeats(color, name, col, getStronge));
}

PRowCol_pair Seats::getPRowCol_pair(PieceColor sideColor, const wstring& str) const
{
    assert(str.size() == 4);
    RowCol_pair frowcol_pair, trowcol_pair;
    SSeat_vector seats;
    // 根据最后一个字符判断该着法属于哪一方
    PieceColor color{ PieceManager::getColorFromZh(str.back()) };
    bool isBottom{ sideColor == color };
    int index{}, movDir{ PieceManager::getMovNum(isBottom, str.at(2)) };
    wchar_t name{ str.front() };

    if (PieceManager::isPiece(name)) { // 首字符为棋子名
        seats = __getLiveSeats(color, name,
            PieceManager::getCurIndex(isBottom,
                PieceManager::getNumIndex(color, str.at(1)), BOARDCOLNUM));
        assert(seats.size() > 0);
        //# 排除：士、象同列时不分前后，以进、退区分棋子。移动方向为退时，修正index
        index = (seats.size() == 2 && movDir == -1) ? 1 : 0; //&& isAdvBish(name)
    } else {
        name = str.at(1);
        seats = (PieceManager::isPawn(name)
                ? __getSortPawnLiveSeats(isBottom, color, name)
                : __getLiveSeats(color, name));
        index = PieceManager::getPreIndex(seats.size(), isBottom, str.front());
    }

    //wcout << __LINE__ << L':' << getRowColsStr(__getRowCols(seats)) << endl;
    assert(seats.size() - index >= 1);
    //wcout << __LINE__ << L':' << getRowColsStr(__getRowCols(seats)) << endl;

    frowcol_pair = seats.at(index)->rowCol_pair();

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

    //assert(str == getZHStr(sideColor, make_pair(frowcol_pair, trowcol_pair)));
    return make_pair(frowcol_pair, trowcol_pair);
}

//中文纵线着法
const wstring Seats::getZHStr(PieceColor sideColor, PRowCol_pair prowcol_pair) const
{
    wstring wstr{};
    auto& fseat = getSeat(prowcol_pair.first);
    const SPiece& fromPiece{ fseat->piece() };
    /*
    wcout << __LINE__ << L':' << setfill(L'0') << setw(2) << SeatManager::getRowCol(prowcol_pair.first) << L'-'
          << setw(2) << SeatManager::getRowCol(prowcol_pair.second)
          << L' ' << (fromPiece ? fromPiece->toString() : L"--") << L'\n';
    //*/

    assert(fromPiece);
    PieceColor color{ fromPiece->color() };
    wchar_t name{ fromPiece->name() };
    int fromRow{ prowcol_pair.first.first }, fromCol{ prowcol_pair.first.second },
        toRow{ prowcol_pair.second.first }, toCol{ prowcol_pair.second.second };
    bool isSameRow{ fromRow == toRow }, isBottom{ sideColor == color };
    auto seats = __getLiveSeats(color, name, fromCol);
    //*

    if (seats.size() > 1 && PieceManager::isStronge(name)) {
        if (PieceManager::isPawn(name))
            seats = __getSortPawnLiveSeats(isBottom, color, name);
        wstr += PieceManager::getPreChar(seats.size(), isBottom,
            distance(seats.begin(), find(seats.begin(), seats.end(), fseat)));
        wstr += name;
    } else { //将帅, 仕(士),相(象): 不用“前”和“后”区别，因为能退的一定在前，能进的一定在后
        wstr += name;
        wstr += PieceManager::getColChar(color, isBottom, fromCol);
    }
    wstr += PieceManager::getMovChar(isSameRow, isBottom, toRow > fromRow);
    wstr += (PieceManager::isLineMove(name) && !isSameRow
            ? PieceManager::getNumChar(color, abs(fromRow - toRow)) // 非同一行
            : PieceManager::getColChar(color, isBottom, toCol));

    //*/
    //assert(getPRowCol_pair(sideColor, wstr) == prowcol_pair);
    return wstr;
}

const SPiece Seats::doneMove(PRowCol_pair prowcol_pair) const
{
    return getSeat(prowcol_pair.first)->movTo(getSeat(prowcol_pair.second));
}

void Seats::undoMove(PRowCol_pair prowcol_pair, const SPiece& eatPie) const
{
    getSeat(prowcol_pair.second)->movTo(getSeat(prowcol_pair.first), eatPie);
}

void Seats::setBoardPieces(const vector<SPiece>& boardPieces)
{
    int index{ 0 };
    for_each(allSeats_.begin(), allSeats_.end(),
        [&](const SSeat& seat) { seat->setPiece(boardPieces[index++]); });
}

void Seats::changeSide(const ChangeType ct, const shared_ptr<Pieces>& pieces)
{
    vector<SPiece> boardPieces{};
    auto changeRowcol = (ct == ChangeType::ROTATE ? &SeatManager::getRotate : &SeatManager::getSymmetry);
    for_each(allSeats_.begin(), allSeats_.end(),
        [&](const SSeat& seat) {
            boardPieces.push_back(ct == ChangeType::EXCHANGE
                    ? pieces->getOtherPiece(seat->piece())
                    : getSeat(changeRowcol(seat->rowCol_pair()))->piece());
        });
    setBoardPieces(boardPieces);
}

const wstring Seats::getPieceChars() const
{
    wostringstream wos{};
    for_each(allSeats_.begin(), allSeats_.end(),
        [&](const SSeat& seat) {
            wos << (seat->piece() ? seat->piece()->ch() : PieceManager::nullChar());
        });
    return wos.str();
}

const wstring Seats::toString() const
{
    wostringstream wos{};
    for_each(allSeats_.begin(), allSeats_.end(),
        [&](const SSeat& seat) {
            wos << seat->toString() << L' ';
        });
    return wos.str();
}

const SSeat& Seats::__getKingSeat(bool isBottom) const
{
    for (auto& rowcol_pair : SeatManager::getKingRowCols(isBottom)) {
        auto& seat = getSeat(rowcol_pair);
        auto& piece = seat->piece();
        if (piece && PieceManager::isKing(piece->name()))
            return seat;
    }
    throw runtime_error("将（帅）不在棋盘上面!");
}

RowCol_pair_vector Seats::__getRowCols(const SSeat_vector& seats) const
{
    RowCol_pair_vector rowcol_pv{};
    for (auto& seat : seats)
        rowcol_pv.push_back(seat->rowCol_pair());
    return rowcol_pv;
}

SSeat_vector Seats::__getSeats(const RowCol_pair_vector& rowcol_pv) const
{
    SSeat_vector seats{};
    for (auto& rowcol_p : rowcol_pv)
        seats.push_back(getSeat(rowcol_p));
    return seats;
}

SSeat_vector Seats::__getMoveSeats(const SSeat_vector& seats, const SSeat& fseat) const
{
    SSeat_vector mseats{};
    if (fseat->piece())
        for (auto& tseat : seats)
            if (!fseat->isSameColor(tseat)) // 非同一颜色
                mseats.push_back(tseat);
    return mseats;
}

SSeat_vector Seats::__getLiveSeats(PieceColor color, wchar_t name, int col, bool getStronge) const
{
    SSeat_vector seats{};
    for (auto& seat : allSeats_) {
        auto& piece = seat->piece();
        if (piece && color == piece->color()
            && (name == BLANKNAME || name == piece->name())
            && (col == BLANKCOL || col == seat->col())
            && (!getStronge || PieceManager::isStronge(piece->name())))
            seats.push_back(seat);
    }
    return seats;
}

SSeat_vector Seats::__getSortPawnLiveSeats(bool isBottom, PieceColor color, wchar_t name) const
{
    // 最多5个兵
    SSeat_vector pawnSeats{ __getLiveSeats(color, name) }, seats{};
    // 按列建立字典，按列排序
    map<int, SSeat_vector> colSeats{};
    int order = (isBottom ? -1 : 1); // isBottom则列倒序,每列位置倒序
    for_each(pawnSeats.begin(), pawnSeats.end(),
        [&](const SSeat& seat) {
            colSeats[order * seat->col()].push_back(seat);
        });

    // 整合成一个数组
    for (auto& colseat_pv : colSeats) {
        auto seat_v = colseat_pv.second;
        if (seat_v.size() > 1) { // 筛除只有一个位置的列
            if (isBottom)
                reverse(seat_v.begin(), seat_v.end());
            copy(seat_v.begin(), seat_v.end(), back_inserter(seats)); //按列存入
        }
    }
    return seats;
}

SSeat_vector Seats::__getKingMoveSeats(bool isBottom, const SSeat& fseat) const
{
    return __getMoveSeats(__getSeats(SeatManager::getKingMoveRowCols(isBottom, fseat->rowCol_pair())), fseat);
}

SSeat_vector Seats::__getAdvisorMoveSeats(bool isBottom, const SSeat& fseat) const
{
    return __getMoveSeats(__getSeats(SeatManager::getAdvisorMoveRowCols(isBottom, fseat->rowCol_pair())), fseat);
}

SSeat_vector Seats::__getBishopMoveSeats(bool isBottom, const SSeat& fseat) const
{
    return __getMoveSeats(__getSeats(__getNonObs_MoveSeats(isBottom, fseat, &SeatManager::getBishopObs_MoveRowCols)), fseat);
}

SSeat_vector Seats::__getKnightMoveSeats(bool isBottom, const SSeat& fseat) const
{
    return __getMoveSeats(__getSeats(__getNonObs_MoveSeats(isBottom, fseat, &SeatManager::getKnightObs_MoveRowCols)), fseat);
}

SSeat_vector Seats::__getRookMoveSeats(const SSeat& fseat) const
{
    return __getMoveSeats(__getRook_MoveSeats(fseat), fseat);
}

SSeat_vector Seats::__getCannonMoveSeats(const SSeat& fseat) const
{
    return __getMoveSeats(__getCannon_MoveSeats(fseat), fseat);
}

SSeat_vector Seats::__getPawnMoveSeats(bool isBottom, const SSeat& fseat) const
{
    return __getMoveSeats(__getSeats(SeatManager::getPawnMoveRowCols(isBottom, fseat->rowCol_pair())), fseat);
}

RowCol_pair_vector Seats::__getNonObs_MoveSeats(bool isBottom, const SSeat& fseat,
    PRowCol_pair_vector (*getObs_MoveRowCols)(bool, const RowCol_pair&)) const
{
    RowCol_pair_vector rowcol_pv{};
    auto obs_MoveRowCols = getObs_MoveRowCols(isBottom, fseat->rowCol_pair());
    for_each(obs_MoveRowCols.begin(), obs_MoveRowCols.end(),
        [&](const PRowCol_pair& obs_Moverowcol) {
            if (!getSeat(obs_Moverowcol.first)->piece()) // 该位置无棋子
                rowcol_pv.push_back(obs_Moverowcol.second);
        });
    return rowcol_pv;
}

SSeat_vector Seats::__getRook_MoveSeats(const SSeat& fseat) const
{
    SSeat_vector seats{};
    for (auto& rowcolpair_Line : SeatManager::getRookCannonMoveRowCol_Lines(fseat->rowCol_pair()))
        for (auto& rowcol_pair : rowcolpair_Line) {
            auto& tseat = getSeat(rowcol_pair);
            seats.push_back(tseat);
            if (tseat->piece()) // 该位置有棋子
                break;
        }
    return seats;
}

SSeat_vector
Seats::__getCannon_MoveSeats(const SSeat& fseat) const
{
    SSeat_vector seats{};
    for (auto& rowcolpair_Line : SeatManager::getRookCannonMoveRowCol_Lines(fseat->rowCol_pair())) {
        bool isSkip = false; // 是否已跳棋子的标志
        for (auto rowcol_pair : rowcolpair_Line) {
            auto& tseat = getSeat(rowcol_pair);
            auto& piece = tseat->piece();
            if (!isSkip) {
                if (!piece) // 该位置无棋子
                    seats.push_back(tseat);
                else
                    isSkip = true;
            } else if (piece) { // 该位置有棋子
                seats.push_back(tseat);
                break;
            }
        }
    }
    return seats;
}

RowCol_pair_vector SeatManager::getAllRowCols()
{
    RowCol_pair_vector rowcol_pv{};
    for (int row = 0; row < BOARDROWNUM; ++row)
        for (int col = 0; col < BOARDCOLNUM; ++col)
            rowcol_pv.emplace_back(row, col);
    return rowcol_pv;
}

RowCol_pair_vector SeatManager::getKingRowCols(bool isBottom)
{
    RowCol_pair_vector rowcol_pv{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            rowcol_pv.emplace_back(row, col);
    return rowcol_pv;
}

RowCol_pair_vector SeatManager::getAdvisorRowCols(bool isBottom)
{
    RowCol_pair_vector rowcol_pv{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ }, rmd{ isBottom ? 1 : 0 }; // 行列和的奇偶值
    for (int row = rowLow; row <= rowUp; ++row)
        for (int col = ColMidLowIndex_; col <= ColMidUpIndex_; ++col)
            if ((col + row) % 2 == rmd)
                rowcol_pv.emplace_back(row, col);
    return rowcol_pv;
}

RowCol_pair_vector SeatManager::getBishopRowCols(bool isBottom)
{
    RowCol_pair_vector rowcol_pv{};
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    for (int row = rowLow; row <= rowUp; row += 2)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2) {
            int gap{ row - col };
            if ((isBottom && (gap == 2 || gap == -2 || gap == -6))
                || (!isBottom && (gap == 7 || gap == 3 || gap == -1)))
                rowcol_pv.emplace_back(row, col);
        }
    return rowcol_pv;
}

RowCol_pair_vector SeatManager::getPawnRowCols(bool isBottom)
{
    RowCol_pair_vector rowcol_pv{};
    int lfrow{ isBottom ? RowLowUpIndex_ - 1 : RowUpLowIndex_ },
        ufrow{ isBottom ? RowLowUpIndex_ : RowUpLowIndex_ + 1 },
        ltrow{ isBottom ? RowUpLowIndex_ : RowLowIndex_ },
        utrow{ isBottom ? RowUpIndex_ : RowLowUpIndex_ };
    for (int row = lfrow; row <= ufrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; col += 2)
            rowcol_pv.emplace_back(row, col);
    for (int row = ltrow; row <= utrow; ++row)
        for (int col = ColLowIndex_; col <= ColUpIndex_; ++col)
            rowcol_pv.emplace_back(row, col);
    return rowcol_pv;
}

RowCol_pair_vector
SeatManager::getKingMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    RowCol_pair_vector rowcol_pv{
        { frow, fcol - 1 }, { frow, fcol + 1 },
        { frow - 1, fcol }, { frow + 1, fcol }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = remove_if(rowcol_pv.begin(), rowcol_pv.end(),
        [&](const pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return RowCol_pair_vector{ rowcol_pv.begin(), pos };
}

RowCol_pair_vector
SeatManager::getAdvisorMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    RowCol_pair_vector rowcol_pv{
        { frow - 1, fcol - 1 }, { frow - 1, fcol + 1 },
        { frow + 1, fcol - 1 }, { frow + 1, fcol + 1 }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpMidIndex_ },
        rowUp{ isBottom ? RowLowMidIndex_ : RowUpIndex_ };
    auto pos = remove_if(rowcol_pv.begin(), rowcol_pv.end(),
        [&](const pair<int, int>& rowcol) {
            return !(rowcol.first >= rowLow && rowcol.first <= rowUp
                && rowcol.second >= ColMidLowIndex_ && rowcol.second <= ColMidUpIndex_);
        });
    return RowCol_pair_vector{ rowcol_pv.begin(), pos };
}

PRowCol_pair_vector
SeatManager::getBishopObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    PRowCol_pair_vector obs_MoveRowCols{
        { { frow - 1, fcol - 1 }, { frow - 2, fcol - 2 } },
        { { frow - 1, fcol + 1 }, { frow - 2, fcol + 2 } },
        { { frow + 1, fcol - 1 }, { frow + 2, fcol - 2 } },
        { { frow + 1, fcol + 1 }, { frow + 2, fcol + 2 } }
    };
    int rowLow{ isBottom ? RowLowIndex_ : RowUpLowIndex_ },
        rowUp{ isBottom ? RowLowUpIndex_ : RowUpIndex_ };
    auto pos = remove_if(obs_MoveRowCols.begin(), obs_MoveRowCols.end(),
        [&](const PRowCol_pair& obs_Moverowcol) {
            return (obs_Moverowcol.first.first < rowLow
                || obs_Moverowcol.first.first > rowUp
                || obs_Moverowcol.first.second < ColLowIndex_
                || obs_Moverowcol.first.second > ColUpIndex_);
        });
    return PRowCol_pair_vector{ obs_MoveRowCols.begin(), pos };
}

PRowCol_pair_vector
SeatManager::getKnightObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    PRowCol_pair_vector obs_MoveRowCols{
        { { frow - 1, fcol }, { frow - 2, fcol - 1 } },
        { { frow - 1, fcol }, { frow - 2, fcol + 1 } },
        { { frow, fcol - 1 }, { frow - 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow - 1, fcol + 2 } },
        { { frow, fcol - 1 }, { frow + 1, fcol - 2 } },
        { { frow, fcol + 1 }, { frow + 1, fcol + 2 } },
        { { frow + 1, fcol }, { frow + 2, fcol - 1 } },
        { { frow + 1, fcol }, { frow + 2, fcol + 1 } }
    };
    auto pos = remove_if(obs_MoveRowCols.begin(), obs_MoveRowCols.end(),
        [&](const PRowCol_pair& obs_Moverowcol) {
            return (obs_Moverowcol.first.first < RowLowIndex_
                || obs_Moverowcol.first.first > RowUpIndex_
                || obs_Moverowcol.first.second < ColLowIndex_
                || obs_Moverowcol.first.second > ColUpIndex_);
        });
    return PRowCol_pair_vector{ obs_MoveRowCols.begin(), pos };
}

vector<RowCol_pair_vector>
SeatManager::getRookCannonMoveRowCol_Lines(const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    vector<RowCol_pair_vector> rowcol_Lines(4);
    for (int col = fcol - 1; col >= ColLowIndex_; --col)
        rowcol_Lines[0].emplace_back(frow, col);
    for (int col = fcol + 1; col <= ColUpIndex_; ++col)
        rowcol_Lines[1].emplace_back(frow, col);
    for (int row = frow - 1; row >= RowLowIndex_; --row)
        rowcol_Lines[2].emplace_back(row, fcol);
    for (int row = frow + 1; row <= RowUpIndex_; ++row)
        rowcol_Lines[3].emplace_back(row, fcol);
    return rowcol_Lines;
}

RowCol_pair_vector
SeatManager::getPawnMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair)
{
    int frow{ rowcol_pair.first }, fcol{ rowcol_pair.second };
    RowCol_pair_vector rowcol_pv{};
    int row{}, col{};
    if ((isBottom && (row = frow + 1) <= RowUpIndex_)
        || (!isBottom && (row = frow - 1) >= RowLowIndex_))
        rowcol_pv.emplace_back(row, fcol);
    if (isBottom == (frow > RowLowUpIndex_)) { // 兵已过河
        if ((col = fcol - 1) >= ColLowIndex_)
            rowcol_pv.emplace_back(frow, col);
        if ((col = fcol + 1) <= ColUpIndex_)
            rowcol_pv.emplace_back(frow, col);
    }
    return rowcol_pv;
}
/* ===== Seats end. ===== */

const wstring getRowColsStr(const RowCol_pair_vector& rowcol_pv)
{
    wostringstream wos{};
    wos << rowcol_pv.size() << L"个: ";
    for (auto& rowcol : rowcol_pv)
        wos << SeatManager::getRowCol(rowcol) << L' ';
    return wos.str();
}
}