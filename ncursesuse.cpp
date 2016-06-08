#include "ncursesuse.h"

//Инициализация ncurses
void initializeNcurses()
{
    initscr();

    start_color();
    init_pair(1, COLOR_BLUE, COLOR_BLACK);
    init_pair(2, COLOR_CYAN, COLOR_BLACK);
    init_pair(3, COLOR_WHITE, COLOR_BLACK);

    curs_set(0);
}

//Вывод настроек приложения
void printInfoMessage(int row,       //Строка
                      QString key,   //Ключ
                      QString value, //Значение
                      int colorPair  //Цвет текста и фона
                      )
{
    attron(COLOR_PAIR(colorPair));
    mvwprintw(stdscr, row+1, 1, "%s:", key.toLatin1().data());
    attron(A_BOLD);
    mvwprintw(stdscr, row+1, strlen(key.toLatin1().data())+3, "%s", value.toLatin1().data());
    attroff(A_BOLD);
    attroff(COLOR_PAIR(colorPair));

    refresh();
}

//Вывод ответов MGCP
void printStatusMessage(const char *mask,   //Маска для вывода
                        int transId,        //Идентификатор транзаакции
                        int code,           //Код ответа
                        const char *comment //Текст ответа
                        )
{
    int totalRow, totalCol;
    getmaxyx(stdscr, totalRow, totalCol);

    attron(A_BOLD);
    mvwprintw(stdscr, totalRow-2, 1, mask, transId, code, comment);
    attroff(A_BOLD);

    refresh();
}

//Возвращение к исходному после ncurses
void restoreScreen()
{
    endwin();
}
