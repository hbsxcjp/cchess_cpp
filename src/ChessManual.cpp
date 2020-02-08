#include "ChessManual.h"
#include "Board.h"
#include "Piece.h"
#include "Seat.h"
#include "Tools.h"
#include "json.h"

extern template const RowCol_pair_vector
Board::getCanMoveRowCols(int arg1, int arg2) const;
extern template const RowCol_pair_vector
Board::getCanMoveRowCols(const wstring& arg1, RecFormat arg2) const;

namespace ChessManualSpace {

static const wchar_t FENKey[] = L"FEN";

/* ===== ChessManual::Move start. ===== */
inline int ChessManual::Move::frowcol() const { return seat_pair_.first->rowcol(); }

inline int ChessManual::Move::trowcol() const { return seat_pair_.second->rowcol(); }

const wstring ChessManual::Move::iccs() const
{
    wostringstream wos{};
    wos << PieceManager::getColICCSChar(seat_pair_.first->col()) << seat_pair_.first->row()
        << PieceManager::getColICCSChar(seat_pair_.second->col()) << seat_pair_.second->row();
    return wos.str();
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addNext()
{
    auto nextMove = make_shared<Move>();
    nextMove->setNextNo(nextNo_ + 1);
    nextMove->setOtherNo(otherNo_);
    nextMove->setPrev(weak_ptr<Move>(shared_from_this()));
    return next_ = nextMove;
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addOther()
{
    auto otherMove = make_shared<Move>();
    otherMove->setNextNo(nextNo_);
    otherMove->setOtherNo(otherNo_ + 1);
    otherMove->setPrev(weak_ptr<Move>(shared_from_this()));
    return other_ = otherMove;
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addNext(const SSeat_pair& prowcol_pair, const wstring& remark)
{

    auto nextMove = this->addNext();
    nextMove->setSeatPairRemark(prowcol_pair, remark);
    return next_ = nextMove;
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addOther(const SSeat_pair& prowcol_pair, const wstring& remark)
{

    auto otherMove = this->addOther();
    otherMove->setSeatPairRemark(prowcol_pair, remark);
    return other_ = otherMove;
}

void ChessManual::Move::setSeatPairRemark(const SSeat_pair& seat_pair, const wstring& remark)
{
    seat_pair_ = seat_pair;
    remark_ = remark;
}

vector<shared_ptr<ChessManual::Move>> ChessManual::Move::getPrevMoves()
{
    SMove thisMove{ shared_from_this() }, preMove{};
    vector<SMove> moves{ thisMove };
    while (preMove = thisMove->prev()) {
        moves.push_back(preMove);
        thisMove = preMove;
    }
    reverse(moves.begin(), moves.end());
    return moves;
}

void ChessManual::Move::done()
{
    eatPie_ = seat_pair_.first->movTo(seat_pair_.second);
}

void ChessManual::Move::undo()
{
    seat_pair_.second->movTo(seat_pair_.first, eatPie_);
}

const wstring ChessManual::Move::toString() const
{
    wostringstream wos{};
    wos << setw(2) << frowcol() << L'_' << setw(2) << trowcol()
        << L'-' << setw(4) << iccs() << L':' << setw(4) << zh()
        << L'@' << (eatPie_ ? eatPie_->name() : L'-') << L' ' << L'{' << remark() << L'}'
        << L" next:" << nextNo_ << L" other:" << otherNo_ << L" CC_Col:" << CC_ColNo_ << L'\n';
    return wos.str();
}
/* ===== ChessManual::Move end. ===== */

/* ===== ChessManual start. ===== */
ChessManual::ChessManual()
    : info_{ map<wstring, wstring>{} }
    , board_{ make_shared<Board>() } // 动态分配内存，初始化对象并指向它
    , rootMove_{ make_shared<Move>() }
    , currentMove_{ make_shared<Move>() }
{
    reset();
}

ChessManual::ChessManual(const string& infilename)
    : ChessManual()
{
    read(infilename);
}

shared_ptr<ChessManual::Move>& ChessManual::addNextMove(
    SMove& move, int frowcol, int trowcol, const wstring& remark) const
{
    return move->addNext(board_->getSeatPair(frowcol, trowcol), remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addOtherMove(
    SMove& move, int frowcol, int trowcol, const wstring& remark) const
{
    return move->addOther(board_->getSeatPair(frowcol, trowcol), remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addNextMove(
    SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const
{
    return move->addNext(board_->getSeatPair(str, fmt), remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addOtherMove(
    SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const
{
    return move->addOther(board_->getSeatPair(str, fmt), remark);
}

void ChessManual::reset()
{
    __setFENplusFromFEN(PieceManager::FirstFEN(), PieceColor::RED);
    __setBoardFromInfo();
    currentMove_ = rootMove_ = make_shared<Move>();
    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
}

void ChessManual::go()
{
    if (currentMove_->next()) {
        currentMove_ = currentMove_->next();
        currentMove_->done();
    }
}

void ChessManual::back()
{
    if (currentMove_->prev()) {
        currentMove_->undo();
        currentMove_ = currentMove_->prev();
    }
}

void ChessManual::backTo(const SMove& move)
{
    while (currentMove_ != rootMove_ && currentMove_ != move)
        back();
}

void ChessManual::goOther()
{
    if (currentMove_ != rootMove_ && currentMove_->other()) {
        currentMove_->undo();
        currentMove_ = currentMove_->other();
        currentMove_->done();
    }
}

void ChessManual::goInc(int inc)
{
    //function<void(ChessManual*)> fbward = inc > 0 ? &ChessManual::go : &ChessManual::back;
    auto fbward = mem_fn(inc > 0 ? &ChessManual::go : &ChessManual::back);
    for (int i = abs(inc); i != 0; --i)
        fbward(this);
}

void ChessManual::changeSide(ChangeType ct)
{
    vector<SMove> prevMoves{};
    if (currentMove_ != rootMove_)
        prevMoves = currentMove_->getPrevMoves();
    backTo(rootMove_);
    board_->changeSide(ct);

    if (ct != ChangeType::EXCHANGE) {
        auto changeRowcol = (ct == ChangeType::ROTATE ? &SeatManager::getRotate : &SeatManager::getSymmetry);
        //auto changeRowcol = mem_fn(ct == ChangeType::ROTATE ? &SeatManager::getRotate : &SeatManager::getSymmetry);
        function<void(const SMove&)>
            __resetMove = [&](const SMove& move) {
                __setMoveFromRowcol(move, changeRowcol(move->frowcol()), // 改成addNext形式后，需要先暂存下一个move，提取rowcol执行addNext后再丢弃
                    changeRowcol(move->trowcol()), move->remark());
                if (move->next())
                    __resetMove(move->next());
                if (move->other())
                    __resetMove(move->other());
            };
        if (rootMove_->next())
            __resetMove(rootMove_->next());
    }

    __setFENplusFromFEN(pieCharsToFEN(board_->getPieceChars()), PieceColor::RED);
    if (ct != ChangeType::ROTATE)
        __setMoveZhStrAndNums();
    for (auto& move : prevMoves)
        move->done();
}

void ChessManual::read(const string& infilename)
{
    RecFormat fmt = getRecFormat(Tools::getExtStr(infilename));
    ifstream is{};
    wifstream wis{};
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        is = ifstream(infilename, ios_base::binary);
    else
        wis = wifstream(infilename);
    switch (fmt) {
    case RecFormat::XQF:
        __readXQF(is);
        break;
    case RecFormat::BIN:
        __readBIN(is);
        break;
    case RecFormat::JSON:
        __readJSON(is);
        break;
    case RecFormat::PGN_ICCS:
        __readInfo_PGN(wis);
        __readMove_PGN_ICCSZH(wis, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __readInfo_PGN(wis);
        __readMove_PGN_ICCSZH(wis, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __readInfo_PGN(wis);
        __readMove_PGN_CC(wis);
        break;
    default:
        break;
    }
    currentMove_ = rootMove_;
    __setMoveZhStrAndNums();
}

void ChessManual::write(const string& outfilename)
{
    RecFormat fmt = getRecFormat(Tools::getExtStr(outfilename));
    ofstream os{};
    wofstream wos{};
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        os = ofstream(outfilename, ios_base::binary);
    else
        wos = wofstream(outfilename);
    switch (fmt) {
    case RecFormat::XQF:
        break;
    case RecFormat::BIN:
        __writeBIN(os);
        break;
    case RecFormat::JSON:
        __writeJSON(os);
        break;
    case RecFormat::PGN_ICCS:
        __writeInfo_PGN(wos);
        __writeMove_PGN_ICCSZH(wos, RecFormat::PGN_ICCS);
        break;
    case RecFormat::PGN_ZH:
        __writeInfo_PGN(wos);
        __writeMove_PGN_ICCSZH(wos, RecFormat::PGN_ZH);
        break;
    case RecFormat::PGN_CC:
        __writeInfo_PGN(wos);
        __writeMove_PGN_CC(wos);
        break;
    default:
        break;
    }
}

const wstring ChessManual::toString()
{
    wostringstream wos{};

    // Board test
    wos << board_->toString() << L'\n';

    /*
    __writeInfo_PGN(wos);
    __writeMove_PGN_CC(wos);

    backTo(rootMove_);
    vector<SMove> preMoves{};
    function<void(bool)>
        __printMoveBoard = [&](bool isOther) {
            isOther ? goOther() : go();
            wos << board_->toString() << currentMove_->toString() << L"\n\n";
            if (currentMove_->other()) {
                preMoves.push_back(currentMove_);
                __printMoveBoard(true);
                // 变着之前着在返回时，应予执行
                if (!preMoves.empty()) {
                    preMoves.back()->done();
                    preMoves.pop_back();
                }
            }
            if (currentMove_->next()) {
                __printMoveBoard(false);
            }
            back();
        };
    if (currentMove_->next())
        __printMoveBoard(false);
    //*/
    return wos.str();
}

void ChessManual::__setBoardFromInfo()
{
    board_->setPieces(FENTopieChars(FENplusToFEN(info_.at(FENKey))));
}

// 将用addNext代替
void ChessManual::__setMoveFromRowcol(const SMove& move,
    int frowcol, int trowcol, const wstring& remark) const
{
    move->setSeatPair(board_->getSeatPair(frowcol, trowcol));
    move->setRemark(remark);
}

// 将用addNext代替
void ChessManual::__setMoveFromStr(const SMove& move,
    const wstring& str, RecFormat fmt, const wstring& remark) const
{
    if (fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC)
        move->setSeatPairRemark(board_->getSeatPair(str, fmt), remark);
    else
        move->setSeatPairRemark(board_->getSeatPair(
                                    PieceManager::getRowFromICCSChar(str.at(1)),
                                    PieceManager::getColFromICCSChar(str.at(0)),
                                    PieceManager::getRowFromICCSChar(str.at(3)),
                                    PieceManager::getColFromICCSChar(str.at(2))),
            remark);
}

void ChessManual::__setMoveZhStrAndNums()
{
    function<void(const SMove&)>
        __setZhStrAndNums = [&](const SMove& move) {
            ++movCount_;
            maxCol_ = max(maxCol_, move->otherNo());
            maxRow_ = max(maxRow_, move->nextNo());
            move->setCC_ColNo(maxCol_); // # 本着在视图中的列数
            if (!move->remark().empty()) {
                ++remCount_;
                remLenMax_ = max(remLenMax_, static_cast<int>(move->remark().size()));
            }
            move->setZhStr(board_->getZhStr(move->getSeat_pair()));

            move->done();
            if (move->next())
                __setZhStrAndNums(move->next());
            move->undo();

            if (move->other()) {
                ++maxCol_;
                __setZhStrAndNums(move->other());
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->next())
        __setZhStrAndNums(rootMove_->next()); // 驱动函数
}

void ChessManual::__setFENplusFromFEN(const wstring& FEN, PieceColor color)
{
    info_[FENKey] = FENToFENplus(FEN, color);
}

const wstring ChessManual::__moveInfo() const
{
    wostringstream wos{};
    wos << L"【着法深度：" << maxRow_ << L", 视图宽度：" << maxCol_ << L", 着法数量：" << movCount_
        << L", 注解数量：" << remCount_ << L", 注解最长：" << remLenMax_ << L"】\n";
    return wos.str();
}

void ChessManual::__readXQF(istream& is)
{
    char Signature[3]{}, Version{}, headKeyMask{}, ProductId[4]{}, //文件标记'XQ'=$5158/版本/加密掩码/ProductId[4], 产品(厂商的产品号)
        headKeyOrA{}, headKeyOrB{}, headKeyOrC{}, headKeyOrD{},
        headKeysSum{}, headKeyXY{}, headKeyXYf{}, headKeyXYt{}, // 加密的钥匙和/棋子布局位置钥匙/棋谱起点钥匙/棋谱终点钥匙
        headQiziXY[PIECENUM]{}, // 32个棋子的原始位置
        // 用单字节坐标表示, 将字节变为十进制, 十位数为X(0-8)个位数为Y(0-9),
        // 棋盘的左下角为原点(0, 0). 32个棋子的位置从1到32依次为:
        // 红: 车马相士帅士相马车炮炮兵兵兵兵兵 (位置从右到左, 从下到上)
        // 黑: 车马象士将士象马车炮炮卒卒卒卒卒 (位置从右到左, 从下到上)PlayStepNo[2],
        PlayStepNo[2]{},
        headWhoPlay{}, headPlayResult{}, PlayNodes[4]{}, PTreePos[4]{}, Reserved1[4]{},
        // 该谁下 0-红先, 1-黑先/最终结果 0-未知, 1-红胜 2-黑胜, 3-和棋
        headCodeA_H[16]{}, TitleA[65]{}, TitleB[65]{}, //对局类型(开,中,残等)
        Event[65]{}, Date[17]{}, Site[17]{}, Red[17]{}, Black[17]{},
        Opening[65]{}, Redtime[17]{}, Blktime[17]{}, Reservedh[33]{},
        RMKWriter[17]{}, Author[17]{}; //, Other[528]{}; // 棋谱评论员/文件的作者

    is.read(Signature, 2).get(Version).get(headKeyMask).read(ProductId, 4) // = 8 bytes
        .get(headKeyOrA)
        .get(headKeyOrB)
        .get(headKeyOrC)
        .get(headKeyOrD)
        .get(headKeysSum)
        .get(headKeyXY)
        .get(headKeyXYf)
        .get(headKeyXYt) // = 16 bytes
        .read(headQiziXY, PIECENUM) // = 48 bytes
        .read(PlayStepNo, 2)
        .get(headWhoPlay)
        .get(headPlayResult)
        .read(PlayNodes, 4)
        .read(PTreePos, 4)
        .read(Reserved1, 4) // = 64 bytes
        .read(headCodeA_H, 16)
        .read(TitleA, 64)
        .read(TitleB, 64)
        .read(Event, 64)
        .read(Date, 16)
        .read(Site, 16)
        .read(Red, 16)
        .read(Black, 16)
        .read(Opening, 64)
        .read(Redtime, 16)
        .read(Blktime, 16)
        .read(Reservedh, 32)
        .read(RMKWriter, 16)
        .read(Author, 16);

    assert(Signature[0] == 0x58 || Signature[1] == 0x51);
    assert((headKeysSum + headKeyXY + headKeyXYf + headKeyXYt) % 256 == 0); // L" 检查密码校验和不对，不等于0。\n";
    assert(Version <= 18); // L" 这是一个高版本的XQF文件，您需要更高版本的XQStudio来读取这个文件。\n";

    unsigned char KeyXY{}, KeyXYf{}, KeyXYt{}, F32Keys[PIECENUM], *head_QiziXY{ (unsigned char*)headQiziXY };
    int KeyRMKSize{ 0 };
    if (Version <= 10) { // version <= 10 兼容1.0以前的版本
        KeyRMKSize = KeyXYf = KeyXYt = 0;
    } else {
        function<unsigned char(unsigned char, unsigned char)> __calkey = [](unsigned char bKey, unsigned char cKey) {
            return (((((bKey * bKey) * 3 + 9) * 3 + 8) * 2 + 1) * 3 + 8) * cKey; // % 256; // 保持为<256
        };
        KeyXY = __calkey(headKeyXY, headKeyXY);
        KeyXYf = __calkey(headKeyXYf, KeyXY);
        KeyXYt = __calkey(headKeyXYt, KeyXYf);
        KeyRMKSize = (static_cast<unsigned char>(headKeysSum) * 256 + static_cast<unsigned char>(headKeyXY)) % 32000 + 767; // % 65536
        if (Version >= 12) { // 棋子位置循环移动
            vector<unsigned char> Qixy(begin(headQiziXY), end(headQiziXY)); // 数组不能拷贝
            for (int i = 0; i != PIECENUM; ++i)
                head_QiziXY[(i + KeyXY + 1) % PIECENUM] = Qixy[i];
        }
        for (int i = 0; i != PIECENUM; ++i)
            head_QiziXY[i] -= KeyXY; // 保持为8位无符号整数，<256
    }
    int KeyBytes[4]{
        (headKeysSum & headKeyMask) | headKeyOrA,
        (headKeyXY & headKeyMask) | headKeyOrB,
        (headKeyXYf & headKeyMask) | headKeyOrC,
        (headKeyXYt & headKeyMask) | headKeyOrD
    };
    const string copyright{ "[(C) Copyright Mr. Dong Shiwei.]" };
    for (int i = 0; i != PIECENUM; ++i)
        F32Keys[i] = copyright[i] & KeyBytes[i % 4]; // ord(c)

    // 取得棋子字符串
    wstring pieceChars(90, PieceManager::nullChar());
    wstring pieChars = L"RNBAKABNRCCPPPPPrnbakabnrccppppp"; // QiziXY设定的棋子顺序
    for (int i = 0; i != PIECENUM; ++i) {
        int xy = head_QiziXY[i];
        if (xy <= 89) // 用单字节坐标表示, 将字节变为十进制,  十位数为X(0-8),个位数为Y(0-9),棋盘的左下角为原点(0, 0)
            pieceChars[xy % 10 * 9 + xy / 10] = pieChars[i];
    }
    info_ = map<wstring, wstring>{
        { L"Version", to_wstring(Version) },
        { L"Result", (map<unsigned char, wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult] },
        { L"PlayType", (map<unsigned char, wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]] },
        { L"TitleA", Tools::cvt.from_bytes(TitleA) },
        { L"Event", Tools::cvt.from_bytes(Event) },
        { L"Date", Tools::cvt.from_bytes(Date) },
        { L"Site", Tools::cvt.from_bytes(Site) },
        { L"Red", Tools::cvt.from_bytes(Red) },
        { L"Black", Tools::cvt.from_bytes(Black) },
        { L"Opening", Tools::cvt.from_bytes(Opening) },
        { L"RMKWriter", Tools::cvt.from_bytes(RMKWriter) },
        { L"Author", Tools::cvt.from_bytes(Author) },
        { FENKey, pieCharsToFEN(pieceChars) } // 可能存在不是红棋先走的情况？在readMove后再更新一下！
    };
    //__setFENplusFromPieChars(pieceChars, rootMove_->getSeat_pair().first->piece()->color());
    __setBoardFromInfo();

    function<unsigned char(unsigned char, unsigned char)>
        __sub = [](unsigned char a, unsigned char b) {
            return a - b;
        }; // 保持为<256

    auto __readBytes = [&](char* bytes, int size) {
        auto pos = is.tellg();
        is.read(bytes, size);
        if (Version > 10) // '字节解密'
            for (int i = 0; i != size; ++i)
                bytes[i] = __sub(bytes[i], F32Keys[(i + pos) % 32]);
    };

    char data[4]{}, &frc{ data[0] }, &trc{ data[1] }, &tag{ data[2] };

    auto __getRemarksize = [&]() {
        char clen[4]{};
        __readBytes(clen, 4);
        return *(int*)clen - KeyRMKSize;
    };

    //function<wstring()>
    auto __readDataAndGetRemark = [&]() {
        __readBytes(data, 4);
        int RemarkSize{};
        wstring wstr{};
        if (Version <= 10) {
            tag = ((tag & 0xF0) ? 0x80 : 0) | ((tag & 0x0F) ? 0x40 : 0);
            RemarkSize = __getRemarksize();
        } else {
            tag &= 0xE0;
            if (tag & 0x20)
                RemarkSize = __getRemarksize();
        }
        if (RemarkSize > 0) { // # 如果有注解
            char* rem = new char[RemarkSize + 1]();
            __readBytes(rem, RemarkSize);
            wstr = Tools::cvt.from_bytes(rem);
            delete[] rem;
        }
        return wstr;
    };

    function<void(const SMove&)>
        __readMove = [&](const SMove& move) {
            auto remark = __readDataAndGetRemark();
            //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
            int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
            assert(fcolrow <= 89 && tcolrow <= 89);
            __setMoveFromRowcol(move, (fcolrow % 10) * 10 + fcolrow / 10,
                (tcolrow % 10) * 10 + tcolrow / 10, remark);

            char ntag{ tag };
            if (ntag & 0x80) //# 有左子树
                __readMove(move->addNext());
            if (ntag & 0x40) // # 有右子树
                __readMove(move->addOther());
        };

    is.seekg(1024);
    rootMove_->setRemark(__readDataAndGetRemark());
    char rtag{ tag };
    if (rtag & 0x80) //# 有左子树
        __readMove(rootMove_->addNext());
}

void ChessManual::__readBIN(istream& is)
{
    char len[sizeof(int)]{};
    function<wstring()> __readWstring = [&]() {
        is.read(len, sizeof(int));
        int length{ *(int*)len };
        wstring wstr{};
        char* rem = new char[length + 1]();
        is.read(rem, length);
        wstr = Tools::cvt.from_bytes(rem);
        delete[] rem;
        return wstr;
    };

    char frowcol{}, trowcol{};
    function<void(const SMove&)>
        __readMove = [&](const SMove& move) {
            char tag{};
            is.get(frowcol).get(trowcol).get(tag);
            __setMoveFromRowcol(move, frowcol, trowcol, (tag & 0x20) ? __readWstring() : wstring{});

            if (tag & 0x80)
                __readMove(move->addNext());
            if (tag & 0x40)
                __readMove(move->addOther());
            return move;
        };

    char atag{};
    is.get(atag);
    if (atag & 0x80) {
        char len{};
        is.get(len);
        wstring key{}, value{};
        for (int i = 0; i < len; ++i) {
            key = __readWstring();
            value = __readWstring();
            info_[key] = value;
        }
    }
    __setBoardFromInfo();

    if (atag & 0x40)
        rootMove_->setRemark(__readWstring());
    if (atag & 0x20)
        __readMove(rootMove_->addNext());
}

void ChessManual::__writeBIN(ostream& os) const
{
    auto __writeWstring = [&](const wstring& wstr) {
        string str{ Tools::cvt.to_bytes(wstr) };
        int len = str.size();
        os.write((char*)&len, sizeof(int)).write(str.c_str(), len);
    };
    function<void(const SMove&)>
        __writeMove = [&](const SMove& move) {
            char tag = ((move->next() ? 0x80 : 0x00)
                | (move->other() ? 0x40 : 0x00)
                | (!move->remark().empty() ? 0x20 : 0x00));
            os.put(move->frowcol()).put(move->trowcol()).put(tag);
            if (tag & 0x20)
                __writeWstring(move->remark());
            if (tag & 0x80)
                __writeMove(move->next());
            if (tag & 0x40)
                __writeMove(move->other());
        };

    char tag = ((!info_.empty() ? 0x80 : 0x00)
        | (!rootMove_->remark().empty() ? 0x40 : 0x00)
        | (rootMove_->next() ? 0x20 : 0x00));
    os.put(tag);
    if (tag & 0x80) {
        int infoLen = info_.size();
        os.put(infoLen);
        for_each(info_.begin(), info_.end(),
            [&](const pair<wstring, wstring>& kv) {
                __writeWstring(kv.first);
                __writeWstring(kv.second);
            });
    }
    if (tag & 0x40)
        __writeWstring(rootMove_->remark());
    if (tag & 0x20)
        __writeMove(rootMove_->next());
}

void ChessManual::__readJSON(istream& is)
{
    Json::CharReaderBuilder builder;
    Json::Value root;
    JSONCPP_STRING errs;
    if (!parseFromStream(builder, is, &root, &errs))
        return;

    Json::Value infoItem{ root["info"] };
    for (auto& key : infoItem.getMemberNames())
        info_[Tools::cvt.from_bytes(key)] = Tools::cvt.from_bytes(infoItem[key].asString());
    __setBoardFromInfo();

    function<void(const SMove&, Json::Value&)>
        __readMove = [&](const SMove& move, Json::Value& item) {
            int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
            __setMoveFromRowcol(move, frowcol, trowcol,
                (item.isMember("r")) ? Tools::cvt.from_bytes(item["r"].asString()) : wstring{});

            if (item.isMember("n"))
                __readMove(move->addNext(), item["n"]);
            if (item.isMember("o"))
                __readMove(move->addOther(), item["o"]);
        };

    rootMove_->setRemark(Tools::cvt.from_bytes(root["remark"].asString()));
    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __readMove(rootMove_->addNext(), rootItem);
}

void ChessManual::__writeJSON(ostream& os) const
{
    Json::Value root{}, infoItem{};
    Json::StreamWriterBuilder builder;
    unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    for_each(info_.begin(), info_.end(),
        [&](const pair<wstring, wstring>& kv) {
            infoItem[Tools::cvt.to_bytes(kv.first)] = Tools::cvt.to_bytes(kv.second);
        });
    root["info"] = infoItem;
    function<Json::Value(const SMove&)>
        __writeItem = [&](const SMove& move) {
            Json::Value item{};
            item["f"] = move->frowcol();
            item["t"] = move->trowcol();
            if (!move->remark().empty())
                item["r"] = Tools::cvt.to_bytes(move->remark());
            if (move->next())
                item["n"] = __writeItem(move->next());
            if (move->other())
                item["o"] = __writeItem(move->other());
            return item;
        };
    root["remark"] = Tools::cvt.to_bytes(rootMove_->remark());
    if (rootMove_->next())
        root["moves"] = __writeItem(rootMove_->next());
    writer->write(root, &os);
}

void ChessManual::__readInfo_PGN(wistream& wis)
{
    wstring line{};
    wregex info{ LR"(\[(\w+)\s+\"([\s\S]*?)\"\])" };
    while (getline(wis, line) && !line.empty()) { // 以空行为终止特征
        wsmatch matches;
        if (regex_match(line, matches, info))
            info_[matches[1]] = matches[2];
    }
    __setBoardFromInfo();
}

void ChessManual::__readMove_PGN_ICCSZH(wistream& wis, RecFormat fmt)
{
    const wstring moveStr{ Tools::getWString(wis) };
    bool isPGN_ZH{ fmt == RecFormat::PGN_ZH };
    wstring otherBeginStr{ LR"((\()?)" };
    wstring boutStr{ LR"((\d+\.)?[\s...]*\b)" };
    wstring ICCSZhStr{ LR"(([)"
        + (isPGN_ZH ? PieceManager::getZhChars() : PieceManager::getICCSChars())
        + LR"(]{4})\b)" };
    wstring remarkStr{ LR"((?:\s*\{([\s\S]*?)\})?)" };
    wstring otherEndStr{ LR"(\s*(\)+)?)" }; // 可能存在多个右括号
    wregex moveReg{ otherBeginStr + boutStr + ICCSZhStr + remarkStr + otherEndStr },
        remReg{ remarkStr + LR"(1\.)" };
    wsmatch wsm{};
    if (regex_search(moveStr, wsm, remReg))
        rootMove_->setRemark(wsm.str(1));
    SMove preMove{ rootMove_ }, move{ rootMove_ };
    vector<SMove> preOtherMoves{};
    for (wsregex_iterator wtiMove{ moveStr.begin(), moveStr.end(), moveReg }, wtiEnd{};
         wtiMove != wtiEnd; ++wtiMove) {
        if ((*wtiMove)[1].matched) {
            move = preMove->addOther();
            preOtherMoves.push_back(preMove);
            if (isPGN_ZH)
                preMove->undo();
        } else
            move = preMove->addNext();
        __setMoveFromStr(move, (*wtiMove)[3], fmt, (*wtiMove)[4]);
        //if (isPGN_ZH)
        // wcout << (*wtiMove).str() << L'\n' << move->toString() << endl;
        if (isPGN_ZH)
            move->done(); // 推进board的状态变化
        //if (isPGN_ZH)
        // wcout << board_->toString() << endl;

        if ((*wtiMove)[5].matched)
            for (int num = (*wtiMove).length(5); num > 0; --num) {
                preMove = preOtherMoves.back();
                preOtherMoves.pop_back();
                if (isPGN_ZH) {
                    do {
                        move->undo();
                    } while ((move = move->prev()) != preMove);
                    preMove->done();
                }
            }
        else
            preMove = move;
    }
    if (isPGN_ZH)
        while (move != rootMove_) {
            move->undo();
            move = move->prev();
        }
}

void ChessManual::__writeInfo_PGN(wostream& wos) const
{
    for_each(info_.begin(), info_.end(),
        [&](const pair<wstring, wstring>& kv) {
            wos << L'[' << kv.first << L" \"" << kv.second << L"\"]\n";
        });
    wos << L'\n';
}

void ChessManual::__writeMove_PGN_ICCSZH(wostream& wos, RecFormat fmt) const
{
    bool isPGN_ZH{ fmt == RecFormat::PGN_ZH };
    auto __getRemarkStr = [&](const SMove& move) {
        return (move->remark().empty()) ? L"" : (L" \n{" + move->remark() + L"}\n ");
    };
    function<void(const SMove&, bool)>
        __writeMove = [&](const SMove& move, bool isOther) {
            wstring boutStr{ to_wstring((move->nextNo() + 1) / 2) + L". " };
            bool isEven{ move->nextNo() % 2 == 0 };
            wos << (isOther ? L"(" + boutStr + (isEven ? L"... " : L"")
                            : (isEven ? wstring{ L" " } : boutStr))
                << (isPGN_ZH ? move->zh() : move->iccs()) << L' '
                << __getRemarkStr(move);

            if (move->other()) {
                __writeMove(move->other(), true);
                wos << L")";
            }
            if (move->next())
                __writeMove(move->next(), false);
        };

    wos << __getRemarkStr(rootMove_);
    if (rootMove_->next())
        __writeMove(rootMove_->next(), false);
}

void ChessManual::__readMove_PGN_CC(wistream& wis)
{
    const wstring move_remStr{ Tools::getWString(wis) };
    auto pos0 = move_remStr.find(L"\n("), pos1 = move_remStr.find(L"\n【");
    wstring moveStr{ move_remStr.substr(0, min(pos0, pos1)) },
        remStr{ move_remStr.substr(min(pos0, move_remStr.size()), pos1) };
    wregex line_rg{ LR"(\n)" }, moveStrrg{ LR"(.{5})" },
        moverg{ LR"(([^…　]{4}[…　]))" },
        remrg{ LR"(\s*(\(\d+,\d+\)): \{([\s\S]*?)\})" };
    map<wstring, wstring> rems{};
    for (wsregex_iterator rp{ remStr.begin(), remStr.end(), remrg };
         rp != wsregex_iterator{}; ++rp)
        rems[(*rp)[1]] = (*rp)[2];

    vector<vector<wstring>> moveLines{};
    for (wsregex_token_iterator lineStrit{ moveStr.begin(), moveStr.end(), line_rg, -1 },
         end{};
         lineStrit != end; ++++lineStrit) {
        vector<wstring> line{};
        for (wsregex_token_iterator moveit{
                 (*lineStrit).first, (*lineStrit).second, moveStrrg, 0 };
             moveit != end; ++moveit)
            line.push_back(*moveit);
        moveLines.push_back(line);
    }
    function<void(const SMove&, int, int)>
        __readMove = [&](const SMove& move, int row, int col) {
            wstring zhStr{ moveLines[row][col] };
            if (regex_match(zhStr, moverg)) {
                __setMoveFromStr(move, zhStr.substr(0, 4), RecFormat::PGN_CC,
                    rems[L'(' + to_wstring(row) + L',' + to_wstring(col) + L')']);

                if (zhStr.back() == L'…')
                    __readMove(move->addOther(), row, col + 1);
                if (int(moveLines.size()) - 1 > row
                    && moveLines[row + 1][col][0] != L'　') {
                    move->done();
                    __readMove(move->addNext(), row + 1, col);
                    move->undo();
                }
            } else if (moveLines[row][col][0] == L'…') {
                while (moveLines[row][++col][0] == L'…')
                    ;
                __readMove(move, row, col);
            }
        };

    rootMove_->setRemark(rems[L"(0,0)"]);
    if (!moveLines.empty())
        __readMove(rootMove_->addNext(), 1, 0);
}

void ChessManual::__writeMove_PGN_CC(wostream& wos) const
{
    wostringstream remWss{};
    wstring blankStr((getMaxCol() + 1) * 5, L'　');
    vector<wstring> lineStr((getMaxRow() + 1) * 2, blankStr);
    function<void(const SMove&)>
        __setMovePGN_CC = [&](const SMove& move) {
            int firstcol{ move->CC_ColNo() * 5 }, row{ move->nextNo() * 2 };
            lineStr.at(row).replace(firstcol, 4, move->zh());
            if (!move->remark().empty())
                remWss << L"(" << move->nextNo() << L"," << move->CC_ColNo() << L"): {"
                       << move->remark() << L"}\n";

            if (move->next()) {
                lineStr.at(row + 1).at(firstcol + 2) = L'↓';
                __setMovePGN_CC(move->next());
            }
            if (move->other()) {
                int fcol{ firstcol + 4 }, num{ move->other()->CC_ColNo() * 5 - fcol };
                lineStr.at(row).replace(fcol, num, wstring(num, L'…'));
                __setMovePGN_CC(move->other());
            }
        };

    if (!currentMove_->remark().empty())
        remWss << L"(0,0): {" << currentMove_->remark() << L"}\n";
    lineStr.front().replace(0, 3, L"　开始");
    lineStr.at(1).at(2) = L'↓';
    if (rootMove_->next())
        __setMovePGN_CC(rootMove_->next());
    for (auto& line : lineStr)
        wos << line << L'\n';
    wos << remWss.str() << __moveInfo();
}
/* ===== ChessManual end. ===== */

void transDir(const string& dirfrom, const RecFormat fmt)
{
    int fcount{}, dcount{}, movcount{}, remcount{}, remlenmax{};
    string extensions{ ".xqf.pgn_iccs.pgn_zh.pgn_cc.bin.json" };
    string dirto{ dirfrom.substr(0, dirfrom.rfind('.')) + getExtName(fmt) };
    function<void(const string&, const string&)>
        __trans = [&](const string& dirfrom, const string& dirto) {
            long hFile = 0; //文件句柄
            struct _finddata_t fileinfo; //文件信息
            if (_access(dirto.c_str(), 0) != 0)
                _mkdir(dirto.c_str());
            if ((hFile = _findfirst((dirfrom + "/*").c_str(), &fileinfo)) != -1) {
                do {
                    string filename{ fileinfo.name };
                    if (fileinfo.attrib & _A_SUBDIR) { //如果是目录,迭代之
                        if (filename != "." && filename != "..") {
                            dcount += 1;
                            __trans(dirfrom + "/" + filename, dirto + "/" + filename);
                        }
                    } else { //如果是文件,执行转换
                        string infilename{ dirfrom + "/" + filename };
                        string fileto{ dirto + "/" + filename.substr(0, filename.rfind('.')) };
                        string ext_old{ Tools::getExtStr(filename) };
                        if (extensions.find(ext_old) != string::npos) {
                            fcount += 1;

                            //cout << infilename << endl;
                            ChessManual ci(infilename);
                            //cout << infilename << " read finished!" << endl;
                            //cout << fileto << endl;
                            ci.write(fileto + getExtName(fmt));
                            //cout << fileto + getExtName(fmt) << " write finished!" << endl;

                            movcount += ci.getMovCount();
                            remcount += ci.getRemCount();
                            remlenmax = max(remlenmax, ci.getRemLenMax());
                        } else
                            Tools::copyFile(infilename.c_str(), (fileto + ext_old).c_str());
                    }
                } while (_findnext(hFile, &fileinfo) == 0);
                _findclose(hFile);
            }
        };

    __trans(dirfrom, dirto);
    cout << dirfrom + " =>" << getExtName(fmt) << ": 转换" << fcount << "个文件, "
         << dcount << "个目录成功！\n   着法数量: "
         << movcount << ", 注释数量: " << remcount << ", 最大注释长度: " << remlenmax << endl;
}

void testTransDir(int fd, int td, int ff, int ft, int tf, int tt)
{
    vector<string> dirfroms{
        "c:\\棋谱\\示例文件",
        "c:\\棋谱\\象棋杀着大全",
        "c:\\棋谱\\疑难文件",
        "c:\\棋谱\\中国象棋棋谱大全"
    };
    vector<RecFormat> fmts{
        RecFormat::XQF, RecFormat::BIN, RecFormat::JSON,
        RecFormat::PGN_ICCS, RecFormat::PGN_ZH, RecFormat::PGN_CC
    };
    // 调节三个循环变量的初值、终值，控制转换目录
    for (int dir = fd; dir != td; ++dir)
        for (int fIndex = ff; fIndex != ft; ++fIndex)
            for (int tIndex = tf; tIndex != tt; ++tIndex)
                if (tIndex > 0 && tIndex != fIndex)
                    transDir(dirfroms[dir] + getExtName(fmts[fIndex]), fmts[tIndex]);
}

//*
const wstring testChessmanual()
{
    wostringstream wos{};
    ChessManual cm{};
    //cm.read("01.xqf");
    wos << cm.toString();

    /*
    write("01.bin");
    read("01.bin");

    write("01.json");
    read("01.json");

    write("01.pgn_iccs");
    read("01.pgn_iccs");

    write("01.pgn_zh");
    read("01.pgn_zh");

    write("01.pgn_cc");
    read("01.pgn_cc");

    auto str0 = toString();
    changeSide(ChangeType::EXCHANGE);
    auto str1 = toString();
    changeSide(ChangeType::ROTATE);
    auto str2 = toString();
    changeSide(ChangeType::SYMMETRY);
    auto str3 = toString();
    wos << str0 + str1 + str2 + str3;
    //*/

    return wos.str();
}
}