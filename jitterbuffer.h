#ifndef JITTERBUFFER_H
#define JITTERBUFFER_H

#include <QObject>
#include <QByteArray>
#include <QList>
#include <QTimer>

#include "ncursesuse.h"

//Элемент в буфере
typedef struct _BufferListElement {

    quint16 sequenceNumber;    //Порядковый номер пакет
    QByteArray media;          //Массив медиа-данных

} BufferListElement;

//Смена элементов местами при записи, в случае неверной последовательности
class JitterBuffer : public QObject
{
    Q_OBJECT
public:

    explicit JitterBuffer(int packetSize, int bufferSize, QObject *parent = 0);
    int append(quint16 sequenceNumber, QByteArray media);
    int getPacketLostCount();
    int getBusySlotCount();

public slots:

    void clearAndWrite();

signals:

    void packetReady(QByteArray packet);

private slots:

    void removeSlot(int num);
    void sendGarbage(quint16 sequenceNumber);
    QByteArray getGarbagePacket();
    quint16 getMinimumNumber();
    int getPreviousSlot(quint16 number);
    int getNextSlot(quint16 number);
    int getSlotByNumber(quint16 number);

private:

    QList <BufferListElement> buffList_;
    int packetSize_;
    int nPacketLost_;
    int bufferSize_;
    quint16 lastSequenceNumber_;
    QTimer *betweenRcp;
};

#endif // JITTERBUFFER_H
