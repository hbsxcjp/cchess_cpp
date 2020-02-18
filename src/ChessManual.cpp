#include "ChessManual.h"
#include "Board.h"
#include "Piece.h"
#include "Seat.h"
#include "Tools.h"
#include "json.h"

namespace ChessManualSpace {

static const map<RecFormat, string> fmt_ext{
    { RecFormat::XQF, ".xqf" },
    { RecFormat::BIN, ".bin" },
    { RecFormat::JSON, ".json" },
    { RecFormat::PGN_ICCS, ".pgn_iccs" },
    { RecFormat::PGN_ZH, ".pgn_zh" },
    { RecFormat::PGN_CC, ".pgn_cc" }
};

wstring_convert<codecvt_utf8<wchar_t>> wscvt; // 未能成功调用

static const wchar_t FENKey[] = L"FEN";

/* ===== ChessManual::Move start. ===== */
int ChessManual::Move::frowcol() const { return SeatManager::getRowCol(prowcol_pair_.first); }

int ChessManual::Move::trowcol() const { return SeatManager::getRowCol(prowcol_pair_.second); }

const wstring ChessManual::Move::iccs() const
{
    wostringstream wos{};
    wos << PieceManager::getColICCSChar(prowcol_pair_.first.second) << prowcol_pair_.first.first
        << PieceManager::getColICCSChar(prowcol_pair_.second.second) << prowcol_pair_.second.first;
    return wos.str();
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addNext(const PRowCol_pair& prowcol_pair, const wstring& remark)
{
    auto nextMove = __addNext();
    nextMove->setPRowCol_pair(prowcol_pair);
    nextMove->setRemark(remark);
    return next_ = nextMove;
}

shared_ptr<ChessManual::Move>& ChessManual::Move::addOther(const PRowCol_pair& prowcol_pair, const wstring& remark)
{

    auto otherMove = __addOther();
    otherMove->setPRowCol_pair(prowcol_pair);
    otherMove->setRemark(remark);
    return other_ = otherMove;
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

const wstring ChessManual::Move::toString() const
{
    wostringstream wos{};
    wos << setw(2) << frowcol() << L'_' << setw(2) << trowcol()
        << L'-' << setw(4) << iccs() << L':' << setw(4) << zh()
        << L'@' << (eatPie_ ? eatPie_->name() : L'-') << L' ' << L'{' << remark() << L'}'
        << L" next:" << nextNo_ << L" other:" << otherNo_ << L" CC_Col:" << CC_ColNo_ << L'\n';
    return wos.str();
}

shared_ptr<ChessManual::Move>& ChessManual::Move::__addNext()
{
    auto nextMove = make_shared<Move>();
    nextMove->setNextNo(nextNo_ + 1);
    nextMove->setOtherNo(otherNo_);
    nextMove->setPrev(weak_ptr<Move>(shared_from_this()));
    return next_ = nextMove;
}

shared_ptr<ChessManual::Move>& ChessManual::Move::__addOther()
{
    auto otherMove = make_shared<Move>();
    otherMove->setNextNo(nextNo_);
    otherMove->setOtherNo(otherNo_ + 1);
    otherMove->setPrev(weak_ptr<Move>(shared_from_this()));
    return other_ = otherMove;
}
/* ===== ChessManual::Move end. ===== */

/* ===== ChessManual start. ===== */
ChessManual::ChessManual(const string& infilename)
    : info_{ map<wstring, wstring>{} }
    , board_{ make_shared<Board>() } // 动态分配内存，初始化对象并指向它
{
    reset();
    read(infilename);
}

void ChessManual::reset()
{
    __setFENplusFromFEN(PieceManager::FirstFEN(), PieceColor::RED);
    __setBoardFromInfo();
    currentMove_ = rootMove_ = make_shared<Move>();
    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
}

shared_ptr<ChessManual::Move>& ChessManual::addNextMove(
    SMove& move, const PRowCol_pair& prowcol_pair, const wstring& remark) const
{
    return move->addNext(prowcol_pair, remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addOtherMove(
    SMove& move, const PRowCol_pair& prowcol_pair, const wstring& remark) const
{
    return move->addOther(prowcol_pair, remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addNextMove(
    SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const
{
    return move->addNext(__getPRowCol_pair(str, fmt), remark);
}

shared_ptr<ChessManual::Move>& ChessManual::addOtherMove(
    SMove& move, const wstring& str, RecFormat fmt, const wstring& remark) const
{
    return move->addOther(__getPRowCol_pair(str, fmt), remark);
}

void ChessManual::done(const SMove& move)
{
    move->setEatPie(board_->doneMove(move->getPRowCol_pair()));
}

void ChessManual::undo(const SMove& move)
{
    board_->undoMove(move->getPRowCol_pair(), move->eatPie());
}

void ChessManual::go()
{
    if (currentMove_->next()) {
        currentMove_ = currentMove_->next();
        done(currentMove_);
    }
}

void ChessManual::back()
{
    if (currentMove_->prev()) {
        undo(currentMove_);
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
        undo(currentMove_);
        currentMove_ = currentMove_->other();
        done(currentMove_);
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
                auto move_cp = move; //复制一个副本，用来修改内部数据
                move_cp->setPRowCol_pair(make_pair(changeRowcol(move->getPRowCol_pair().first),
                    changeRowcol(move->getPRowCol_pair().second)));
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
        done(move);
}

void ChessManual::read(const string& infilename)
{
    if (infilename.empty())
        return;
    ifstream is;
    wifstream wis;
    RecFormat fmt{ RecFormat::XQF };
    fmt = getRecFormat(Tools::getExtStr(infilename));
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        is.open(infilename, ios_base::binary);
    else
        wis.open(infilename);
    if (is.fail() || wis.fail())
        return;

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
    if (outfilename.empty())
        return;
    ofstream os{};
    wofstream wos{};
    RecFormat fmt{ RecFormat::PGN_CC };
    fmt = getRecFormat(Tools::getExtStr(outfilename));
    if (fmt == RecFormat::XQF || fmt == RecFormat::BIN || fmt == RecFormat::JSON)
        os.open(outfilename, ios_base::binary);
    else
        wos.open(outfilename);
    if (os.fail() || wos.fail())
        return;

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

bool ChessManual::isBottomSide(PieceColor color) const { return board_->isBottomSide(color); }

const wstring ChessManual::getPieceChars() const { return board_->getPieceChars(); }

const wstring ChessManual::getBoardStr() const { return board_->toString(); }

const wstring ChessManual::getMoveStr() const
{
    wostringstream wos{};
    __writeMove_PGN_CC(wos);
    return wos.str();
}

const wstring ChessManual::toString()
{
    wostringstream wos{};

    // Board test
    //wos << board_->toString() << L'\n';

    __writeInfo_PGN(wos);
    __writeMove_PGN_CC(wos);

    /*
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
                    //preMoves.back()->done();
                    done(preMoves.back());
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

void ChessManual::__setFENplusFromFEN(const wstring& FEN, PieceColor color)
{
    info_[FENKey] = FENToFENplus(FEN, color);
}

void ChessManual::__setBoardFromInfo()
{
    board_->setBoard(FENTopieChars(FENplusToFEN(info_.at(FENKey))));
}

PRowCol_pair ChessManual::__getPRowCol_pair(const wstring& str, RecFormat fmt) const
{
    if (fmt == RecFormat::PGN_ZH || fmt == RecFormat::PGN_CC)
        return board_->getPRowCol_pair(str);
    else // RecFormat::PGN_ICCS
        return make_pair(
            make_pair(PieceManager::getRowFromICCSChar(str.at(1)), PieceManager::getColFromICCSChar(str.at(0))),
            make_pair(PieceManager::getRowFromICCSChar(str.at(3)), PieceManager::getColFromICCSChar(str.at(2))));
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
            move->setZhStr(board_->getZHStr(move->getPRowCol_pair()));

            //wcout << move->zh() << L'\n' << board_->toString() << L'\n' << endl;
            done(move);
            if (move->next())
                __setZhStrAndNums(move->next());
            undo(move);

            if (move->other()) {
                ++maxCol_;
                __setZhStrAndNums(move->other());
            }
        };

    movCount_ = remCount_ = remLenMax_ = maxRow_ = maxCol_ = 0;
    if (rootMove_->next())
        __setZhStrAndNums(rootMove_->next()); // 驱动函数
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

    //wcout << __LINE__ << L":" << pieceChars << endl;
    info_ = map<wstring, wstring>{
        { L"Version", to_wstring(Version) },
        { L"Result", (map<unsigned char, wstring>{ { 0, L"未知" }, { 1, L"红胜" }, { 2, L"黑胜" }, { 3, L"和棋" } })[headPlayResult] },
        { L"PlayType", (map<unsigned char, wstring>{ { 0, L"全局" }, { 1, L"开局" }, { 2, L"中局" }, { 3, L"残局" } })[headCodeA_H[0]] },
        { L"TitleA", Tools::s2ws(TitleA) },
        { L"Event", Tools::s2ws(Event) },
        { L"Date", Tools::s2ws(Date) },
        { L"Site", Tools::s2ws(Site) },
        { L"Red", Tools::s2ws(Red) },
        { L"Black", Tools::s2ws(Black) },
        { L"Opening", Tools::s2ws(Opening) },
        { L"RMKWriter", Tools::s2ws(RMKWriter) },
        { L"Author", Tools::s2ws(Author) },
        { FENKey, pieCharsToFEN(pieceChars) } // 可能存在不是红棋先走的情况？在readMove后再更新一下！
    };

    //wcout << __LINE__ << L":" << pieceChars << endl;
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
            wstr = Tools::s2ws(rem);
            delete[] rem;
        }
        return wstr;
    };

    function<void(SMove&, bool)>
        __readMove = [&](SMove& move, bool isOther) {
            auto remark = __readDataAndGetRemark();
            //# 一步棋的起点和终点有简单的加密计算，读入时需要还原
            int fcolrow = __sub(frc, 0X18 + KeyXYf), tcolrow = __sub(trc, 0X20 + KeyXYt);
            assert(fcolrow <= 89 && tcolrow <= 89);

            auto prowcol_pair = make_pair(make_pair(fcolrow % 10, fcolrow / 10), make_pair(tcolrow % 10, tcolrow / 10));
            auto& newMove = (isOther ? addOtherMove(move, prowcol_pair, remark) : addNextMove(move, prowcol_pair, remark));

            char ntag{ tag };
            if (ntag & 0x80) //# 有左子树
                __readMove(newMove, false);
            if (ntag & 0x40) // # 有右子树
                __readMove(newMove, true);
        };

    is.seekg(1024);
    rootMove_->setRemark(__readDataAndGetRemark());
    char rtag{ tag };
    //wcout << __LINE__ << L":" << rootMove_->remark() << endl;

    if (rtag & 0x80) //# 有左子树
        __readMove(rootMove_, false);
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
        wstr = Tools::s2ws(rem);
        delete[] rem;
        return wstr;
    };

    char frowcol{}, trowcol{};
    function<void(SMove&, bool)>
        __readMove = [&](SMove& move, bool isOther) {
            char tag{};
            is.get(frowcol).get(trowcol).get(tag);
            auto prowcol_pair = make_pair(SeatManager::getRowCol_pair(frowcol), SeatManager::getRowCol_pair(trowcol));
            auto remark = (tag & 0x20) ? __readWstring() : wstring{};
            auto& newMove = (isOther ? addOtherMove(move, prowcol_pair, remark) : addNextMove(move, prowcol_pair, remark));

            if (tag & 0x80)
                __readMove(newMove, false);
            if (tag & 0x40)
                __readMove(newMove, true);
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
        __readMove(rootMove_, false);
}

void ChessManual::__writeBIN(ostream& os) const
{
    auto __writeWstring = [&](const wstring& wstr) {
        string str{ Tools::ws2s(wstr) };
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
        info_[Tools::s2ws(key)] = Tools::s2ws(infoItem[key].asString());
    __setBoardFromInfo();

    function<void(SMove&, bool, Json::Value&)>
        __readMove = [&](SMove& move, bool isOther, Json::Value& item) {
            int frowcol{ item["f"].asInt() }, trowcol{ item["t"].asInt() };
            auto prowcol_pair = make_pair(SeatManager::getRowCol_pair(frowcol), SeatManager::getRowCol_pair(trowcol));
            auto remark = (item.isMember("r") ? Tools::s2ws(item["r"].asString()) : wstring{});
            auto& newMove = (isOther ? addOtherMove(move, prowcol_pair, remark) : addNextMove(move, prowcol_pair, remark));

            if (item.isMember("n"))
                __readMove(newMove, false, item["n"]);
            if (item.isMember("o"))
                __readMove(newMove, true, item["o"]);
        };

    rootMove_->setRemark(Tools::s2ws(root["remark"].asString()));
    Json::Value rootItem{ root["moves"] };
    if (!rootItem.isNull())
        __readMove(rootMove_, false, rootItem);
}

void ChessManual::__writeJSON(ostream& os) const
{
    Json::Value root{}, infoItem{};
    Json::StreamWriterBuilder builder;
    unique_ptr<Json::StreamWriter> writer(builder.newStreamWriter());
    for_each(info_.begin(), info_.end(),
        [&](const pair<wstring, wstring>& kv) {
            infoItem[Tools::ws2s(kv.first)] = Tools::ws2s(kv.second);
        });
    root["info"] = infoItem;
    function<Json::Value(const SMove&)>
        __writeItem = [&](const SMove& move) {
            Json::Value item{};
            item["f"] = move->frowcol();
            item["t"] = move->trowcol();
            if (!move->remark().empty())
                item["r"] = Tools::ws2s(move->remark());
            if (move->next())
                item["n"] = __writeItem(move->next());
            if (move->other())
                item["o"] = __writeItem(move->other());
            return item;
        };
    root["remark"] = Tools::ws2s(rootMove_->remark());
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
            preOtherMoves.push_back(preMove);
            if (isPGN_ZH)
                undo(preMove);
            move = addOtherMove(preMove, (*wtiMove)[3], fmt, (*wtiMove)[4]);
        } else
            move = addNextMove(preMove, (*wtiMove)[3], fmt, (*wtiMove)[4]);
        if (isPGN_ZH)
            done(move); // 推进board的状态变化

        if ((*wtiMove)[5].matched)
            for (int num = (*wtiMove).length(5); num > 0; --num) {
                preMove = preOtherMoves.back();
                preOtherMoves.pop_back();
                if (isPGN_ZH) {
                    do {
                        undo(move);
                    } while ((move = move->prev()) != preMove);
                    done(preMove);
                }
            }
        else
            preMove = move;
    }
    if (isPGN_ZH)
        while (move != rootMove_) {
            undo(move);
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
    function<void(SMove&, bool, int, int)>
        __readMove = [&](SMove& move, bool isOther, int row, int col) {
            wstring zhStr{ moveLines[row][col] };
            if (regex_match(zhStr, moverg)) {
                wstring zhStr0{ zhStr.substr(0, 4) },
                    remark{ rems[L'(' + to_wstring(row) + L',' + to_wstring(col) + L')'] };
                auto& newMove = (isOther ? addOtherMove(move, zhStr0, RecFormat::PGN_CC, remark)
                                         : addNextMove(move, zhStr0, RecFormat::PGN_CC, remark));

                if (zhStr.back() == L'…') {
                    int inc = 1;
                    while (moveLines[row][col + inc].front() == L'…')
                        ++inc;
                    __readMove(newMove, true, row, col + inc);
                }
                if (int(moveLines.size()) - 1 > row
                    && moveLines[row + 1][col][0] != L'　') {
                    done(newMove);
                    __readMove(newMove, false, row + 1, col);
                    undo(newMove);
                }
            }
        };

    rootMove_->setRemark(rems[L"(0,0)"]);
    if (!moveLines.empty())
        __readMove(rootMove_, false, 1, 0);
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

const string getExtName(const RecFormat fmt)
{
    return fmt_ext.at(fmt);
}

RecFormat getRecFormat(const string& ext)
{
    for (auto& fmtext : fmt_ext)
        if (ext == fmtext.second)
            return fmtext.first;
    return RecFormat::PGN_CC;
}

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
    //*
    cm.read("01.xqf");

    cm.write("01.bin");
    cm.read("01.bin");

    cm.write("01.json");
    cm.read("01.json");

    cm.write("01.pgn_iccs");
    cm.read("01.pgn_iccs");

    cm.write("01.pgn_zh");
    cm.read("01.pgn_zh");

    cm.write("01.pgn_cc");
    cm.read("01.pgn_cc");

    cm.write("01.pgn_cc");
    cm.read("01.pgn_cc");
    //*/
    wos << boolalpha << cm.isBottomSide(PieceColor::RED) << L'\n'
        << cm.getPieceChars() << L'\n' << cm.getBoardStr().c_str();
    wos << cm.toString();

    return wos.str();
}
}