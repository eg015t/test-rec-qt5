#include "cleanexit.h"

static CleanExit *pCleanExit;

CleanExit::CleanExit(QObject *parent) : QObject(parent)
{
    pCleanExit = this;

    signal(SIGINT, &CleanExit::exitQt);
    signal(SIGTERM, &(CleanExit::exitQt));
}

//Событие на завершение приложения
void CleanExit::exitQt(int sig)
{
    emit pCleanExit->closeApplicationSignal();
    QCoreApplication::exit(0);
}
