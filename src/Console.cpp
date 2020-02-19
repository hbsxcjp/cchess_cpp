#include "Console.h"
#include "ChessManual.h"
#include "Piece.h"

namespace ConsoleSpace {

static DWORD written; // 公用变量
static constexpr wchar_t PROGRAMNAME[] = L"中国象棋 ";

static constexpr SHORT WINROWS = 45, WINCOLS = 128;
static constexpr SHORT BOARDROWS = 10 + 9, BOARDCOLS = (9 + 8) * 2;
static constexpr SHORT STATUSROWS = 2;
static constexpr COORD HOMEPOS = { 0, 0 };

static constexpr SMALL_RECT WinRect = { 0, 0, SHORT(WINCOLS - 1), SHORT(WINROWS - 1) };
static constexpr SMALL_RECT MenuRect = { WinRect.Left, WinRect.Top, WinRect.Right, 0 };
static constexpr SMALL_RECT StatusRect = { WinRect.Left, SHORT(WinRect.Bottom - STATUSROWS + 1), WinRect.Right, WinRect.Bottom };

static constexpr SMALL_RECT BoardRect = { WinRect.Left + 2, SHORT(MenuRect.Bottom + 2), // 阴影需要1行、2列
    SHORT(WinRect.Left + BOARDCOLS + 2 * 2 + 2), SHORT(MenuRect.Bottom + BOARDROWS + 2 * 2 + 2) }; // 阴影需要1行、2列
static constexpr SMALL_RECT iBoardRect = { SHORT(BoardRect.Left + 2), SHORT(BoardRect.Top + 1), SHORT(BoardRect.Right - 4), SHORT(BoardRect.Bottom - 1) }; // 填充字符区域

static constexpr SMALL_RECT CurmoveRect = { BoardRect.Left, SHORT(BoardRect.Bottom + 2), BoardRect.Right, SHORT(StatusRect.Top - 2) };
static constexpr SMALL_RECT iCurmoveRect = { SHORT(CurmoveRect.Left + 2), SHORT(CurmoveRect.Top + 1), SHORT(CurmoveRect.Right - 4), SHORT(CurmoveRect.Bottom - 1) }; // 填充字符区域

static constexpr SMALL_RECT MoveRect = { SHORT(BoardRect.Right + 3), BoardRect.Top, SHORT(WinRect.Right - 2), CurmoveRect.Bottom }; // 阴影需要1行、2列
static constexpr SMALL_RECT iMoveRect = { SHORT(MoveRect.Left + 2), SHORT(MoveRect.Top + 1), SHORT(MoveRect.Right - 4), SHORT(MoveRect.Bottom - 1) }; // 填充字符区域

/*
颜色属性由两个十六进制数字指定 -- 第一个对应于背景，第二个对应于前景。每个数字可以为以下任何值:
    0 = 黑色       8 = 灰色
    1 = 蓝色       9 = 淡蓝色
    2 = 绿色       A = 淡绿色
    3 = 浅绿色     B = 淡浅绿色
    4 = 红色       C = 淡红色
    5 = 紫色       D = 淡紫色
    6 = 黄色       E = 淡黄色
    7 = 白色       F = 亮白色
//*/
static constexpr WORD WINATTR[] = { 0x07, 0x77 };
static constexpr WORD MENUATTR[] = { 0x80, 0x4F };
static constexpr WORD BOARDATTR[] = { 0xF8, 0xE2 };
static constexpr WORD CURMOVEATTR[] = { 0x70, 0xB5 };
static constexpr WORD MOVEATTR[] = { 0x70, 0x35 };
static constexpr WORD STATUSATTR[] = { 0xF0, 0x51 };
static constexpr WORD SHADOWATTR[] = { 0x08, 0x82 };

static constexpr WORD RedSideAttr[] = { 0x0C | (BOARDATTR[0] & 0xF0), 0x0C | (BOARDATTR[1] & 0xF0) };
static constexpr WORD BlackSideAttr[] = { 0x00 | (BOARDATTR[0] & 0xF0), 0x00 | (BOARDATTR[1] & 0xF0) };
static constexpr WORD RedAttr[] = { 0xCF, 0xCF };
static constexpr WORD BlackAttr[] = { 0x0F, 0x0F };
static constexpr WORD SelRedAttr[] = { 0xFC, 0xC0 };
static constexpr WORD SelBlackAttr[] = { 0xF0, 0xE0 };

/*/ 填充字符信息、矩形区域
static constexpr int menuHeight = MenuRect.Bottom - MenuRect.Top + 1, menuWidth = MenuRect.Right - MenuRect.Left + 1;
static CHAR_INFO menuCharBuf[menuHeight * menuWidth];

static constexpr int boardHeight = BOARDROWS - 2, boardWidth = BOARDCOLS - 6;
static CHAR_INFO boardCharBuf[boardHeight * boardWidth];
static SMALL_RECT iBoardRect = { SHORT(BoardRect.Left + 3), SHORT(BoardRect.Top + 1), SHORT(boardWidth), SHORT(boardHeight) };

static constexpr int statusHeight = StatusRect.Bottom - StatusRect.Top + 1, statusWidth = StatusRect.Right - StatusRect.Left + 1;
static CHAR_INFO statusCharBuf[statusHeight * statusWidth];
static COORD statusBufSize = { statusWidth, statusHeight };
static COORD statusBufCoord = { StatusRect.Left, StatusRect.Top };
static SMALL_RECT iStatusRect = { StatusRect.Left, StatusRect.Top, StatusRect.Right, StatusRect.Bottom };

static constexpr int curmoveHeight = CurmoveRect.Bottom - CurmoveRect.Top + 1, curmoveWidth = CurmoveRect.Right - CurmoveRect.Left + 1;
static CHAR_INFO curmoveCharBuf[curmoveHeight * curmoveWidth];
static SMALL_RECT iCurmoveRect = { SHORT(curmoveWidth), SHORT(curmoveHeight) };

static constexpr int moveHeight = MoveRect.Bottom - MoveRect.Top + 1, moveWidth = MoveRect.Right - MoveRect.Left + 1;
static CHAR_INFO moveCharBuf[moveHeight * moveWidth];
static SMALL_RECT iMoveRect = { SHORT(moveWidth), SHORT(moveHeight) };
//*/

Console::Console(const string& fileName)
    : rootMenu_{ nullptr }
    , hIn_{ GetStdHandle(STD_INPUT_HANDLE) }
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
    //SetConsoleMode(hIn_, ENABLE_PROCESSED_INPUT);
    SetConsoleTitleW(PROGRAMNAME); // 设置窗口标题

