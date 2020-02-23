#ifndef CONSOLE_H
#define CONSOLE_H

#include "ChessType.h"
#include <conio.h>
#include <stdio.h>
#include <windows.h>
#define UNICODE

#define KEY_ESC 0x1b /* Escape */

namespace ConsoleSpace {

// 控制台焦点区域类型
typedef enum {
    MOVEA,
    CURMOVEA,
    BOARDA,
    MENUA,
    //STATUSA
} FocusArea;

// 区域主题颜色配置类型
typedef enum {
    SIMPLE,
    SHOWY,
    HIGHLIGHT
} Thema;

// 菜单命令
typedef void (*MENU_FUNC)(void);

// 菜单结构
typedef struct Menu_ {
    wstring name, desc;
    MENU_FUNC func; // 菜单关联的命令函数，如有子菜单则应为空
    struct Menu_ *preMenu, *brotherMenu, *childMenu;
    int brotherIndex, childIndex;
} Menu;

// 菜单初始信息结构
typedef struct MenuData_ {
    wstring name, desc;
    MENU_FUNC func;
} MenuData;

class Console {
public:
    Console(const string& fileName = string{});

    void setThema(Thema thema) { thema_ = thema; }

    ~Console();

private:
    HANDLE hIn_, hOut_;
    Menu *rootMenu_, *curMenu_;
    shared_ptr<ChessManual> cm_;
    Thema thema_{ SHOWY };
    int areaI_{};
    int cmFirstRow_{}, cmFirstCol_{}, mFirstRow_{}, mFirstCol_{};

    void __operateWin();
    bool __operateMenu(const KEY_EVENT_RECORD& ker);
    void __operateBoard(WORD key);
    void __operateMove(WORD key);
    void __operateCurMove(WORD key);

    void __writeAreas();
    void __writeBoard();
    void __writeMove();
    void __writeCurmove();
    void __writeStatus();
    void __writeSubMenu(Menu* menu, int rightSpaceNum);
    void __writeAreaLineChars(WORD attr, const wchar_t* lineChars, const SMALL_RECT& rc, int firstRow = 0, int firstCol = 0, bool cutLine = false);

    void __initMenu();
    void __initArea(WORD attr, WORD shadowAttr, const SMALL_RECT& rc);
    void __cleanArea(WORD attr, const SMALL_RECT& rc);
};

// 选定菜单定位至顶层菜单
Menu* getTopMenu(Menu* menu);
// 选定菜单定位至底层菜单
Menu* getBottomMenu(Menu* menu, int row = 100);
// 选定菜单定位至左边或右边相同或相近层菜单
Menu* getSameRowMenu(Menu* menu, bool isRight);

// 清除内容

void writeCharBuf(CHAR_INFO* charBuf, COORD bufSize, COORD bufCoord, SMALL_RECT& writeRect);
void setCharBuf(CHAR_INFO* charBuf, COORD charCoord, const wchar_t* wchars, WORD attr);

// 取得宽字符串所占的屏幕宽度
int getWstrLength(const wstring& wstr);
}

#endif
