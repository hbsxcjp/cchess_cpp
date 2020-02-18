#include "Console.h"
#include "ChessManual.h"
#include "Piece.h"

namespace ConsoleSpace {

static wchar_t showWstr[1024 * 8]; // 临时字符串
static constexpr wchar_t PROGRAMNAME[] = L"中国象棋 ";

static constexpr SHORT WINROWS = 40, WINCOLS = 120;
static constexpr SHORT STATUSROWS = 2;
static constexpr SHORT BOARDROWS = 19 + 4 + 2, BOARDCOLS = (9 + 8) * 2 + 6;
static constexpr COORD HOMEPOS = { 0, 0 };
static constexpr COORD WINCOORD = { WINCOLS, WINROWS };

static constexpr SMALL_RECT WinRect = { 0, 0, SHORT(WINCOLS - 1), SHORT(WINROWS - 1) };
static constexpr SMALL_RECT MenuRect = { WinRect.Left, WinRect.Top, WinRect.Right, 0 };
static constexpr SMALL_RECT StatusRect = { WinRect.Left, SHORT(WinRect.Bottom - STATUSROWS + 1), WinRect.Right, WinRect.Bottom };
static constexpr SMALL_RECT BoardRect = { WinRect.Left, SHORT(MenuRect.Bottom + 1), SHORT(WinRect.Left + BOARDCOLS - 1), SHORT(MenuRect.Bottom + BOARDROWS) };
static constexpr SMALL_RECT CurmoveRect = { BoardRect.Left, SHORT(BoardRect.Bottom + 1), BoardRect.Right, SHORT(StatusRect.Top - 1) };
static constexpr SMALL_RECT MoveRect = { SHORT(BoardRect.Right + 1), BoardRect.Top, WinRect.Right, CurmoveRect.Bottom };

/*
颜色属性由两个十六进制数字指定 -- 第一个对应于背景，第二个对应于前景。每个数字
可以为以下任何值:
    0 = 黑色       8 = 灰色
    1 = 蓝色       9 = 淡蓝色
    2 = 绿色       A = 淡绿色
    3 = 浅绿色     B = 淡浅绿色
    4 = 红色       C = 淡红色
    5 = 紫色       D = 淡紫色
    6 = 黄色       E = 淡黄色
    7 = 白色       F = 亮白色
//*/
static constexpr WORD WINATTR = 0x1F, MENUATTR = 0x8F, STATUSATTR = 0x84;
static constexpr WORD BOARDATTR = 0xF8, CURMOVEATTR = 0x65, MOVEATTR = 0x71;
static constexpr WORD RedSideAttr = 0xFC, BlackSideAttr = 0xF0;
static constexpr WORD RedAttr = 0xCF, BlackAttr = 0x0F, SelRedAttr = 0x8C, SelBlackAttr = 0x0F;

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

Console::Console()
    : pwritten{ nullptr }
    , rootMenu_{ nullptr }
    , hIn_{ GetStdHandle(STD_INPUT_HANDLE) }
    , hOut_{ CreateConsoleScreenBuffer(
          GENERIC_READ | GENERIC_WRITE, // read/write access
          FILE_SHARE_READ | FILE_SHARE_WRITE, // shared
          nullptr, // default security attributes
          CONSOLE_TEXTMODE_BUFFER, // must be TEXTMODE
          NULL) } //*/
    , cm_{ make_shared<ChessManual>() }
{
    SetConsoleCP(936);
    SetConsoleOutputCP(936);
    //SetConsoleMode(hIn_, ENABLE_PROCESSED_INPUT);
    SetConsoleTitleW(PROGRAMNAME); // 设置窗口标题

    SetConsoleScreenBufferSize(hOut_, WINCOORD);
    CONSOLE_CURSOR_INFO cInfo = { 5, false };
    SetConsoleCursorInfo(hOut_, &cInfo);
    SetConsoleTextAttribute(hOut_, WINATTR);
    SetConsoleWindowInfo(hOut_, true, &WinRect);
    SetConsoleActiveScreenBuffer(hOut_);

    //FillConsoleOutputAttribute(hOut_, MOVEATTR, WINROWS*WINCOLS, HOMEPOS, pwritten);
    __initArea();
    __initMenu();

    //_getch();
    //GetConsoleScreenBufferInfo(hOut_, &bInfo); // 获取窗口信息
}

void Console::open(const string& fileName)
{
    cm_ = make_shared<ChessManual>(fileName);
    __writeBoard();
    __writeMove();

    _getch();
}

Console::~Console()
{
    CloseHandle(hOut_); // 关闭标准输出设备句柄
    __delMenu(rootMenu_);
}

void Console::__writeBoard()
{
    bool bottomIsRed{ cm_->isBottomSide(PieceColor::RED) };
    int rows = BOARDROWS - 2, cols = BOARDCOLS - 6;
    SHORT left = BoardRect.Left + 3, top = BoardRect.Top + 1;
    WORD bottomAttr{ bottomIsRed ? RedSideAttr : BlackSideAttr }, topAttr{ bottomIsRed ? BlackSideAttr : RedSideAttr };
    for (auto row : { 0, 1, rows - 2, rows - 1 })
        FillConsoleOutputAttribute(hOut_, (row > 1 ? bottomAttr : topAttr), cols, { left, SHORT(top + row) }, pwritten);
    const wstring pieceChars{ cm_->getPieceChars() };
    for (int i = 0; i < SEATNUM; ++i) {
        wchar_t ch = pieceChars[i];
        if (ch == PieceManager::nullChar())
            continue;
        FillConsoleOutputAttribute(hOut_, (PieceManager::getColor(ch) == PieceColor::RED ? RedAttr : BlackAttr),
            2, { SHORT(left + (i % BOARDCOLNUM) * 4), SHORT(top + rows - 3 - (i / BOARDCOLNUM) * 2) }, pwritten);
    }
    int index = 0;
    getShowWstr(cm_->getBoardStr().c_str());
    for (int row = 0; row < rows; ++row) {
        COORD rowPos = { left, SHORT(top + row) };
        wchar_t* lineStr = &showWstr[index];
        int size = getLineSize(lineStr);
        WriteConsoleOutputCharacterW(hOut_, lineStr, size - 1, rowPos, pwritten); // size-1 去掉回车符所占长度
        index += size;
    }
}

void Console::__writeMove()
{
    getShowWstr(cm_->getMoveStr().c_str());
    int index{ 0 }, rows{ MoveRect.Bottom - MoveRect.Top - 2 }, cols{ MoveRect.Right - MoveRect.Left - 2 };
    for (int row = 0; row < rows; ++row) {
        //COORD rowPos = { SHORT(MoveRect.Left + (row % 2 == 1 ? 2 : 3)), SHORT(MoveRect.Top + 1 + row) };
        COORD rowPos = { SHORT(MoveRect.Left + 2), SHORT(MoveRect.Top + 1 + row) };
        wchar_t* lineStr = &showWstr[index];
        int size = getLineSize(lineStr);
        if (size > cols) // size为含有全角、半角的字符数，cols为半角字符数?
            size = cols;
        WriteConsoleOutputCharacterW(hOut_, lineStr, size - 1, rowPos, pwritten); // size-1 去掉回车符所占长度
        index += size;
    }
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
        WriteConsoleOutputCharacterW(hOut_, brotherMenu->name.c_str(), brotherMenu->name.size(),
            { SHORT(width), MenuRect.Top }, pwritten);
        width += brotherMenu->name.size() + 4;
    }
}