    SetConsoleScreenBufferSize(hOut_, { WINCOLS, WINROWS });
    CONSOLE_CURSOR_INFO cInfo = { 5, false };
    SetConsoleCursorInfo(hOut_, &cInfo);
    SetConsoleWindowInfo(hOut_, true, &WinRect);
    SetConsoleActiveScreenBuffer(hOut_);
    //SetConsoleTextAttribute(hOut_, WINATTR[attrIndex]);
    //GetConsoleScreenBufferInfo(hOut_, &bInfo); // 获取窗口信息

    FillConsoleOutputAttribute(hOut_, WINATTR[attrIndex], WINROWS * WINCOLS, HOMEPOS, &written);
    cleanArea(hOut_, MENUATTR[attrIndex], MenuRect);
    cleanArea(hOut_, STATUSATTR[attrIndex], StatusRect);
    map<WORD, SMALL_RECT> rectAttrs = {
        { BOARDATTR[attrIndex], BoardRect },
        { CURMOVEATTR[attrIndex], CurmoveRect },
        { MOVEATTR[attrIndex], MoveRect }
    };
    for (const auto& rectAttr : rectAttrs)
        drawArea(hOut_, rectAttr.first, SHADOWATTR[attrIndex], rectAttr.second);

    __initMenu();
    __writeAreas();
}

