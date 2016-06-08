#ifndef CLEANEXIT_H
#define CLEANEXIT_H

#include <QObject>
#include <QtCore/QCoreApplication>
#include <QFile>

#include <csignal>

class CleanExit : public QObject
{
    Q_OBJECT

public:

    explicit CleanExit(QObject *parent = 0);

private:

    static void exitQt(int sig);

signals:

    void closeApplicationSignal();
};

#endif // CLEANEXIT_H
