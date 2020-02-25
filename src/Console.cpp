#include "Console.h"
#include "ChessManual.h"
#include "Piece.h"

namespace ConsoleSpace {

static constexpr int CHARSSIZE = 1024;
static DWORD rwNum; // 公用变量
static constexpr wchar_t PROGRAMNAME[] = L"中国象棋 ";

static constexpr SHORT WINROWS = 47, WINCOLS = 130;
static constexpr COORD HOMEPOS = { 0, 0 };
static constexpr SHORT BOARDROWS = 10 + 9, BOARDCOLS = (9 + 8) * 2, BOARDTITLEH = 2;
static constexpr SHORT SHADOWCOLS = 2, SHADOWROWS = 1, BORDERCOLS = 2, BORDERROWS = 1;
static constexpr SHORT STATUSROWS = 2;

static constexpr SMALL_RECT WINRECT = { 0, 0, SHORT(WINCOLS - 1), SHORT(WINROWS - 1) };

static constexpr SMALL_RECT MenuRect = { WINRECT.Left, WINRECT.Top, WINRECT.Right, SHADOWROWS };
static constexpr SMALL_RECT iMenuRect = { MenuRect.Left, MenuRect.Top, SHORT(MenuRect.Right - SHADOWCOLS), SHORT(MenuRect.Bottom - 1) };

static constexpr SMALL_RECT StatusRect = { WINRECT.Left, SHORT(WINRECT.Bottom - STATUSROWS - SHADOWROWS + 1), WINRECT.Right, WINRECT.Bottom };
static constexpr SMALL_RECT iStatusRect = { StatusRect.Left, StatusRect.Top, SHORT(StatusRect.Right - SHADOWCOLS), SHORT(StatusRect.Bottom - 1) };

static constexpr SMALL_RECT BoardRect = { WINRECT.Left + SHADOWCOLS, SHORT(MenuRect.Bottom + 1 + SHADOWROWS),
    SHORT(WINRECT.Left + 1 + BOARDCOLS + BORDERCOLS * 2 + SHADOWCOLS * 2), SHORT(MenuRect.Bottom + BOARDROWS + (SHADOWROWS + BORDERROWS + BOARDTITLEH) * 2) };
static constexpr SMALL_RECT iBoardRect = { SHORT(BoardRect.Left + 1 + BORDERCOLS), SHORT(BoardRect.Top + BORDERROWS),
    SHORT(BoardRect.Right - BORDERCOLS - SHADOWCOLS), SHORT(BoardRect.Bottom - BORDERROWS - SHADOWROWS) };

static constexpr SMALL_RECT CurmoveRect = { BoardRect.Left, SHORT(BoardRect.Bottom + SHADOWROWS + 1),
    BoardRect.Right, SHORT(StatusRect.Top - 1 - SHADOWROWS) };
static constexpr SMALL_RECT iCurmoveRect = { SHORT(CurmoveRect.Left + BORDERCOLS), SHORT(CurmoveRect.Top + BORDERROWS),
    SHORT(CurmoveRect.Right - BORDERCOLS - SHADOWCOLS), SHORT(CurmoveRect.Bottom - BORDERROWS - SHADOWROWS) };
/*
static constexpr COORD iCurmoveCOORD = { iCurmoveRect.Right - iCurmoveRect.Left + 1, iCurmoveRect.Bottom - iCurmoveRect.Top + 1 };
static SMALL_RECT iCurmoveRect_x = { 0, 0, SHORT(iCurmoveCOORD.X - 1), SHORT(iCurmoveCOORD.Y - 1) };
static CHAR_INFO curmoveCharBuf[iCurmoveCOORD.Y * iCurmoveCOORD.X];
//*/
static constexpr SMALL_RECT MoveRect = { SHORT(BoardRect.Right + 1 + SHADOWCOLS), BoardRect.Top,
    SHORT(WINRECT.Right - SHADOWCOLS), CurmoveRect.Bottom };
static constexpr SMALL_RECT iMoveRect = { SHORT(MoveRect.Left + BORDERCOLS), SHORT(MoveRect.Top + BORDERROWS),
    SHORT(MoveRect.Right - BORDERCOLS - SHADOWCOLS), iCurmoveRect.Bottom }; // 填充字符区域

/*
颜色属性由两个十六进制数字指定 -- 第一个对应于背景，第二个对应于前景。每个数字可以为以下任何值:
    0 = 黑色       8 = 灰色       1 = 蓝色       9 = 淡蓝色    2 = 绿色       A = 淡绿色
    3 = 浅绿色     B = 淡浅绿色   4 = 红色       C = 淡红色    5 = 紫色       D = 淡紫色
    6 = 黄色       E = 淡黄色     7 = 白色       F = 亮白色
//*/
static constexpr WORD WINATTR[] = { 0x07, 0x77 };
static constexpr WORD MENUATTR[] = { 0x80, 0x4F };
static constexpr WORD BOARDATTR[] = { 0xF8, 0xE2 };
static constexpr WORD CURMOVEATTR[] = { 0x70, 0xB4 };
static constexpr WORD MOVEATTR[] = { 0x70, 0x1F };
static constexpr WORD STATUSATTR[] = { 0xF0, 0x5F };
static constexpr WORD SHADOWATTR[] = { 0x08, 0x82 };

static constexpr WORD ShowMenuAttr[] = { 0x80, 0x4F };
static constexpr WORD SelMenuAttr[] = { 0x10, 0x2F };
static constexpr WORD RedSideAttr[] = { 0x0C | (BOARDATTR[0] & 0xF0), 0x0C | (BOARDATTR[1] & 0xF0) };
static constexpr WORD BlackSideAttr[] = { 0x00 | (BOARDATTR[0] & 0xF0), 0x00 | (BOARDATTR[1] & 0xF0) };
static constexpr WORD CurmoveAttr[] = { 0x0C | (CURMOVEATTR[0] & 0xF0), 0x0C | (CURMOVEATTR[1] & 0xF0) };
static constexpr WORD RedAttr[] = { 0xCF, 0xCF };
static constexpr WORD BlackAttr[] = { 0x0F, 0x0F };
static constexpr WORD SelRedAttr[] = { 0xFC, 0xC0 };
static constexpr WORD SelBlackAttr[] = { 0xF0, 0xE0 };

static const wchar_t* const TabChars[] = { L"─│┌┐└┘", L"═║╔╗╚╝" };

Console::Console(const string& fileName)
    : hIn_{ GetStdHandle(STD_INPUT_HANDLE) }
    , hOut_{ CreateConsoleScreenBuffer(
          GENERIC_READ | GENERIC_WRITE, // read/write access
          FILE_SHARE_READ | FILE_SHARE_WRITE, // shared
          nullptr, // default security attributes
          CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
          NULL) } //*/
    , cm_{ make_shared<ChessManual>(fileName) }
{
    SetConsoleCP(936);
    SetConsoleOutputCP(936);
    SetConsoleMode(hIn_, ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT); //ENABLE_PROCESSED_INPUT |
    SetConsoleTitleW(PROGRAMNAME); // 设置窗口标题

    SetConsoleScreenBufferSize(hOut_, { WINCOLS, WINROWS });
    CONSOLE_CURSOR_INFO cInfo = { 5, false };
    SetConsoleCursorInfo(hOut_, &cInfo);
    SetConsoleWindowInfo(hOut_, true, &WINRECT);
    SetConsoleActiveScreenBuffer(hOut_);
    //SetConsoleTextAttribute(hOut_, WINATTR[thema_]);
    //GetConsoleScreenBufferInfo(hOut_, &bInfo); // 获取窗口信息

    __cleanAreaAttr(WINATTR[thema_], WINRECT);
    __initArea(MENUATTR[thema_], MenuRect, false);
    __initArea(STATUSATTR[thema_], StatusRect, false);
    map<WORD, SMALL_RECT> rectAttrs = {
        { BOARDATTR[thema_], BoardRect },
        { CURMOVEATTR[thema_], CurmoveRect },
        { MOVEATTR[thema_], MoveRect }
    };
    for (const auto& rectAttr : rectAttrs)
        __initArea(rectAttr.first, rectAttr.second);

    __initMenu();

    __writeAreas();
    __operateWin();
}

Console::~Console()
{
    function<void(Menu*)>
        __delMenu = [&](Menu* menu) {
            if (menu == nullptr)
                return;
            __delMenu(menu->brotherMenu);
            __delMenu(menu->childMenu);
            delete menu;
        };

    CloseHandle(hOut_);
    __delMenu(rootMenu_);
}

void Console::__operateWin()
{
    int oldArea{ area_ };
    auto __keyEventProc = [&](const KEY_EVENT_RECORD& ker) {
        if (!ker.bKeyDown)
            return;
        WORD key = ker.wVirtualKeyCode;
        if (key == VK_TAB) {
            if (area_ == MENUA)
                __cleanSubMenuArea();
            area_ = (area_ + ((ker.dwControlKeyState & SHIFT_PRESSED) ? -1 : 1) + 4) % 4; // 四个区域循环
            __writeStatus();
            return;
        }
        if (ker.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) {
            if (area_ == MENUA) {
                __cleanSubMenuArea();
                area_ = oldArea;
                return;
            } else
                area_ = MENUA;
        }
        switch (area_) { // 区分不同区域, 进行操作
        case MOVEA:
            __operateMove(key);
            break;
        case CURMOVEA:
            __operateCurMove(key);
            break;
        case BOARDA:
            __operateBoard(key);
            break;
        case MENUA:
            if (__operateMenu(ker))
                area_ = oldArea;
            break;
        default:
            break;
        }
    };

    auto __mouseEventProc = [&](const MOUSE_EVENT_RECORD& ker) {

    };

    INPUT_RECORD irInBuf[128];
    while (true) {
        ReadConsoleInput(hIn_, irInBuf, 128, &rwNum);
        for (DWORD i = 0; i < rwNum; i++) {
            auto ker = irInBuf[i].Event.KeyEvent;
            switch (irInBuf[i].EventType) {
            case KEY_EVENT: // keyboard input
                if (ker.wVirtualKeyCode == VK_ESCAPE && ker.bKeyDown
                    && area_ != MENUA)
                    return;
                __keyEventProc(ker);
                break;
            case MOUSE_EVENT: // mouse input
                __mouseEventProc(irInBuf[i].Event.MouseEvent);
                break;
            default:
                break;
            }
            if (ker.bKeyDown)
                __writeStatus();
        }
    }
}

bool Console::__operateMenu(const KEY_EVENT_RECORD& ker)
{
    // 菜单顶层第i个
    auto __getTopIndexMenu = [&](int index = 0) {
        auto menu = rootMenu_->brotherMenu;
        while (index-- > 0 && menu->brotherMenu)
            menu = menu->brotherMenu;
        return menu;
    };

    // 同组菜单顶层
    auto __getTopMenu = [&](Menu* menu) {
        while (menu->childIndex != 0)
            menu = menu->preMenu;
        return menu;
    };

    // 同组菜单第n个
    auto __getLevelTopMenu = [&](Menu* menu, int level) {
        menu = __getTopMenu(menu);
        while (level-- > 0 && menu->childMenu != nullptr)
            menu = menu->childMenu;
        return menu;
    };

    // 菜单底部
    auto __getBottomMenu = [&](Menu* menu) {
        while (menu->childMenu != nullptr)
            menu = menu->childMenu;
        return menu;
    };

    bool selected{ false };
    switch (ker.wVirtualKeyCode) {
    case 'F':
        curMenu_ = __getTopIndexMenu()->childMenu;
        break;
    case 'B':
        curMenu_ = __getTopIndexMenu(1)->childMenu;
        break;
    case 'S':
        curMenu_ = __getTopIndexMenu(2)->childMenu;
        break;
    case 'A':
        curMenu_ = __getTopIndexMenu(3)->childMenu;
        break;
    case VK_DOWN:
        if (curMenu_ == rootMenu_)
            curMenu_ = __getTopIndexMenu();
        else if (curMenu_->childMenu != nullptr)
            curMenu_ = curMenu_->childMenu;
        else
            curMenu_ = __getTopMenu(curMenu_);
        break;
    case VK_UP:
        if (curMenu_ != __getTopMenu(curMenu_))
            curMenu_ = curMenu_->preMenu;
        else
            curMenu_ = __getBottomMenu(curMenu_);
        break;
    case VK_HOME:
        curMenu_ = __getTopMenu(curMenu_);
        break;
    case VK_END:
        curMenu_ = __getBottomMenu(curMenu_);
        break;
    case VK_LEFT:
        if (curMenu_->childIndex == 0) { // 顶层菜单
            if (curMenu_->preMenu != rootMenu_)
                curMenu_ = curMenu_->preMenu;
            else
                curMenu_ = __getTopIndexMenu(3);
        } else {
            auto tmenu = __getTopMenu(curMenu_);
            if (tmenu->preMenu->brotherIndex > 0) // 向左
                tmenu = tmenu->preMenu;
            else
                tmenu = __getTopIndexMenu(3);
            curMenu_ = __getLevelTopMenu(tmenu, curMenu_->childIndex);
        }
        break;
    case VK_RIGHT:
        if (curMenu_->childIndex == 0) { // 顶层菜单
            if (curMenu_->brotherMenu != nullptr)
                curMenu_ = curMenu_->brotherMenu;
            else
                curMenu_ = __getTopIndexMenu();
        } else {
            auto tmenu = __getTopMenu(curMenu_);
            if (tmenu->brotherMenu != nullptr) // 向右
                tmenu = tmenu->brotherMenu;
            else
                tmenu = __getTopIndexMenu();
            curMenu_ = __getLevelTopMenu(tmenu, curMenu_->childIndex);
        }
        break;
    case VK_RETURN:
    case VK_ESCAPE:
        selected = true;
        break;
    default:
        curMenu_ = __getTopIndexMenu();
        break;
    }

    auto __getTopSize = [&](Menu* menu) {
        return int(menu->name.size()) + 3;
    };

    // 菜单最大尺寸
    auto __getMaxSize = [&](Menu* menu) {
        menu = __getTopMenu(menu)->childMenu;
        int maxSize = menu->name.size();
        while ((menu = menu->childMenu) != nullptr)
            maxSize = max(maxSize, int(menu->name.size()));
        return maxSize * 2; // 中文字符占两个位置
    };

    // 同组菜单组合成字符串
    auto __getWstr = [&](Menu* menu) {
        menu = __getTopMenu(menu)->childMenu;
        wstring wstr{ menu->name + L'\n' };
        while ((menu = menu->childMenu) != nullptr)
            wstr += menu->name + L'\n';
        return wstr;
    };

    auto __getPosL = [&](Menu* menu) {
        SHORT width{ 0 };
        menu = __getTopMenu(menu);
        while ((menu = menu->preMenu)->brotherIndex > 0) // 顶层菜单往左推进
            width += __getTopSize(menu);
        return width;
    };

    if (curMenu_ == nullptr || curMenu_ == rootMenu_)
        return selected;
    SHORT level = curMenu_->childIndex;
    SHORT posL = __getPosL(curMenu_),
          posT = iMenuRect.Bottom + (level == 0 ? 0 : 1),
          posR = posL + (level == 0 ? __getTopSize(curMenu_) : __getMaxSize(curMenu_)) + SHADOWCOLS,
          posB = posT + (level == 0 ? 0 : __getBottomMenu(curMenu_)->childIndex - 1) + SHADOWROWS;

    SMALL_RECT rect = { posL, posT, posR, posB };

    __cleanSubMenuArea();
    if (!selected) {
        if (level > 0) {
            //__cleanArea(ShowMenuAttr[thema_], rect);
            __initArea(ShowMenuAttr[thema_], rect, false);
            __writeAreaLineChars(SelMenuAttr[thema_], __getWstr(curMenu_).c_str(), rect);
        }
        __cleanAreaAttr(SelMenuAttr[thema_], SMALL_RECT{ rect.Left, level, SHORT(rect.Right - SHADOWCOLS), level }); // 位置在clean之后
        __writeStatus();
    }
    return selected;
}

void Console::__operateBoard(WORD key) {}

void Console::__operateMove(WORD key) {}

void Console::__operateCurMove(WORD key) {}

void Console::__writeAreas()
{
    __writeBoard();
    __writeCurmove();
    __writeMove();
    __writeStatus();
}

void Console::__writeBoard()
{
    __writeAreaLineChars(BOARDATTR[thema_], cm_->getBoardStr().c_str(), iBoardRect);
    bool bottomIsRed{ cm_->isBottomSide(PieceColor::RED) };
    WORD bottomAttr{ bottomIsRed ? RedSideAttr[thema_] : BlackSideAttr[thema_] },
        topAttr{ bottomIsRed ? BlackSideAttr[thema_] : RedSideAttr[thema_] };
    // 顶、底两行上颜色
    for (int row = 0; row < BOARDTITLEH; ++row) {
        FillConsoleOutputAttribute(hOut_, topAttr, BOARDCOLS, { iBoardRect.Left, SHORT(iBoardRect.Top + row) }, &rwNum);
        FillConsoleOutputAttribute(hOut_, bottomAttr, BOARDCOLS, { iBoardRect.Left, SHORT(iBoardRect.Bottom - row) }, &rwNum);
    }
    // 棋子文字上颜色
    const wstring pieceChars{ cm_->getPieceChars() };
    for (int i = 0; i < SEATNUM; ++i) {
        wchar_t ch = pieceChars[i];
        if (ch == PieceManager::nullChar())
            continue;
        // 字符属性函数不识别全角字符，均按半角字符计数
        FillConsoleOutputAttribute(hOut_, (PieceManager::getColor(ch) == PieceColor::RED ? RedAttr[thema_] : BlackAttr[thema_]), 2,
            { SHORT(iBoardRect.Left + (i % BOARDCOLNUM) * 4), SHORT(iBoardRect.Bottom - BOARDTITLEH - (i / BOARDCOLNUM) * 2) }, &rwNum);
    }
}

void Console::__writeCurmove()
{
    __writeAreaLineChars(CURMOVEATTR[thema_], cm_->getCurmoveStr().c_str(), iCurmoveRect, cmFirstRow_, cmFirstCol_);
    int cols = iCurmoveRect.Right - iCurmoveRect.Left + 1;
    for (int row : { 2, 8, 9 })
        FillConsoleOutputAttribute(hOut_, CurmoveAttr[thema_], cols, { iCurmoveRect.Left, SHORT(iCurmoveRect.Top + row) }, &rwNum);
}

void Console::__writeMove()
{
    __writeAreaLineChars(MOVEATTR[thema_], cm_->getMoveStr().c_str(), iMoveRect, mFirstRow_, mFirstCol_);
}

void Console::__writeStatus()
{
    wostringstream wos{};
    switch (area_) {
    case MOVEA:
        wos << L"【着法】";
        break;
    case CURMOVEA:
        wos << L"【详解】";
        break;
    case BOARDA:
        wos << L"【棋盘】";
        break;
    case MENUA:
        wos << L"【菜单】";
        if (curMenu_ != rootMenu_)
            wos << curMenu_->name << L": " << curMenu_->desc;
        break;
    default:
        break;
    }
    __cleanAreaChar(iStatusRect);
    __writeAreaLineChars(STATUSATTR[thema_], wos.str().c_str(), iStatusRect);
}

void Console::__writeAreaLineChars(WORD attr, const wchar_t* lineChars, const SMALL_RECT& rc, int firstRow, int firstCol, bool cutLine)
{
    int cols = rc.Right - rc.Left + 1;
    static wchar_t tempLineChar[CHARSSIZE];
    auto __getLine = [&]() {
        int srcIndex = 0, desIndex = 0, showCols = 0;
        wchar_t wch = lineChars[0];
        while (wch != L'\x0' && wch != L'\n') {
            tempLineChar[desIndex++] = wch;
            ++showCols;
            if (wch >= 0x2500) {
                ++showCols; // 显示位置加一
                if (wch <= 0x2573) // 制表字符后加一空格 wch >= 0x2500 &&
                    tempLineChar[desIndex++] = L' ';
            }
            wch = lineChars[++srcIndex];
            // 已至行尾最后一个 或 行尾前一个且下一个为全角
            if (cutLine && (showCols == cols || (showCols == cols - 1 && wch > 0x2573)))
                break;
        }
        if (wch == L'\n')
            ++srcIndex; // 消除换行符
        lineChars += srcIndex;
        tempLineChar[desIndex] = L'\x0';
        return desIndex;
    };

    while (firstRow-- > 0) // 去掉开始数行
        __getLine();
    int showSize;
    //__cleanAreaChar(rc);
    for (SHORT row = rc.Top; row <= rc.Bottom; ++row)
        if ((showSize = __getLine() - firstCol) > 0)
            WriteConsoleOutputCharacterW(hOut_, tempLineChar, showSize, COORD{ rc.Left, row }, &rwNum);
}

void Console::__initMenu()
{
    vector<vector<MenuData>> menuDatas = {
        { { L" 文件(F)", L"新建、打开、保存文件，退出 (Alt+F)", nullptr },
            { L" 新建", L"新建一个棋局", nullptr },
            { L" 打开...", L"打开已有的一个棋局", nullptr },
            { L" 另存为...", L"将显示的棋局另存为一个棋局", nullptr },
            { L" 保存", L"保存正在显示的棋局", nullptr },
            { L" 退出", L"退出程序", nullptr },
            { L"", L"", nullptr } },
        { { L" 棋局(B)", L"对棋盘局面进行操作 (Alt+B)", nullptr },
            { L" 对换棋局", L"红黑棋子互换", nullptr },
            { L" 对换位置", L"红黑位置互换", nullptr },
            { L" 棋子左右换位", L"棋子的位置左右对称换位", nullptr },
            { L"", L"", nullptr } },
        { { L" 设置(S)", L"设置显示主题，主要是棋盘、棋子的颜色配置 (Alt+S)", nullptr },
            { L" 静雅朴素", L"比较朴素的颜色配置", nullptr },
            { L" 鲜艳亮丽", L"比较鲜艳的颜色配置", nullptr },
            { L" 高对比度", L"高对比度的颜色配置", nullptr },
            { L"", L"", nullptr } },
        { { L" 关于(A)", L"帮助、程序信息 (Alt+A)", nullptr },
            { L" 帮助", L"显示帮助信息", nullptr },
            { L" 版本信息", L"程序有关的信息", nullptr },
            { L"", L"", nullptr } }
    };

    // 生成一个菜单
    auto newMenu = [](const MenuData& menuData) {
        Menu* menu = new Menu();
        menu->name = menuData.name;
        menu->desc = menuData.desc;
        menu->func = menuData.func;
        return menu;
    };

    // 增加一个子菜单项
    auto addChildMenu = [](Menu* preMenu, Menu* childMenu) {
        childMenu->preMenu = preMenu;
        childMenu->brotherIndex = preMenu->brotherIndex;
        childMenu->childIndex = preMenu->childIndex + 1;
        return preMenu->childMenu = childMenu;
    };

    // 增加一个兄弟菜单项
    auto addBrotherMenu = [](Menu* preMenu, Menu* brotherMenu) {
        brotherMenu->preMenu = preMenu;
        brotherMenu->brotherIndex = preMenu->brotherIndex + 1;
        brotherMenu->childIndex = preMenu->childIndex;
        return preMenu->brotherMenu = brotherMenu;
    };

    rootMenu_ = newMenu((MenuData){ L"", L"", nullptr });
    Menu *brotherMenu = rootMenu_, *childMenu;
    int levelOneNum = menuDatas.size();
    for (int oneIndex = 0; oneIndex < levelOneNum; ++oneIndex) {
        brotherMenu = addBrotherMenu(brotherMenu, newMenu(menuDatas[oneIndex][0])); // 第一个为菜单组名称
        childMenu = brotherMenu;
        int levelTwoNum = menuDatas[oneIndex].size();
        for (int twoIndex = 1; twoIndex < levelTwoNum && !menuDatas[oneIndex][twoIndex].name.empty(); ++twoIndex)
            childMenu = addChildMenu(childMenu, newMenu(menuDatas[oneIndex][twoIndex]));
    }

    // 绘制菜单区域
    brotherMenu = rootMenu_;
    int width = 0;
    while ((brotherMenu = brotherMenu->brotherMenu)) {
        int namelen = brotherMenu->name.size();
        WriteConsoleOutputCharacterW(hOut_, brotherMenu->name.c_str(), namelen,
            { SHORT(width), MenuRect.Top }, &rwNum);
        width += namelen + 3;
    }
    curMenu_ = rootMenu_;
}

void Console::__initArea(WORD attr, const SMALL_RECT& rc, bool drawFrame)
{
    SMALL_RECT irc = { rc.Left, rc.Top, SHORT(rc.Right - SHADOWCOLS), SHORT(rc.Bottom - SHADOWROWS) };
    __cleanArea(attr, irc);
    __initAreaShadow(rc);

    if (drawFrame) {
        SHORT left = irc.Left + 1, right = irc.Right - 1;
        const wchar_t* const tabChar = TabChars[thema_];
        for (auto row : { irc.Top, irc.Bottom }) // 顶、底行
            FillConsoleOutputCharacterW(hOut_, tabChar[0], irc.Right - irc.Left - 2, { left, row }, &rwNum);
        for (SHORT row = irc.Top + 1; row < irc.Bottom; ++row)
            for (SHORT col : { irc.Left, right })
                FillConsoleOutputCharacterW(hOut_, tabChar[1], 1, { col, row }, &rwNum);
        map<wchar_t, COORD>
            wchCoords = {
                { tabChar[2], { irc.Left, irc.Top } },
                { tabChar[3], { right, irc.Top } },
                { tabChar[4], { irc.Left, irc.Bottom } },
                { tabChar[5], { right, irc.Bottom } },
            };
        for (const auto& wchCoord : wchCoords)
            FillConsoleOutputCharacterW(hOut_, wchCoord.first, 1, wchCoord.second, &rwNum);
    }
}

void Console::__initAreaShadow(const SMALL_RECT& rc)
{
    //FillConsoleOutputAttribute(hOut_, WINATTR[thema_], SHADOWCOLS, { rc.Left, rc.Bottom }, &rwNum);
    FillConsoleOutputAttribute(hOut_, SHADOWATTR[thema_], rc.Right - rc.Left + 1 - SHADOWCOLS, { SHORT(rc.Left + SHADOWCOLS), rc.Bottom }, &rwNum);
    SHORT right = rc.Right - SHADOWCOLS + 1;
    //if (rc.Bottom > rc.Top + 1)
    //    FillConsoleOutputAttribute(hOut_, WINATTR[thema_], SHADOWCOLS, { right, rc.Top }, &rwNum);
    for (SHORT row = rc.Top + (rc.Bottom > rc.Top + 1 ? 1 : 0); row <= rc.Bottom; ++row)
        FillConsoleOutputAttribute(hOut_, SHADOWATTR[thema_], 2, { right, row }, &rwNum);
}

void Console::__cleanAreaWIN()
{
    int colWidth = 2;
    SMALL_RECT rc = { WINRECT.Left, MenuRect.Bottom + 1, WINRECT.Right, StatusRect.Top - 1 };
    for (auto row : { rc.Top, rc.Bottom }) { // 顶、底行
        int width = rc.Right - rc.Left + 1;
        COORD pos = { rc.Left, row };
        FillConsoleOutputAttribute(hOut_, WINATTR[thema_], width, pos, &rwNum);
        FillConsoleOutputCharacterW(hOut_, L' ', width, pos, &rwNum);
    }
    for (SHORT row = rc.Top; row < rc.Bottom; ++row) // 左、右列
        for (SHORT col : { rc.Left, SHORT(rc.Right - colWidth + 1) }) {
            COORD pos = { col, row };
            FillConsoleOutputAttribute(hOut_, WINATTR[thema_], colWidth, pos, &rwNum);
            FillConsoleOutputCharacterW(hOut_, L' ', colWidth, pos, &rwNum);
        }
}

void Console::__cleanSubMenuArea()
{
    __cleanAreaAttr(MENUATTR[thema_], iMenuRect);
    FillConsoleOutputCharacterW(hOut_, L' ', WINCOLS, { MenuRect.Left, MenuRect.Bottom }, &rwNum);
    FillConsoleOutputAttribute(hOut_, WINATTR[thema_], SHADOWCOLS, { MenuRect.Left, MenuRect.Bottom }, &rwNum);
    __initAreaShadow(MenuRect);

    __cleanAreaWIN();
    __initArea(BOARDATTR[thema_], BoardRect);
    __writeBoard();
}

void Console::__cleanArea(WORD attr, const SMALL_RECT& rc)
{
    __cleanAreaChar(rc);
    __cleanAreaAttr(attr, rc);
}

void Console::__cleanAreaChar(const SMALL_RECT& rc)
{
    int cols{ rc.Right - rc.Left + 1 };
    for (SHORT row = rc.Top; row <= rc.Bottom; ++row)
        FillConsoleOutputCharacterW(hOut_, L' ', cols, COORD{ rc.Left, row }, &rwNum);
}

void Console::__cleanAreaAttr(WORD attr, const SMALL_RECT& rc)
{
    int cols{ rc.Right - rc.Left + 1 };
    for (SHORT row = rc.Top; row <= rc.Bottom; ++row)
        FillConsoleOutputAttribute(hOut_, attr, cols, COORD{ rc.Left, row }, &rwNum);
}

void writeCharBuf(CHAR_INFO* charBuf, COORD bufSize, COORD bufCoord, SMALL_RECT& writeRect)
{
    //for (auto& chInfo : statusCharBuf)
    //  chInfo = { L' ', STATUSATTR };
    //WriteConsoleOutputW(hOut_, statusCharBuf, statusBufSize, statusBufCoord, &iStatusRect);
    //SetConsoleTextAttribute();
}

void setCharBuf(CHAR_INFO* charBuf, COORD charCoord, const wchar_t* wchars, WORD attr)
{
    int bufIndex = 0, wchIndex = 0;
    for (int row = 0; row < charCoord.Y; ++row) {
        for (int col = 0; col < charCoord.X; ++col) {
            wchar_t wch = wchars[wchIndex++];
            charBuf[bufIndex++] = CHAR_INFO{ wch, attr };
            if (wch > 255) {
                ++col;
                ++bufIndex;
                //charBuf[bufIndex++] = CHAR_INFO{ L' ', attr };
            }
        }
        while (wchars[wchIndex++] != L'\n')
            ; // 递进到下一行
    }
}

/*
void DrawBox(bool bSingle, SMALL_RECT rc); // 函数功能：画边框

void ShadowWindowLine(char* str)
{
    SMALL_RECT rc;
    CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
    WORD att0, att1; //,attText;
    int i, chNum = strlen(str);
    GetConsoleScreenBufferInfo(hOut, &bInfo); // 获取窗口缓冲区信息
    // 计算显示窗口大小和位置
    rc.Left = (bInfo.dwSize.X - chNum) / 2 - 2;
    rc.Top = 10; // 原代码段中此处为bInfo.dwSize.Y/2 - 2，但是如果您的DOS屏幕有垂直滚动条的话，还需要把滚动条下拉才能看到,为了方便就把它改为10
    rc.Right = rc.Left + chNum + 3;
    rc.Bottom = rc.Top + 4;
    att0 = BACKGROUND_INTENSITY; // 阴影属性
    att1 = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY | BACKGROUND_RED | BACKGROUND_BLUE; // 文本属性
    //attText = FOREGROUND_RED |FOREGROUND_INTENSITY; // 文本属性
    // 设置阴影然后填充
    COORD posShadow = { SHORT(rc.Left + 1), SHORT(rc.Top + 1) }, posText = { rc.Left, rc.Top };
    DWORD rwNum;
    for (i = 0; i < 5; i++) {
        FillConsoleOutputAttribute(hOut, att0, chNum + 4, posShadow, &rwNum);
        posShadow.Y++;
    }
    for (i = 0; i < 5; i++) {
        FillConsoleOutputAttribute(hOut, att1, chNum + 4, posText, &rwNum);
        posText.Y++;
    }
    // 写文本和边框
    posText.X = rc.Left + 2;
    posText.Y = rc.Top + 2;
    WriteConsoleOutputCharacterA(hOut, str, strlen(str), posText, &rwNum);
    //DrawBox(true, rc);
    DrawBox(false, rc);
    SetConsoleTextAttribute(hOut, bInfo.wAttributes); // 恢复原来的属性
}

void DrawBox(bool bSingle, SMALL_RECT rc) // 函数功能：画边框
{
    char chBox[6];
    COORD pos;
    if (bSingle) {
        chBox[0] = (char)0xda; // 左上角点
        chBox[1] = (char)0xbf; // 右上角点
        chBox[2] = (char)0xc0; // 左下角点
        chBox[3] = (char)0xd9; // 右下角点
        chBox[4] = (char)0xc4; // 水平
        chBox[5] = (char)0xb3; // 坚直
    } else {
        chBox[0] = (char)0xc9; // 左上角点
        chBox[1] = (char)0xbb; // 右上角点
        chBox[2] = (char)0xc8; // 左下角点
        chBox[3] = (char)0xbc; // 右下角点
        chBox[4] = (char)0xcd; // 水平
        chBox[5] = (char)0xba; // 坚直
    }
    DWORD rwNum;
    // 画边框的上 下边界
    for (pos.X = rc.Left + 1; pos.X < rc.Right - 1; pos.X++) {
        pos.Y = rc.Top;
        // 画上边界
        WriteConsoleOutputCharacterA(hOut, &chBox[4], 1, pos, &rwNum);
        // 画左上角
        if (pos.X == rc.Left + 1) {
            pos.X--;
            WriteConsoleOutputCharacterA(hOut, &chBox[0], 1, pos, &rwNum);
            pos.X++;
        }
        // 画右上角
        if (pos.X == rc.Right - 2) {
            pos.X++;
            WriteConsoleOutputCharacterA(hOut, &chBox[1], 1, pos, &rwNum);
            pos.X--;
        }
        pos.Y = rc.Bottom;
        // 画下边界
        WriteConsoleOutputCharacterA(hOut, &chBox[4], 1, pos, &rwNum);
        // 画左下角
        if (pos.X == rc.Left + 1) {
            pos.X--;
            WriteConsoleOutputCharacterA(hOut, &chBox[2], 1, pos, &rwNum);
            pos.X++;
        }
        // 画右下角
        if (pos.X == rc.Right - 2) {
            pos.X++;
            WriteConsoleOutputCharacterA(hOut, &chBox[3], 1, pos, &rwNum);
            pos.X--;
        }
    }
    // 画边框的左右边界
    for (pos.Y = rc.Top + 1; pos.Y <= rc.Bottom - 1; pos.Y++) {
        pos.X = rc.Left;
        // 画左边界
        WriteConsoleOutputCharacterA(hOut, &chBox[5], 1, pos, &rwNum);
        pos.X = rc.Right - 1;
        // 画右边界
        WriteConsoleOutputCharacterA(hOut, &chBox[5], 1, pos, &rwNum);
    }
}

void DeleteLine(int row); // 删除一行
void MoveText(int x, int y, SMALL_RECT rc); // 移动文本块区域
void ClearScreen(void); // 清屏

void DeleteLine(int row)
{
    SMALL_RECT rcScroll, rcClip;
    COORD crDest = { 0, SHORT(row - 1) };
    CHAR_INFO chFill;
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    rcScroll.Left = 0;
    rcScroll.Top = row;
    rcScroll.Right = bInfo.dwSize.X - 1;
    rcScroll.Bottom = bInfo.dwSize.Y - 1;
    rcClip = rcScroll;
    chFill.Attributes = bInfo.wAttributes;
    chFill.Char.AsciiChar = ' ';
    ScrollConsoleScreenBuffer(hOut, &rcScroll, &rcClip, crDest, &chFill);
}

void MoveText(int x, int y, SMALL_RECT rc)
{
    COORD crDest = { SHORT(x), SHORT(y) };
    CHAR_INFO chFill;
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    chFill.Attributes = bInfo.wAttributes;
    chFill.Char.AsciiChar = ' ';
    SMALL_RECT rcClip;
    ScrollConsoleScreenBuffer(hOut, &rc, &rcClip, crDest, &chFill);
}

void ClearScreen(void)
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    COORD home = { 0, 0 };
    WORD att = bInfo.wAttributes;
    unsigned long size = bInfo.dwSize.X * bInfo.dwSize.Y;
    FillConsoleOutputAttribute(hOut, att, size, home, nullptr);
    FillConsoleOutputCharacter(hOut, ' ', size, home, nullptr);
}

//HANDLE hIn;
void CharWindow(char ch, SMALL_RECT rc); // 将ch输入到指定的窗口中
void ControlStatus(DWORD state); // 在最后一行显示控制键的状态
void DeleteTopLine(SMALL_RECT rc); // 删除指定窗口中最上面的行并滚动

void CharWindow(char ch, SMALL_RECT rc) // 将ch输入到指定的窗口中
{
    static COORD chPos = { SHORT(rc.Left + 1), SHORT(rc.Top + 1) };
    SetConsoleCursorPosition(hOut, chPos); // 设置光标位置
    if ((ch < 0x20) || (ch > 0x7e)) // 如果是不可打印的字符，具体查看ASCII码表
        return;
    WriteConsoleOutputCharacter(hOut, &ch, 1, chPos, nullptr);
    if (chPos.X >= (rc.Right - 2)) {
        chPos.X = rc.Left;
        chPos.Y++;
    }
    if (chPos.Y > (rc.Bottom - 1)) {
        DeleteTopLine(rc);
        chPos.Y = rc.Bottom - 1;
    }
    chPos.X++;
    SetConsoleCursorPosition(hOut, chPos); // 设置光标位置
}

void ControlStatus(DWORD state) // 在第一行显示控制键的状态
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    COORD home = { 0, 24 }; // 原来此处为bInfo.dwSize.Y-1，但为了更便于观察，我把这里稍微修改了一下
    WORD att0 = BACKGROUND_INTENSITY;
    WORD att1 = FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_RED;
    FillConsoleOutputAttribute(hOut, att0, bInfo.dwSize.X, home, nullptr);
    FillConsoleOutputCharacter(hOut, ' ', bInfo.dwSize.X, home, nullptr);
    SetConsoleTextAttribute(hOut, att1);
    COORD staPos = { SHORT(bInfo.dwSize.X - 16), 24 }; // 原来此处为bInfo.dwSize.Y-1
    SetConsoleCursorPosition(hOut, staPos);
    if (state & NUMLOCK_ON)
        WriteConsole(hOut, "NUM", 3, nullptr, nullptr);
    staPos.X += 4;
    SetConsoleCursorPosition(hOut, staPos);
    if (state & CAPSLOCK_ON)
        WriteConsole(hOut, "CAPS", 4, nullptr, nullptr);
    staPos.X += 5;
    SetConsoleCursorPosition(hOut, staPos);
    if (state & SCROLLLOCK_ON)
        WriteConsole(hOut, "SCROLL", 6, nullptr, nullptr);
    SetConsoleTextAttribute(hOut, bInfo.wAttributes); // 恢复原来的属性
    SetConsoleCursorPosition(hOut, bInfo.dwCursorPosition); // 恢复原来的光标位置
}

void DeleteTopLine(SMALL_RECT rc)
{
    COORD crDest;
    CHAR_INFO chFill;
    SMALL_RECT rcClip = rc;
    rcClip.Left++;
    rcClip.Right -= 2;
    rcClip.Top++;
    rcClip.Bottom--;
    crDest.X = rcClip.Left;
    crDest.Y = rcClip.Top - 1;
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    chFill.Attributes = bInfo.wAttributes;
    chFill.Char.AsciiChar = ' ';
    ScrollConsoleScreenBuffer(hOut, &rcClip, &rcClip, crDest, &chFill);
}

void DispMousePos(COORD pos) // 在第24行显示鼠标位置
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo;
    GetConsoleScreenBufferInfo(hOut, &bInfo);
    COORD home = { 0, 24 };
    WORD att0 = BACKGROUND_INTENSITY;
    FillConsoleOutputAttribute(hOut, att0, bInfo.dwSize.X, home, nullptr);
    FillConsoleOutputCharacter(hOut, ' ', bInfo.dwSize.X, home, nullptr);
    char s[20];
    sprintf(s, "X = %2d, Y = %2d", pos.X, pos.Y);
    SetConsoleTextAttribute(hOut, att0);
    SetConsoleCursorPosition(hOut, home);
    WriteConsole(hOut, s, strlen(s), nullptr, nullptr);
    SetConsoleTextAttribute(hOut, bInfo.wAttributes); // 恢复原来的属性
    SetConsoleCursorPosition(hOut, bInfo.dwCursorPosition); // 恢复原来的光标位置
}

HANDLE hStdout;
int readAndWirte(void)
{
    HANDLE hStdout, hNewScreenBuffer;
    SMALL_RECT srctReadRect;
    SMALL_RECT srctWriteRect;
    CHAR_INFO chiBuffer[160]; // [2][80];
    COORD coordBufSize;
    COORD coordBufCoord;
    BOOL fSuccess;

    // Get a handle to the STDOUT screen buffer to copy from and
    // create a new screen buffer to copy to.

    hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    hNewScreenBuffer = CreateConsoleScreenBuffer(
        GENERIC_READ | // read/write access
            GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE, // shared
        nullptr, // default security attributes
        CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
        nullptr); // reserved; must be nullptr
    if (hStdout == INVALID_HANDLE_VALUE || hNewScreenBuffer == INVALID_HANDLE_VALUE) {
        printf("CreateConsoleScreenBuffer failed - (%d)\n", GetLastError());
        return 1;
    }

    // Make the new screen buffer the active screen buffer.

    if (!SetConsoleActiveScreenBuffer(hNewScreenBuffer)) {
        printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        return 1;
    }

    // Set the source rectangle.

    srctReadRect.Top = 0; // top left: row 0, col 0
    srctReadRect.Left = 0;
    srctReadRect.Bottom = 1; // bot. right: row 1, col 79
    srctReadRect.Right = 79;

    // The temporary buffer size is 2 rows x 80 columns.

    coordBufSize.Y = 2;
    coordBufSize.X = 80;

    // The top left destination cell of the temporary buffer is
    // row 0, col 0.

    coordBufCoord.X = 0;
    coordBufCoord.Y = 0;

    // Copy the block from the screen buffer to the temp. buffer.

    fSuccess = ReadConsoleOutput(
        hStdout, // screen buffer to read from
        chiBuffer, // buffer to copy into
        coordBufSize, // col-row size of chiBuffer
        coordBufCoord, // top left dest. cell in chiBuffer
        &srctReadRect); // screen buffer source rectangle
    if (!fSuccess) {
        printf("ReadConsoleOutput failed - (%d)\n", GetLastError());
        return 1;
    }

    // Set the destination rectangle.

    srctWriteRect.Top = 10; // top lt: row 10, col 0
    srctWriteRect.Left = 0;
    srctWriteRect.Bottom = 11; // bot. rt: row 11, col 79
    srctWriteRect.Right = 79;

    // Copy from the temporary buffer to the new screen buffer.

    fSuccess = WriteConsoleOutput(
        hNewScreenBuffer, // screen buffer to write to
        chiBuffer, // buffer to copy from
        coordBufSize, // col-row size of chiBuffer
        coordBufCoord, // top left src cell in chiBuffer
        &srctWriteRect); // dest. screen buffer rectangle
    if (!fSuccess) {
        printf("WriteConsoleOutput failed - (%d)\n", GetLastError());
        return 1;
    }
    Sleep(5000);

    // Restore the original active screen buffer.

    if (!SetConsoleActiveScreenBuffer(hStdout)) {
        printf("SetConsoleActiveScreenBuffer failed - (%d)\n", GetLastError());
        return 1;
    }

    return 0;
}

void cls(HANDLE hConsole)
{
    COORD coordScreen = { 0, 0 }; // home for the cursor
    DWORD cCharsWritten;
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    DWORD dwConSize;

    // Get the number of character cells in the current buffer.

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    dwConSize = csbi.dwSize.X * csbi.dwSize.Y;

    // Fill the entire screen with blanks.

    if (!FillConsoleOutputCharacter(hConsole, // Handle to console screen buffer
            (TCHAR)' ', // Character to write to the buffer
            dwConSize, // Number of cells to write
            coordScreen, // Coordinates of first cell
            &cCharsWritten)) // Receive number of characters rwNum
    {
        return;
    }

    // Get the current text attribute.

    if (!GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        return;
    }

    // Set the buffer's attributes accordingly.

    if (!FillConsoleOutputAttribute(hConsole, // Handle to console screen buffer
            csbi.wAttributes, // Character attributes to use
            dwConSize, // Number of cells to set attribute
            coordScreen, // Coordinates of first cell
            &cCharsWritten)) // Receive number of characters rwNum
    {
        return;
    }

    // Put the cursor at its home coordinates.

    SetConsoleCursorPosition(hConsole, coordScreen);
}

int ScrollByAbsoluteCoord(int iRows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    SMALL_RECT srctWindow;

    // Get the current screen buffer size and window position.

    if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) {
        printf("GetConsoleScreenBufferInfo (%d)\n", GetLastError());
        return 0;
    }

    // Set srctWindow to the current window size and location.

    srctWindow = csbiInfo.srWindow;

    // Check whether the window is too close to the screen buffer top

    if (srctWindow.Top >= iRows) {
        srctWindow.Top -= (SHORT)iRows; // move top up
        srctWindow.Bottom -= (SHORT)iRows; // move bottom up

        if (!SetConsoleWindowInfo(
                hStdout, // screen buffer handle
                TRUE, // absolute coordinates
                &srctWindow)) // specifies new location
        {
            printf("SetConsoleWindowInfo (%d)\n", GetLastError());
            return 0;
        }
        return iRows;
    } else {
        printf("\nCannot scroll; the window is too close to the top.\n");
        return 0;
    }
}

int ScrollByRelativeCoord(int iRows)
{
    CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
    SMALL_RECT srctWindow;

    // Get the current screen buffer window position.

    if (!GetConsoleScreenBufferInfo(hStdout, &csbiInfo)) {
        printf("GetConsoleScreenBufferInfo (%d)\n", GetLastError());
        return 0;
    }

    // Check whether the window is too close to the screen buffer top

    if (csbiInfo.srWindow.Top >= iRows) {
        srctWindow.Top = -(SHORT)iRows; // move top up
        srctWindow.Bottom = -(SHORT)iRows; // move bottom up
        srctWindow.Left = 0; // no change
        srctWindow.Right = 0; // no change

        if (!SetConsoleWindowInfo(
                hStdout, // screen buffer handle
                FALSE, // relative coordinates
                &srctWindow)) // specifies new location
        {
            printf("SetConsoleWindowInfo (%d)\n", GetLastError());
            return 0;
        }
        return iRows;
    } else {
        printf("\nCannot scroll; the window is too close to the top.\n");
        return 0;
    }
}
//*/

/*
void view0()
{
    CONSOLE_SCREEN_BUFFER_INFO bInfo; // 存储窗口信息
    COORD pos = {0, 0};
    // 获取标准输出设备句柄
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); 
    // 获取窗口信息
    GetConsoleScreenBufferInfo(hOut, &bInfo ); 
    printf("\n\nThe soul selects her own society\n");
    printf("Then shuts the door\n");
    printf("On her devine majority\n");
    printf("Obtrude no more\n\n");
    _getch();
    // 向窗口中填充字符以获得清屏的效果
    FillConsoleOutputCharacter(hOut,'-', bInfo.dwSize.X * bInfo.dwSize.Y, pos, nullptr);
    // 关闭标准输出设备句柄
    CloseHandle(hOut); 
    //*/

/*
    setlocale(LC_ALL, "C");
    char strTitle[255];
    CONSOLE_SCREEN_BUFFER_INFO bInfo; // 窗口缓冲区信息
    COORD size = { 80, 25 };
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
    GetConsoleScreenBufferInfo(hOut, &bInfo); // 获取窗口缓冲区信息
    GetConsoleTitle(strTitle, 255); // 获取窗口标题
    printf("当前窗口标题是：%s\n", strTitle);
    _getch();
    SetConsoleTitle("控制台窗口操作"); // 设置窗口标题
    GetConsoleTitle(strTitle, 255);
    printf("当前窗口标题是：%s\n", strTitle);
    _getch();
    SetConsoleScreenBufferSize(hOut, size); // 重新设置缓冲区大小
    _getch();
    SMALL_RECT rc = { 0, 0, 80 - 1, 25 - 1 }; // 重置窗口位置和大小 (窗口边框会消失？)
    SetConsoleWindowInfo(hOut, true, &rc);
    setlocale(LC_ALL, "chs");

    CloseHandle(hOut); // 关闭标准输出设备句柄
    //*/

/*
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
    SetConsoleOutputCP(437); // 设置代码页，这里如果设置成936（简体中文），那么程序会怎样？那样的话，将画不出边框。
    //SetConsoleOutputCP(936); // 设置代码页，这里如果设置成936（简体中文），那么程序会怎样？那样的话，将画不出边框。
    char str[] = "Display a line of words, and center the window with shadow.";
    ShadowWindowLine(str);
    CloseHandle(hOut); // 关闭标准输出设备句柄
    //*/

/*
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
    WORD att = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE;// 背景是蓝色，文本颜色是黄色
    SetConsoleTextAttribute(hOut, att);
    ClearScreen();
    printf("\n\nThe soul selects her own society\n");
    printf("Then shuts the door;\n");
    printf("On her devine majority;\n");
    printf("Obtrude no more.\n\n");
    COORD endPos = {0, 15};
    SetConsoleCursorPosition(hOut, endPos); // 设置光标位置
    SMALL_RECT rc = {0, 2, 40, 5};
    _getch();
    MoveText(10, 5, rc);
    _getch();
    DeleteLine(5);
    CloseHandle(hOut); // 关闭标准输出设备句柄
    //*/

/*
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
    hIn = GetStdHandle(STD_INPUT_HANDLE); // 获取标准输入设备句柄
    WORD att = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE ; // 背景是蓝色，文本颜色是黄色
    SetConsoleTextAttribute(hOut, att);
    ClearScreen(); // 清屏
    INPUT_RECORD keyRec;
    DWORD state = 0, res;
    char ch;
    SMALL_RECT rc = {20, 2, 40, 12};
    DrawBox(false, rc);
    COORD pos = {rc.Left+1, rc.Top+1};
    SetConsoleCursorPosition(hOut, pos); // 设置光标位置
    for(;;) // 循环
    {
        ReadConsoleInput(hIn, &keyRec, 1, &res);
        if (state != keyRec.Event.KeyEvent.dwControlKeyState)
        {
            state = keyRec.Event.KeyEvent.dwControlKeyState;
            ControlStatus(state);
        }
        if (keyRec.EventType == KEY_EVENT)
        {
            if (keyRec.Event.KeyEvent.wVirtualKeyCode == VK_ESCAPE) 
                break;
            // 按ESC键退出循环
            if (keyRec.Event.KeyEvent.bKeyDown)
            {
                ch = keyRec.Event.KeyEvent.uChar.AsciiChar;
                CharWindow(ch, rc);
            }
        }
    }
    pos.X = 0; pos.Y = 0;
    SetConsoleCursorPosition(hOut, pos); // 设置光标位置
    CloseHandle(hOut); // 关闭标准输出设备句柄
    CloseHandle(hIn); // 关闭标准输入设备句柄
    //*/

/*
    hOut = GetStdHandle(STD_OUTPUT_HANDLE); // 获取标准输出设备句柄
    hIn = GetStdHandle(STD_INPUT_HANDLE); // 获取标准输入设备句柄
    WORD att = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY | BACKGROUND_BLUE ;
    // 背景是蓝色，文本颜色是黄色
    SetConsoleTextAttribute(hOut, att);
    ClearScreen(); // 清屏
    INPUT_RECORD mouseRec;
    DWORD state = 0, res;
    COORD pos = {0, 0};
    for(;;) // 循环
    {
        ReadConsoleInput(hIn, &mouseRec, 1, &res);
        if (mouseRec.EventType == MOUSE_EVENT)
        {
            if (mouseRec.Event.MouseEvent.dwEventFlags == DOUBLE_CLICK) 
                break; // 双击鼠标退出循环		
            pos = mouseRec.Event.MouseEvent.dwMousePosition;
            DispMousePos(pos);
            if (mouseRec.Event.MouseEvent.dwButtonState == FROM_LEFT_1ST_BUTTON_PRESSED)
                FillConsoleOutputCharacter(hOut, 'A', 1, pos, nullptr); 
        }
    } 
    pos.X = pos.Y = 0;
    SetConsoleCursorPosition(hOut, pos); // 设置光标位置
    CloseHandle(hOut); // 关闭标准输出设备句柄
    CloseHandle(hIn); // 关闭标准输入设备句柄
}
    //*/
}