#ifndef CONSOLE_H
#define CONSOLE_H

//#include "ChessType.h"

namespace ConsoleSpace {

#define MAXSTRLEN 256
#define MENUNUM 4
#define MENULEVEL 10
#define MENUWIDTH 9
#define KEY_ESC 0x1b /* Escape */

#define COLOR_V(num) ((short)((num) / 255.0 * 1000.0))
#define COLOR_PAIR_NUM(theme, area) ((theme) | (area))
#define THEMECOLOR(theme, area) COLOR_PAIR(COLOR_PAIR_NUM(theme, area))

// ����̨������������
typedef enum {
    MENUA,
    BOARDA,
    MOVESA,
    CURMOVEA,
    STATUSA
} FocusArea;

// ����������ɫ��������
typedef enum {
    SIMPLE = 0x10,
    SHOWY = 0x20,
    HIGHLIGHT = 0x30
} Theme;

// �˵�����
typedef void (*MENU_FUNC)(void);

// �˵��ṹ
typedef struct Menu_ {
    wchar_t name[12]; /* item label */
    MENU_FUNC func; /* (pointer to) function */
    wchar_t desc[50]; /* function description */
    struct Menu_ *preMenu, *nextMenu, *otherMenu;
    int colIndex, rowIndex;
} Menu;

// �˵���ʼ���ݽṹ
typedef struct MenuData_ {
    wchar_t name[12]; /* item label */
    MENU_FUNC func; /* (pointer to) function */
    wchar_t desc[50]; /* function description */
} MenuData;

void doView(void);

void view0();

}

#endif