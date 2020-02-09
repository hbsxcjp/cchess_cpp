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

const SSeat& Seats::getKingSeat(bool isBottom) const
{
    for (auto& rowcol_pair : SeatManager::getKingRowCols(isBottom)) {
        auto& seat = getSeat(rowcol_pair);
        auto& piece = seat->piece();
        if (piece && PieceManager::isKing(piece->name()))
            return seat;
    }
    throw runtime_error("将（帅）不在棋盘上面!");
}

RowCol_pair_vector Seats::getPutRowCols(bool isBottom, const SPiece& piece) const
{
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

RowCol_pair_vector Seats::getMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    // 该位置需有棋子，由调用者board来保证
    switch (getSeat(rowcol_pair)->piece()->kind()) {
    case PieceKind::ROOK:
        return __getRookMoveRowCols(rowcol_pair);
    case PieceKind::KNIGHT:
        return __getKnightMoveRowCols(isBottom, rowcol_pair);
    case PieceKind::CANNON:
        return __getCannonMoveRowCols(rowcol_pair);
    case PieceKind::BISHOP:
        return __getBishopMoveRowCols(isBottom, rowcol_pair);
    case PieceKind::ADVISOR:
        return __getAdvisorMoveRowCols(isBottom, rowcol_pair);
    case PieceKind::PAWN:
        return __getPawnMoveRowCols(isBottom, rowcol_pair);
    case PieceKind::KING:
        return __getKingMoveRowCols(isBottom, rowcol_pair);
    default:
        break;
    };
    return RowCol_pair_vector{};
}

RowCol_pair_vector Seats::getLiveRowCols(PieceColor color, wchar_t name, int col, bool getStronge) const
{
    RowCol_pair_vector rowcol_pv{};
    for (auto& seat : allSeats_) {
        auto& piece = seat->piece();
        if (piece
            && color == piece->color()
            && (name == BLANKNAME || name == piece->name())
            && (col == BLANKCOL || col == seat->col())
            && (!getStronge || PieceManager::isStronge(piece->name())))
            rowcol_pv.push_back(seat->rowCol_pair());
    }
    return rowcol_pv;
}

RowCol_pair_vector Seats::getSortPawnLiveRowCols(bool isBottom, PieceColor color, wchar_t name) const
{
    // 最多5个兵
    RowCol_pair_vector pawnRowCols{ getLiveRowCols(color, name) }, rowcol_pv{};
    // 按列建立字典，按列排序
    map<int, RowCol_pair_vector> colRowCols{};
    for_each(pawnRowCols.begin(), pawnRowCols.end(),
        [&](const RowCol_pair& rowcol_pair) {
            colRowCols[isBottom ? -rowcol_pair.second : rowcol_pair.second].push_back(rowcol_pair);
        }); // isBottom则列倒序,每列位置倒序

    // 整合成一个数组
    for_each(colRowCols.begin(), colRowCols.end(),
        [&](const pair<int, RowCol_pair_vector>& colRowCol) {
            auto col_pv = colRowCol.second;
            if (col_pv.size() > 1) { // 筛除只有一个位置的列
                if (isBottom)
                    reverse(col_pv.begin(), col_pv.end());
                copy(col_pv.begin(), col_pv.end(), back_inserter(rowcol_pv));
            }
        }); //按列存入
    return rowcol_pv;
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

RowCol_pair_vector Seats::__getKingMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(SeatManager::getKingMoveRowCols(isBottom, rowcol_pair), rowcol_pair);
}

RowCol_pair_vector Seats::__getAdvisorMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(SeatManager::getAdvisorMoveRowCols(isBottom, rowcol_pair), rowcol_pair);
}

RowCol_pair_vector Seats::__getBishopMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(__getNonObs_MoveRowCols(isBottom, rowcol_pair, SeatManager::getBishopObs_MoveRowCols), rowcol_pair);
}

RowCol_pair_vector Seats::__getKnightMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(__getNonObs_MoveRowCols(isBottom, rowcol_pair, SeatManager::getKnightObs_MoveRowCols), rowcol_pair);
}

RowCol_pair_vector Seats::__getRookMoveRowCols(const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(__getRook_MoveRowCols(rowcol_pair), rowcol_pair);
}

RowCol_pair_vector Seats::__getCannonMoveRowCols(const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(__getCannon_MoveRowCols(rowcol_pair), rowcol_pair);
}

RowCol_pair_vector Seats::__getPawnMoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair) const
{
    return __getMoveRowCols(SeatManager::getPawnMoveRowCols(isBottom, rowcol_pair), rowcol_pair);
}

RowCol_pair_vector Seats::__getMoveRowCols(const RowCol_pair_vector& rowcol_pairs, const RowCol_pair& frowcol_pair) const
{
    RowCol_pair_vector rowcol_pv{};
    auto& fseat = getSeat(frowcol_pair);
    if (fseat->piece())
        for (auto& rowcol_pair : rowcol_pairs)
            if (!fseat->isSameColor(getSeat(rowcol_pair))) // 非同一颜色
                rowcol_pv.push_back(rowcol_pair);
    return rowcol_pv;
}

RowCol_pair_vector Seats::__getNonObs_MoveRowCols(bool isBottom, const RowCol_pair& rowcol_pair,
    PRowCol_pair_vector getObs_MoveRowCols(bool, const RowCol_pair&)) const
{
    RowCol_pair_vector rowcol_pv{};
    auto obs_MoveRowCols = getObs_MoveRowCols(isBottom, rowcol_pair);
    for_each(obs_MoveRowCols.begin(), obs_MoveRowCols.end(),
        [&](const PRowCol_pair& obs_Moverowcol) {
            if (!getSeat(obs_Moverowcol.first)->piece()) // 该位置无棋子
                rowcol_pv.push_back(obs_Moverowcol.second);
        });
    return rowcol_pv;
}

RowCol_pair_vector Seats::__getRook_MoveRowCols(const RowCol_pair& rowcol_pair) const
{
    RowCol_pair_vector rowcol_pv{};
    for (auto& rowcolpair_Line : SeatManager::getRookCannonMoveRowCol_Lines(rowcol_pair))
        for (auto& rowcol_pair : rowcolpair_Line) {
            rowcol_pv.push_back(rowcol_pair);
            if (getSeat(rowcol_pair)->piece()) // 该位置有棋子
                break;
        }
    return rowcol_pv;
}

RowCol_pair_vector
Seats::__getCannon_MoveRowCols(const RowCol_pair& rowcol_pair) const
{
    RowCol_pair_vector rowcol_pv{};
    for (auto& rowcolpair_Line : SeatManager::getRookCannonMoveRowCol_Lines(rowcol_pair)) {
        bool isSkip = false; // 是否已跳棋子的标志
        for (auto rowcol_pair : rowcolpair_Line) {
            auto& piece = getSeat(rowcol_pair)->piece();
            if (!isSkip) {
                if (!piece) // 该位置无棋子
                    rowcol_pv.push_back(rowcol_pair);
                else
                    isSkip = true;
            } else if (piece) { // 该位置有棋子
                rowcol_pv.push_back(rowcol_pair);
                break;
            }
        }
    }
    return rowcol_pv;
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
        wos << rowcol.first << rowcol.second << L' ';
    return wos.str();
}
}