// 释放菜单资源
void Console::__delMenu(Menu* menu)
{
    if (menu->brotherMenu)
        __delMenu(menu->brotherMenu);
    if (menu->childMenu)
        __delMenu(menu->childMenu);
    delete menu;
}

void Console::__initArea()
{
    auto __cleanArea = [&](WORD attr, const SMALL_RECT& rc) {
        int width{ rc.Right - rc.Left + 1 };
        for (SHORT row = rc.Top; row <= rc.Bottom; ++row) {
            COORD pos = { rc.Left, row };
            FillConsoleOutputAttribute(hOut_, attr, width, pos, pwritten);
            FillConsoleOutputCharacterW(hOut_, L' ', width, pos, pwritten);
        }
    };
    auto __drawArea = [&](WORD attr, const SMALL_RECT& rc) {
        //__cleanArea(attr, rc);
        static const wchar_t tabChar[] = L"─│┌┐└┘";
        int chWidth{ rc.Right - rc.Left - 2 }, height{ rc.Bottom - rc.Top };
        //int width{ rc.Right - rc.Left + 1 }, chWidth{ width - 2 }, height{ rc.Bottom - rc.Top };
        //FillConsoleOutputAttribute(hOut_, attr, width, COORD{ rc.Left, rc.Top }, pwritten);
        //if (height > 0)
        //    FillConsoleOutputAttribute(hOut_, attr, width, COORD{ rc.Left, rc.Bottom }, pwritten);
        if (height <= 1)
            return;
        FillConsoleOutputCharacterW(hOut_, tabChar[0], chWidth, { SHORT(rc.Left + 1), rc.Top }, pwritten);
        FillConsoleOutputCharacterW(hOut_, tabChar[0], chWidth, { SHORT(rc.Left + 1), rc.Bottom }, pwritten);
        for (int row = rc.Top + 1; row < rc.Bottom; ++row) {
            //FillConsoleOutputAttribute(hOut_, attr, width, COORD{ rc.Left, SHORT(row) }, pwritten);
            FillConsoleOutputCharacterW(hOut_, tabChar[1], 1, { rc.Left, SHORT(row) }, pwritten);
            FillConsoleOutputCharacterW(hOut_, tabChar[1], 1, { SHORT(rc.Right - 1), SHORT(row) }, pwritten);
        }
        /*
        if (rc.Bottom < rc.Top + 2) // 少于2行则不画边框
            return;
        // 画边框
        for (auto row : { rc.Top, rc.Bottom }) // 顶、底行
            FillConsoleOutputCharacterW(hOut_, tabChar[0], width - 2, { SHORT(rc.Left + 1), row }, pwritten);
        SHORT right = rc.Right - 1;
        for (int row = rc.Top + 1; row < rc.Bottom; ++row)
            for (auto col : { rc.Left, right })
                FillConsoleOutputCharacterW(hOut_, tabChar[1], 1, { col, SHORT(row) }, pwritten);
        const map<wchar_t, COORD> wchCoords = {
            { tabChar[2], { rc.Left, rc.Top } },
            { tabChar[3], { right, rc.Top } },
            { tabChar[4], { rc.Left, rc.Bottom } },
            { tabChar[5], { right, rc.Bottom } },
        };
        for (auto& wchCoord : wchCoords)
            FillConsoleOutputCharacterW(hOut_, wchCoord.first, 1, wchCoord.second, pwritten);
            //*/
    };

    for (const auto& rectAttr : map<WORD, SMALL_RECT>{
             { MENUATTR, MenuRect },
             { BOARDATTR, BoardRect },
             { CURMOVEATTR, CurmoveRect },
             { MOVEATTR, MoveRect },
             { STATUSATTR, StatusRect } }) {
        __cleanArea(rectAttr.first, rectAttr.second);
        __drawArea(rectAttr.first, rectAttr.second);
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

wchar_t* getShowWstr(const wchar_t* srcWstr)
{
    int srcIndex = 0, desIndex = 0;
    wchar_t wch;
    while ((wch = srcWstr[srcIndex++]) != L'\x0') {
        showWstr[desIndex++] = wch;
        // 制表字符
        if (wch >= 0x2500 && wch <= 0x2573)
            showWstr[desIndex++] = L' ';
    }
    showWstr[desIndex] = L'\x0';
    return showWstr;
}

int getLineSize(const wchar_t* srcWstr)
{
    int index = 0;
    while (srcWstr[index++] != L'\n')
        ;
    return index;
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