#include <QCoreApplication>
#include <QCommandLineParser>

#include "recorder.h"
#include "cleanexit.h"
#include "ncursesuse.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QCommandLineParser parser;

    parser.addHelpOption();

    QCommandLineOption ipOption(QStringList() << "i" << "ip",
                                QCoreApplication::translate("main", "IP Call Agent"),
                                QCoreApplication::translate("main", "ip"));
    parser.addOption(ipOption);

    QCommandLineOption endPointOption(QStringList() << "e" << "endpoint",
                                      QCoreApplication::translate("main", "Endpoint"),
                                      QCoreApplication::translate("main", "endpoint"));
    parser.addOption(endPointOption);

    QCommandLineOption pathOption(QStringList() << "p" << "path",
                                  QCoreApplication::translate("main", "Path to save"),
                                  QCoreApplication::translate("main", "path"));
    parser.addOption(pathOption);

    parser.process(a);

    QString ipString = parser.value(ipOption);
    QString endPointString = parser.value(endPointOption);
    QString pathString = parser.value(pathOption);

    if ((ipString != "") && (endPointString != "") && (pathString != "")) {
        initializeNcurses();

        VoiceRecorder *voiceRec;
        voiceRec = new VoiceRecorder(QHostAddress(ipString), endPointString, pathString);
        voiceRec->bindMgcpSocket();

        CleanExit *cleanExit = new CleanExit;

        QObject::connect(cleanExit, SIGNAL(closeApplicationSignal()),
                         voiceRec, SLOT(closeApplication()));
    } else parser.showHelp(0);

    return a.exec();
}
