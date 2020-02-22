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
    MENUA,
    BOARDA,
    MOVESA,
    CURMOVEA,
    STATUSA
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
    MENU_FUNC func{}; // 菜单关联的命令函数，如有子菜单则应为空
    struct Menu_ *preMenu{}, *brotherMenu{}, *childMenu{};
    int brotherIndex{}, childIndex{};
} Menu;

// 菜单初始信息结构
typedef struct MenuData_ {
    wstring name, desc;
    MENU_FUNC func;
} MenuData;

// 区域结构
typedef struct {
    SMALL_RECT rect, irect;
    wchar_t* pwchar{};
    SHORT sr, sc, br, bc;
    int firstRow, firstCol; // 字符串显示的首行、首列
} Area;

class Console {
public:
    Console(const string& fileName = string{});

    void open(const string& fileName);

    ~Console();

private:
    HANDLE hIn_, hOut_;
    shared_ptr<ChessManual> cm_;
    Area *winA_, *menuA_, *statusA_, *boardA_, *curmoveA_, *moveA_;
    Menu* rootMenu_{};
    Thema thema_{ SHOWY };

    void __writeAreas();
    void __writeBoard();
    void __writeMove();
    void __writeCurmove();
    void __writeStatus();
    void __writeSubMenu(Menu* menu, int rightSpaceNum);

    void __initArea();
    void __initMenu();
};

// 选定菜单定位至顶层菜单
Menu* getTopMenu(Menu* menu);
// 选定菜单定位至底层菜单
Menu* getBottomMenu(Menu* menu, int row = 100);
// 选定菜单定位至左边或右边相同或相近层菜单
Menu* getSameRowMenu(Menu* menu, bool isRight);

void writeAreaChars(HANDLE hOut, const wchar_t* pwchar, const SMALL_RECT& rc, int firstRow = 0, int firstCol = 0);
void writeAreaWstr(HANDLE hOut, const wstring& wstr, int firstCol, int firstRow, const SMALL_RECT& rc);
// 清除内容
void drawArea(HANDLE hOut, WORD attr, WORD shadowAttr, const SMALL_RECT& rc);
void cleanArea(HANDLE hOut, WORD attr, const SMALL_RECT& rc);

void writeCharBuf(CHAR_INFO* charBuf, COORD bufSize, COORD bufCoord, SMALL_RECT& writeRect);
void setCharBuf(CHAR_INFO* charBuf, COORD charCoord, const wchar_t* wchars, WORD attr);

// 使用公用变量，获取用于屏幕显示的字符串（经试验，只有在制表符字符后插入空格，才能显示正常）
// 全角字符占2个字符位置，制表符占1个字符位置
void setTabsWstr(wchar_t* showWstr, const wchar_t* srcWstr);
// 取得给定列数范围的对齐行
void setAlignedWstr(wchar_t* showWstr, const wchar_t* srcWstr, int cols);
// 取得宽字符串所占的屏幕宽度
int getWstrLength(const wstring& wstr);
}

#endif
