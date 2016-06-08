#include "jitterbuffer.h"

#define SAMPLING_FREQUENCY 8000
#define BIT_REGISTER 8

JitterBuffer::JitterBuffer(int packetSize, //Размер пакета в мс
                           int bufferSize, //Размер буфера
                           QObject *parent) : QObject(parent),
    packetSize_(packetSize),
    nPacketLost_(0),
    bufferSize_(bufferSize),
    lastSequenceNumber_(0)
{
    //Таймер - если секунду не было входящих пакетов - освобождаем буфер
    betweenRcp = new QTimer(this);
    connect(betweenRcp, SIGNAL(timeout()), this, SLOT(clearAndWrite()));
    betweenRcp->setInterval(1000);
}

//Выемка из буфера и отправка в файл
void JitterBuffer::removeSlot(int num //Номер элемента
                              )
{
    lastSequenceNumber_ = buffList_.at(num).sequenceNumber;

    emit packetReady(buffList_.at(num).media);

    buffList_.removeAt(num);

    betweenRcp->start();
}

//Отпрвка пустого пакета, вместо потерянного
void JitterBuffer::sendGarbage(quint16 sequenceNumber //Порядковый номер пакета
                               )
{
    lastSequenceNumber_ = sequenceNumber;

    nPacketLost_++;

    emit packetReady(getGarbagePacket());
}

//Добавление пакета в буфер
int JitterBuffer::append(quint16 sequenceNumber, //Порядковый номер пакета
                         QByteArray media)       //Данные
{
    betweenRcp->stop();

    BufferListElement newEl;
    newEl.sequenceNumber = sequenceNumber;
    newEl.media = media;

    //Есть место в буфере - добавляем
    if (buffList_.count() < bufferSize_)
        buffList_.append(newEl);

    //Буфер заполнен - ищем кандидатов на отправку
    else {
        int minNumSlot = getSlotByNumber(getMinimumNumber());

        //Еще отправлений не было
        if (lastSequenceNumber_ == 0)
            removeSlot(minNumSlot);
        else {
            //Номера последовательны
            if ((lastSequenceNumber_+1 ) == buffList_.at(minNumSlot).sequenceNumber)
                removeSlot(minNumSlot);
            //Отправляем пустые до покрытия пропуска
            else {
                for (int i = lastSequenceNumber_ + 1; i < buffList_.at(minNumSlot).sequenceNumber; i++)
                    sendGarbage(i);

                removeSlot(minNumSlot);
            }
        }
        buffList_.append(newEl);
    }

    return buffList_.count();
}

//Дописать оставшиеся пакеты из буфера
void JitterBuffer::clearAndWrite()
{
    lastSequenceNumber_ = 0;

    while (buffList_.count() != 0)
        removeSlot(0);
}

//Заполнить пакет нулями
//Размер цифрового моноаудиофайла измеряется по формуле: A = D*T*i,
QByteArray JitterBuffer::getGarbagePacket()
{
    QByteArray retBA;
    retBA.clear();

    int a = SAMPLING_FREQUENCY * packetSize_ / 1000 * BIT_REGISTER / 8;

    for (int i = 0; i < a; i++) retBA.append('0');

    return retBA;
}

//Получить позицию с предидущим номером (-1 при отстутсвтии)
int JitterBuffer::getPreviousSlot(quint16 number //Порядковый номер
                                  )
{
    if (buffList_.count() == 0) return -1;

    for (int i = 0; i < buffList_.count() - 1; i++)
        if (buffList_.at(i).sequenceNumber == (number-1)) return i;

    return -1;
}

//Получить позицию со следующим номером (-1 при отстутсвтии)
int JitterBuffer::getNextSlot(quint16 number //Порядковый номер
                              )
{
    if (buffList_.count() == 0) return -1;

    for (int i = 0; i < buffList_.count() - 1; i++)
        if (buffList_.at(i).sequenceNumber == (number+1)) return i;

    return -1;
}

//Получить позицию по номеру
int JitterBuffer::getSlotByNumber(quint16 number //Порядковый номер
                                  )
{
    if (buffList_.count() == 0) return -1;

    for (int i = 0; i < buffList_.count() - 1; i++)
        if (buffList_.at(i).sequenceNumber == number) return i;

    return -1;
}

//Получить минимальное значение в буфере
quint16 JitterBuffer::getMinimumNumber()
{
    if (buffList_.count() == 0) return -1;

    quint16 minNum = UINT_MAX;

    for (int i = 0; i < buffList_.count() - 1; i++)
        if (buffList_.at(i).sequenceNumber < minNum) minNum = buffList_.at(i).sequenceNumber;

    return minNum;
}

//Получить количество потерянных пакетов
int JitterBuffer::getPacketLostCount()
{
    return nPacketLost_;
}

int JitterBuffer::getBusySlotCount()
{
    return buffList_.count();
}
