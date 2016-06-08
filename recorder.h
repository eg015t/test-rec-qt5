#ifndef VOICERECORDER_H
#define VOICERECORDER_H

#include <QObject>
#include <QUdpSocket>
#include <QFile>

#include "parser.h"
#include "audio.h"
#include "jitterbuffer.h"

#ifdef _DEBUG
#define CRCX_TEST_RET1 "200 2 OK\r\n"                           \
    "I: 1\r\n"                                                  \
    "\r\n"                                                      \
    "v=0\r\n"                                                   \
    "o=- 1 23 IN IP4 127.0.0.1\r\n"                             \
    "c=IN IP4 127.0.0.1\r\n"                                    \
    "t=0 0\r\n"                                                 \
    "m=audio 5004 RTP/AVP 126\r\n"                              \
    "a=rtpmap:126 AMR/8000\r\n"

#define CRCX_TEST_RET2 "200 1204 OK\r\n"                        \
    "I: FDE234C8\r\n"                                           \
    "v=0\r\n"                                                   \
    "o=- 1815849 0 IN IP4 194.67.15.181\r\n"                    \
    "s=-\r\n"                                                   \
    "c=IN IP4 194.67.15.181\r\n"                                \
    "t=0 0\r\n"                                                 \
    "m=audio 3456 RTP/AVP 99 18 101 100\r\n"                    \
    "m=video 10 RTP/AVP 31 34\r\n"

#define CRCX_TEST_RET3 "510 1 Protocol Error\r\n"

#define DLCX_TEST_RET1 "250 3205 OK\r\n"                        \
    "P: ps=1245, os=62345, pr=780, or=45123, pl=10, ji=26, la=4"
#endif

class VoiceRecorder : public QObject
{
    Q_OBJECT

public:

    explicit VoiceRecorder(QHostAddress Ip, QString endPoint, QString path,
                           QObject *parent = 0);
    void bindMgcpSocket();

signals:

    void readyRead();

public slots:

    void closeApplication();

private slots:

    void readMgcpDatagrams();
    void readRtpDatagrams();
    void createConnection(QString callId, QString version, int period,
                          QString codeck, QString mode);
    void deleteConnection();
    void modifyConnection();
    int getAudioPort(QList <MetadataStruct> metadataList);
    void bindRtpSocket(ResponseMgcpStruct MgcpDatagram);
    void saveFromBuffer(QByteArray inDatagram);
    void showStatics();

private:

    QUdpSocket *mgcpUdpSocket_; //Сокет для управления
    QUdpSocket *rtpUdpSocket_;  //Сокет для мультимедиа

    QHostAddress Ip_;
    QString endPoint_;

    quint16 localPort_;
    quint16 remotePort_;

    int transIdCounter_;        //Счетчик транзакций
    int currentTransId_;        //Идентификатор транзакции
    QString connectionId_;      //Идентификатор подключения

    int nPacketRtpRecieve_;     //Количество принятых медиа-пакетов
    int nPacketRtpWrite_;       //Количество записанных медиа-пакетов
    int nBytesRecieve_;         //Количество принятых байт
    int nBytesRec_;             //Количество записанных в файл байт
    QString fileRecName_;       //Имя записываемого файла
    QFile recFile_;             //Указатель на файл записи

    JitterBuffer *jittBuffer_;  //Буфер для записи правильной последовательности
};

#endif // VOICERECORDER_H
