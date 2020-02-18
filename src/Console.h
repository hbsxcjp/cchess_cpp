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
    SIMPLE = 0x10,
    SHOWY = 0x20,
    HIGHLIGHT = 0x30
} Theme;

// 菜单命令
typedef void (*MENU_FUNC)(void);

// 菜单结构
typedef struct Menu_ {
    wstring name, desc;
    MENU_FUNC func{}; // 菜单关联的命令函数，如有子菜单则应为空
    struct Menu_ *preMenu{}, *brotherMenu{}, *childMenu{};
    int brotherIndex{}, childIndex{};
} Menu;

// 菜单初始数据结构
typedef struct MenuData_ {
    wstring name, desc;
    MENU_FUNC func;
} MenuData;

class Console {
public:
    Console();

    void open(const string& fileName = string{});

    ~Console();

private:
    PDWORD pwritten;
    Menu* rootMenu_;
    HANDLE hIn_, hOut_;
    shared_ptr<ChessManual> cm_;

    void    __writeBoard();
    void __writeMove();

    void __initArea();
    void __initMenu();
    void __delMenu(Menu* menu);
};

void writeCharBuf(CHAR_INFO* charBuf, COORD bufSize, COORD bufCoord, SMALL_RECT& writeRect);

void setCharBuf(CHAR_INFO* charBuf, COORD charCoord, const wchar_t* wchars, WORD attr);

// 使用公用变量，获取用于屏幕显示的字符串（经试验，只有在制表符字符后插入空格，才能显示正常）
wchar_t* getShowWstr(const wchar_t* srcWstr);

// 取得字符指针至回车符的长度
int getLineSize(const wchar_t* srcWstr);

}

#endif
