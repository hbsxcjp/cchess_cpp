#include "Piece.h"
#include "Seat.h"

namespace PieceSpace {

/* ===== Piece start. ===== */
Piece::Piece(wchar_t ch)
    : ch_{ ch }
    , name_{ PieceManager::getName(ch) }
    , color_{ PieceManager::getColor(ch) }
    , kind_{ PieceManager::getKindFromCh(ch) }
{
}

 const wstring Piece::toString() const
{
    wostringstream wos{};
    wos << (color() == PieceColor::RED ? L'+' : L'*') << static_cast<int>(kind())
        << ch() << name() << PieceManager::getPrintName(*this);
    return wos.str();
}
/* ===== Piece end. ===== */

/* ===== Pieces start. ===== */
Pieces::Pieces()
{
    for (auto& ch : PieceManager::getPiecesChars())
        allPieces_.push_back(make_shared<Piece>(ch));
}

const SPiece& Pieces::getOtherPiece(const SPiece& piece) const
{
    if (!piece)
        return piece;
    auto first = allPieces_.begin(), last = allPieces_.end();
    return allPieces_.at((distance(first, find(first, last, piece)) + PIECENUM / 2) % PIECENUM);
}

const vector<SPiece> Pieces::getBoardPieces(const wstring& pieceChars) const
{
    vector<SPiece> pieces(SEATNUM);
    if (pieceChars.size() == SEATNUM) {
        vector<bool> used(allPieces_.size(), false);
        int pieceIndex{ 0 }, allPiecesSize = used.size();
        for_each(pieceChars.begin(), pieceChars.end(),
            [&](wchar_t ch) {
                if (ch != PieceManager::nullChar())
                    for (int index = 0; index < allPiecesSize; ++index)
                        if (allPieces_[index]->ch() == ch && !used[index]) {
                            pieces[pieceIndex] = allPieces_[index];
                            used[index] = true;
                            break;
                        }
                ++pieceIndex;
            });
    }
    return pieces;
}

const wstring Pieces::toString() const
{
    wostringstream wos{};
    for (auto& piece : allPieces_)
        wos << piece->toString() << L' ';
    return wos.str();
}
/* ===== Pieces end. ===== */

/* ===== PieceManager start. ===== */
const wstring PieceManager::piecesChar_{ L"KAABBNNRRCCPPPPPkaabbnnrrccppppp" };
const wstring PieceManager::chChars_{ L"KABNRCPkabnrcp" };
const wstring PieceManager::preChars_{ L"前中后" };
const wstring PieceManager::nameChars_{ L"帅将仕士相象马车炮兵卒" };
const wstring PieceManager::movChars_{ L"退平进" };
const map<PieceColor, wstring> PieceManager::numChars_{
    { PieceColor::RED, L"一二三四五六七八九" },
    { PieceColor::BLACK, L"１２３４５６７８９" }
};
const wstring PieceManager::ICCSChars_{ L"abcdefghi" };
const wstring PieceManager::FirstFEN_{ L"rnbakabnr/9/1c5c1/p1p1p1p1p/9/9/P1P1P1P1P/1C5C1/9/RNBAKABNR" };
/* ===== PieceManager end. ===== */
} 