void Console::open(const string& fileName)
{
    cm_ = make_shared<ChessManual>(fileName);
    __writeAreas();
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

void Console::__writeAreas()
{
    __writeBoard();
    writeAreaWstr(hOut_, cm_->getMoveStr(), 0, 0, iMoveRect);

    _getch();
}

void Console::__writeBoard()
{
    bool bottomIsRed{ cm_->isBottomSide(PieceColor::RED) };
    int rows = BoardRect.Bottom - BoardRect.Top + 1; // 减：上下边框、阴影
    WORD bottomAttr{ bottomIsRed ? RedSideAttr[attrIndex] : BlackSideAttr[attrIndex] },
        topAttr{ bottomIsRed ? BlackSideAttr[attrIndex] : RedSideAttr[attrIndex] };
    // 顶、底两行上颜色
    for (auto row : { 0, 1, rows - 2, rows - 1 })
        FillConsoleOutputAttribute(hOut_, (row > 1 ? bottomAttr : topAttr), BOARDCOLS, { iBoardRect.Left, SHORT(iBoardRect.Top + row) }, &written);
    // 棋子文字上颜色
    const wstring pieceChars{ cm_->getPieceChars() };
    for (int i = 0; i < SEATNUM; ++i) {
        wchar_t ch = pieceChars[i];
        if (ch == PieceManager::nullChar())
            continue;
        // 字符属性函数不识别全角字符，均按半角字符计数
        FillConsoleOutputAttribute(hOut_, (PieceManager::getColor(ch) == PieceColor::RED ? RedAttr[attrIndex] : BlackAttr[attrIndex]),
            2, { SHORT(iBoardRect.Left + (i % BOARDCOLNUM) * 4), SHORT(iBoardRect.Top + rows - 3 - (i / BOARDCOLNUM) * 2) }, &written);
    }

    static wchar_t showWstr[1024];
    getShowWstr(showWstr, cm_->getBoardStr().c_str());
    writeAreaWstr(hOut_, wstring(showWstr), 0, 0, iBoardRect);
}

void Console::__initMenu()
{
    vector<vector<MenuData>> menuDatas = {
        { { L"文件(F)", L"新建、打开、保存文件，退出 (Alt+F)", nullptr },
            { L"新建", L"新建一个棋局", nullptr },
            { L"打开...", L"打开已有的一个棋局", nullptr },
            { L"另存为...", L"将显示的棋局另存为一个棋局", nullptr },
            { L"保存", L"保存正在显示的棋局", nullptr },
            { L"退出", L"退出程序", nullptr },
            { L"", L"", nullptr } },
        { { L"棋局(B)", L"对棋盘局面进行操作 (Alt+B)", nullptr },
            { L"对换棋局", L"红黑棋子互换", nullptr },
            { L"对换位置", L"红黑位置互换", nullptr },
            { L"棋子左右换位", L"棋子的位置左右对称换位", nullptr },
            { L"", L"", nullptr } },
        { { L"设置(S)", L"设置显示主题，主要是棋盘、棋子的颜色配置 (Alt+S)", nullptr },
            { L"静雅朴素", L"比较朴素的颜色配置", nullptr },
            { L"鲜艳亮丽", L"比较鲜艳的颜色配置", nullptr },
            { L"高对比度", L"高对比度的颜色配置", nullptr },
            { L"", L"", nullptr } },
        { { L"关于(A)", L"帮助、程序信息 (Alt+A)", nullptr },
            { L"帮助", L"显示帮助信息", nullptr },
            { L"版本信息", L"程序有关的信息", nullptr },
            { L"", L"", nullptr } }
    };

    // 生成一个菜单
    auto newMenu = [](const MenuData& menuData) {
        Menu* menu = new Menu;
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
    int width = 1;
    while ((brotherMenu = brotherMenu->brotherMenu)) { // 赋值且判断是否为空
        int namelen = brotherMenu->name.size();
        WriteConsoleOutputCharacterW(hOut_, brotherMenu->name.c_str(), namelen,
            { SHORT(width), MenuRect.Top }, &written);
        width += namelen + 4;
    }
}

void Console::__writeSubMenu(Menu* menu, int rightSpaceNum)
{
    function<int(Menu*)> __getMaxWidth = [](Menu* menu) {
        menu = getTopMenu(menu);
        int maxWidth = 0;
        while (menu != nullptr) {
            int namelen = menu->name.size();
            if (maxWidth < namelen)
                maxWidth = namelen;
            menu = menu->brotherMenu;
        }
        return maxWidth;
    };

    function<wstring(Menu*, int)> __getWstr = [](Menu* menu, int maxWidth) {
        wstring wstr{};
        menu = getTopMenu(menu);
        while (menu != nullptr) {
            wstr += menu->name + wstring(maxWidth - menu->name.size(), L' ') + L'\n';
            menu = menu->brotherMenu;
        }
        return wstr;
    };

    function<int(Menu*)> __getPosL = [&](Menu* menu) {
        int width = __getMaxWidth(menu) + rightSpaceNum;
        while (menu->childIndex != 0) {
            menu = menu->preMenu; // 取得该级顶层菜单
            width += __getMaxWidth(menu) + rightSpaceNum;
        }
        while (menu->brotherIndex != 0) {
            width += menu->name.size() + rightSpaceNum;
            menu = menu->preMenu; // 顶层菜单往左推进
        }
        return width + 1; // 最左菜单前留一空白
    };

    Menu* cmenu;
    if (menu == nullptr || (cmenu = menu->childMenu) == nullptr)
        return;
    int maxWidth = __getMaxWidth(menu);
    SHORT posL = __getPosL(menu), posT = MenuRect.Bottom + menu->childMenu->childIndex,
          posR = posL + maxWidth + rightSpaceNum, posB = posT + getBottomMenu(cmenu)->brotherIndex - cmenu->brotherIndex;
    SMALL_RECT rect = { posL, posT, posR, posB };
    cleanArea(hOut_, MENUATTR[attrIndex], rect);
    writeAreaWstr(hOut_, __getWstr(menu, maxWidth), 0, 0, rect);
}

Menu* getTopMenu(Menu* menu)
{
    if (menu->brotherIndex == 1) // 菜单顶层的brotherIndex==1
        return menu;
    int childIndex = menu->childIndex;
    while (menu->brotherIndex > 1 && menu->childIndex == childIndex)
        menu = menu->preMenu;
    return menu->childMenu;
}

Menu* getBottomMenu(Menu* menu, int row)
{
    int index = 0;
    while (index++ < row && menu->brotherMenu != nullptr)
        menu = menu->brotherMenu;
    return menu;
}

Menu* getSameRowMenu(Menu* menu, bool isRight)
{
    if (isRight && menu->childMenu) // 向右且有子菜单
        return menu->childMenu;
    else if (!isRight && menu->preMenu->childIndex < menu->childIndex) // 向左且前项为父菜单
        return menu->preMenu;
    Menu* tmenu = getTopMenu(menu);
    while (tmenu->childIndex != 0)
        tmenu = getTopMenu(tmenu->preMenu); // 取得顶层菜单
    if ((isRight && !tmenu->brotherMenu) // 向右且顶层没有兄弟菜单
        || (!isRight && tmenu->preMenu->brotherIndex == 0)) // 向左且前项菜单为根菜单
        return menu;
    return getBottomMenu(isRight ? tmenu->brotherMenu : tmenu->preMenu,
        menu->brotherIndex - tmenu->brotherIndex + 1);
}

void writeAreaWstr(HANDLE hOut, const wstring& wstr, int firstCol, int firstRow, const SMALL_RECT& rc)
{
    wstringstream wss(wstr);
    wstring lineStr{};
    while (firstRow-- > 0)
        getline(wss, lineStr); // 去掉开始数行

    int width = rc.Right - rc.Left;
    for (SHORT row = rc.Top; row <= rc.Bottom; ++row) {
        if (!getline(wss, lineStr))
            break;
        int size = lineStr.size();
        if (size <= firstCol) // 小于起始列退出
            break;
        if (size < width)
            lineStr += wstring(width - size, L' ');
        else
            size = width;
        const wchar_t* lineChars = lineStr.c_str() + firstCol; // 定位于起始列字符指针
        WriteConsoleOutputCharacterW(hOut, lineChars, size, COORD{ rc.Left, row }, &written);
    }
}

void drawArea(HANDLE hOut, WORD attr, WORD shadowAttr, const SMALL_RECT& rc)
{
    SHORT shright = rc.Right - 2, right = shright - 1, bottom = rc.Bottom - 1;
    cleanArea(hOut, attr, SMALL_RECT{ rc.Left, rc.Top, shright, bottom });
    const wchar_t tabChar[] = L"─│┌┐└┘";
    for (auto row : { rc.Top, bottom }) // 顶、底行
        FillConsoleOutputCharacterW(hOut, tabChar[0], right - 2, { SHORT(rc.Left + 1), row }, &written);
    for (SHORT row = rc.Top + 1; row < bottom; ++row)
        for (SHORT col : { rc.Left, right })
            FillConsoleOutputCharacterW(hOut, tabChar[1], 1, { col, row }, &written);
    map<wchar_t, COORD> wchCoords = {
        { tabChar[2], { rc.Left, rc.Top } },
        { tabChar[3], { right, rc.Top } },
        { tabChar[4], { rc.Left, bottom } },
        { tabChar[5], { right, bottom } },
    };
    for (const auto& wchCoord : wchCoords)
        FillConsoleOutputCharacterW(hOut, wchCoord.first, 1, wchCoord.second, &written);
    // 底、右阴影色
    FillConsoleOutputAttribute(hOut, shadowAttr, shright - rc.Left, { SHORT(rc.Left + 1), rc.Bottom }, &written);
    right = rc.Right - 1;
    for (SHORT row = rc.Top + 1; row <= rc.Bottom; ++row)
        FillConsoleOutputAttribute(hOut, shadowAttr, 2, { right, row }, &written);
}

void cleanArea(HANDLE hOut, WORD attr, const SMALL_RECT& rc)
{
    int width{ rc.Right - rc.Left + 1 };
    for (SHORT row = rc.Top; row <= rc.Bottom; ++row) {
        FillConsoleOutputAttribute(hOut, attr, width, COORD{ rc.Left, row }, &written);
        FillConsoleOutputCharacterW(hOut, L' ', width, COORD{ rc.Left, row }, &written);
    }
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

wchar_t* getShowWstr(wchar_t* showWstr, const wchar_t* srcWstr)
{
    wchar_t wch;
    int srcIndex = 0, desIndex = 0;
    while ((wch = srcWstr[srcIndex++]) != L'\x0') {
        showWstr[desIndex++] = wch;
        // 制表字符
        if (wch >= 0x2500 && wch <= 0x2573)
            showWstr[desIndex++] = L' ';
    }
    showWstr[desIndex] = L'\x0';
    return showWstr;
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
    DWORD written;
    for (i = 0; i < 5; i++) {
        FillConsoleOutputAttribute(hOut, att0, chNum + 4, posShadow, &written);
        posShadow.Y++;
    }
    for (i = 0; i < 5; i++) {
        FillConsoleOutputAttribute(hOut, att1, chNum + 4, posText, &written);
        posText.Y++;
    }
    // 写文本和边框
    posText.X = rc.Left + 2;
    posText.Y = rc.Top + 2;
    WriteConsoleOutputCharacterA(hOut, str, strlen(str), posText, &written);
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
    DWORD written;
    // 画边框的上 下边界
    for (pos.X = rc.Left + 1; pos.X < rc.Right - 1; pos.X++) {
        pos.Y = rc.Top;
        // 画上边界
        WriteConsoleOutputCharacterA(hOut, &chBox[4], 1, pos, &written);
        // 画左上角
        if (pos.X == rc.Left + 1) {
            pos.X--;
            WriteConsoleOutputCharacterA(hOut, &chBox[0], 1, pos, &written);
            pos.X++;
        }
        // 画右上角
        if (pos.X == rc.Right - 2) {
            pos.X++;
            WriteConsoleOutputCharacterA(hOut, &chBox[1], 1, pos, &written);
            pos.X--;
        }
        pos.Y = rc.Bottom;
        // 画下边界
        WriteConsoleOutputCharacterA(hOut, &chBox[4], 1, pos, &written);
        // 画左下角
        if (pos.X == rc.Left + 1) {
            pos.X--;
            WriteConsoleOutputCharacterA(hOut, &chBox[2], 1, pos, &written);
            pos.X++;
        }
        // 画右下角
        if (pos.X == rc.Right - 2) {
            pos.X++;
            WriteConsoleOutputCharacterA(hOut, &chBox[3], 1, pos, &written);
            pos.X--;
        }
    }
    // 画边框的左右边界
    for (pos.Y = rc.Top + 1; pos.Y <= rc.Bottom - 1; pos.Y++) {
        pos.X = rc.Left;
        // 画左边界
        WriteConsoleOutputCharacterA(hOut, &chBox[5], 1, pos, &written);
        pos.X = rc.Right - 1;
        // 画右边界
        WriteConsoleOutputCharacterA(hOut, &chBox[5], 1, pos, &written);
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
            &cCharsWritten)) // Receive number of characters written
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
            &cCharsWritten)) // Receive number of characters written
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