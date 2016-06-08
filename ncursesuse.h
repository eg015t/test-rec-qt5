#ifndef NCURSESUSE_H
#define NCURSESUSE_H

#include <QString>

#include <ncurses.h>

//Количество строк и столбцов на экране терминала
//int row, col;

void initializeNcurses();

void restoreScreen();

void printInfoMessage(int rowC, QString key, QString value, int colorPair);

void printStatusMessage(const char *mask, int transId, int code, const char *comment);

#endif // NCURSESUSE_H
