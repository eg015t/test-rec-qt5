#include <QDataStream>
#include <QtCore/QtEndian>
#include <QDateTime>

#include "recorder.h"
#include "parser.h"

#define BUFFER_SIZE 5
#define PACKET_SIZE_MS 20

VoiceRecorder::VoiceRecorder(QHostAddress Ip,     //IP-адресс Call Agent
                             QString endPoint,    //Оконечное устройство
                             QString path,        //Путь для сохранения
                             QObject *parent) : QObject(parent),
    Ip_(Ip),
    endPoint_(endPoint),
    localPort_(2727),
    remotePort_(2427),
    transIdCounter_(0),
    nBytesRecieve_(0),
    nBytesRec_(0),
    fileRecName_(path.append(QDateTime::currentDateTime().toString("rec_hh_mm_ss").append(".raw"))),
    nPacketRtpRecieve_(0),
    nPacketRtpWrite_(0)
{
    printInfoMessage(0, "Call Ageent IP", Ip_.toString(), 2);
    printInfoMessage(1, "Endpoint", endPoint_, 2);
    printInfoMessage(2, "Record file", fileRecName_, 2);

    recFile_.setFileName(fileRecName_);
    recFile_.open(QIODevice::WriteOnly);

#ifdef _DEBUG
    /*    jittBuffer_ = new JitterBuffer(PACKET_SIZE_MS, BUFFER_SIZE);
    ResponseMgcpStruct respTest = parseMgcpReturn(QByteArray(CRCX_TEST_RET1));
    printStatusMessage("[%d] test %d (%s)", respTest.transId,
                       respTest.code, respTest.comment.toStdString().c_str());
    bindRtpSocket(respTest);
    connect(jittBuffer_, SIGNAL(packetReady(QByteArray)),
            this, SLOT(saveFromBuffer(QByteArray)));*/
#endif
}

//Инициализация сокета для приема MGCP-датаграмм
void VoiceRecorder::bindMgcpSocket()
{
    mgcpUdpSocket_ = new QUdpSocket(this);
    mgcpUdpSocket_->bind(Ip_, localPort_);

    connect(mgcpUdpSocket_, SIGNAL(readyRead()), this, SLOT(readMgcpDatagrams()));

    createConnection("1234", "0.1", PACKET_SIZE_MS, "PCMA", "recvonly");
}

//Создать соединение отправкой MGCP-датаграммы
void VoiceRecorder::createConnection(QString callId,    //Идентификатор вызова
                                     QString version,   //Версия протокола
                                     int period,        //Период пакетирования
                                     QString codeck,    //Кодек
                                     QString mode)      //Режим
{
    transIdCounter_++;

    QByteArray datagram = "CRCX";
    datagram.append(' '); datagram.append(QString::number(transIdCounter_));
    datagram.append(' '); datagram.append(endPoint_);
    datagram.append(' '); datagram.append("MGCP");
    datagram.append(' '); datagram.append(version); datagram.append("\r\n");

    datagram.append("C: "); datagram.append(callId); datagram.append("\r\n");

    datagram.append("L: p:"); datagram.append(QString::number(period));
    datagram.append(' '); datagram.append("a:"); datagram.append(codeck);
    datagram.append("\r\n");

    datagram.append("M: "); datagram.append(mode); datagram.append("\r\n");

    mgcpUdpSocket_->writeDatagram(datagram.data(), datagram.size(), Ip_, remotePort_);
}

//Удалить соединение
void VoiceRecorder::deleteConnection(QString version,     //Версия протокола
                                     QString connectionId //Идентификатор подключения
                                     )
{
    transIdCounter_++;

    QByteArray datagram = "DLCX";
    datagram.append(' '); datagram.append(QString::number(transIdCounter_));
    datagram.append(' '); datagram.append(endPoint_);
    datagram.append(' '); datagram.append("MGCP");
    datagram.append(' '); datagram.append(version); datagram.append("\r\n");

    datagram.append("I: "); datagram.append(connectionId); datagram.append("\r\n");

    mgcpUdpSocket_->writeDatagram(datagram.data(), datagram.size(), Ip_, remotePort_);
}

//Изменить соединение
//ex: modifyConnection("1234", "0.1", connectionId, "ca@ca1.whatever.net", "sendrecv");
void VoiceRecorder::modifyConnection(QString callId,         //Идентификатор вызова
                                     QString version,        //Версия протокола
                                     QString connectionId,   //Идентификатор подключения
                                     QString notifiedEntity, //Уведомляемый объект
                                     QString mode            //Режим
                                     )
{
    transIdCounter_++;

    QByteArray datagram = "MDCX";
    datagram.append(' '); datagram.append(QString::number(transIdCounter_));
    datagram.append(' '); datagram.append(endPoint_);
    datagram.append(' '); datagram.append("MGCP");
    datagram.append(' '); datagram.append(version); datagram.append("\r\n");

    datagram.append("C: "); datagram.append(callId); datagram.append("\r\n");

    datagram.append("I: "); datagram.append(connectionId); datagram.append("\r\n");

    datagram.append("N: "); datagram.append(notifiedEntity); datagram.append("\r\n");

    datagram.append("M: "); datagram.append(mode); datagram.append("\r\n");

    mgcpUdpSocket_->writeDatagram(datagram.data(), datagram.size(), Ip_, remotePort_);
}

//Обработка приема MGCP-датаграмм
void VoiceRecorder::readMgcpDatagrams()
{
    while (mgcpUdpSocket_->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(mgcpUdpSocket_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        mgcpUdpSocket_->readDatagram(datagram.data(), datagram.size(),
                                     &sender, &senderPort);

        ResponseMgcpStruct mgcpDatagram = parseMgcpReturn(datagram);

        //Предварительные ответы
        if ((mgcpDatagram.code >= 100) && (mgcpDatagram.code <= 199)) {
            printStatusMessage("[%d] preliminary response: %d (%s)", mgcpDatagram.transId,
                               mgcpDatagram.code, mgcpDatagram.comment.toStdString().c_str());
        } else

            //Команда успешно выполнена
            if ((mgcpDatagram.code >= 200) && (mgcpDatagram.code <= 299)) {
                printStatusMessage("[%d] completed successfully: %d (%s)", mgcpDatagram.transId,
                                   mgcpDatagram.code, mgcpDatagram.comment.toStdString().c_str());

                connectionId_ = mgcpDatagram.connectionId;

                //Запуск приема RTP-сообщений
                if (getAudioPort(mgcpDatagram.metadata) != 0) {
                    jittBuffer_ = new JitterBuffer(PACKET_SIZE_MS, BUFFER_SIZE);
                    connect(jittBuffer_, SIGNAL(packetReady(QByteArray)),
                            this, SLOT(saveFromBuffer(QByteArray)));
                    bindRtpSocket(mgcpDatagram);
                }

            } else

                //Отказ из-за случайной ошибки
                if ((mgcpDatagram.code >= 400) && (mgcpDatagram.code <= 499)) {
                    printStatusMessage("[%d] failure random error: %d (%s)", mgcpDatagram.transId,
                                       mgcpDatagram.code, mgcpDatagram.comment.toStdString().c_str());
                } else

                    //Отказ из-за постоянной ошибки
                    if ((mgcpDatagram.code >= 500) && (mgcpDatagram.code <= 599)) {
                        printStatusMessage("[%d] failure permanent error: %d (%s)", mgcpDatagram.transId,
                                           mgcpDatagram.code, mgcpDatagram.comment.toStdString().c_str());
                    } else

                        //Ответ с пакетной спецификой
                        if ((mgcpDatagram.code >= 800) && (mgcpDatagram.code <= 899)) {
                            printStatusMessage("[%d] packet specifics: %d (%s)", mgcpDatagram.transId,
                                               mgcpDatagram.code, mgcpDatagram.comment.toStdString().c_str());
                        }
    }
}

//Обработка приема RTP-датаграмм
void VoiceRecorder::readRtpDatagrams()
{
    while (rtpUdpSocket_->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(rtpUdpSocket_->pendingDatagramSize());
        QHostAddress sender;
        quint16 senderPort;

        rtpUdpSocket_->readDatagram(datagram.data(), datagram.size(),
                                    &sender, &senderPort);

        QDataStream datagramStream(&datagram, QIODevice::ReadOnly);
        datagramStream.setVersion(QDataStream::Qt_5_5);

        RtpHeader rtpHeader;
        datagramStream.readRawData((char*)&rtpHeader, sizeof(rtpHeader));

        rtpHeader.sequenceNumber = qToBigEndian<quint16>(rtpHeader.sequenceNumber);
        rtpHeader.timeStamp = qToBigEndian<quint32>(rtpHeader.timeStamp);
        rtpHeader.synchSourceId = qToBigEndian<quint32>(rtpHeader.synchSourceId);

        //Если принят именно RTP-пакет
        if (rtpHeader.version == 2)
            //Если кодек PCMA
            if (rtpHeader.payloadType == PCMA) {
                nPacketRtpRecieve_++;
                nBytesRecieve_ = nBytesRecieve_ + datagram.size() - sizeof(RtpHeader);
                datagram.remove(0, sizeof(RtpHeader));

                jittBuffer_->append(rtpHeader.sequenceNumber, datagram);

                showStatics();
            }
    }
}

//Вывести на экран статистику по работе приложения
void VoiceRecorder::showStatics()
{
    /*QString bufferBusyString;
    bufferBusyString.clear();

    int bufferBusy = jittBuffer_->getBusySlotCount();
    for (int i = 0; i < BUFFER_SIZE; i++)
        if (i < bufferBusy) bufferBusyString.append('|');
        else bufferBusyString.append(' ');
    bufferBusyString.prepend('['); bufferBusyString.append(']');*/

    printInfoMessage(4, "Jitter-Buffer", QString::number(jittBuffer_->getBusySlotCount()), 3);
    printInfoMessage(5, "Total packets received", QString::number(nPacketRtpRecieve_), 3);
    printInfoMessage(6, "Total packets written", QString::number(nPacketRtpWrite_), 3);
    printInfoMessage(7, "Total packets lost", QString::number(jittBuffer_->getPacketLostCount()), 3);
    printInfoMessage(8, "Total bytes received", QString::number(nBytesRecieve_), 3);
    printInfoMessage(9, "Total bytes written", QString::number(nBytesRec_), 3);
}

//Сохранение в файл пришедшего пакета из буфера
void VoiceRecorder::saveFromBuffer(QByteArray inDatagram)
{
    nBytesRec_ = nBytesRec_ + recFile_.write(inDatagram);

    nPacketRtpWrite_++;

    showStatics();
}

//Получение порта аудиопотока
int VoiceRecorder::getAudioPort(QList <MetadataStruct> metadataList //Список медиаданных
                                )
{
    for (short i = 0; i < metadataList.count(); i++)
        if (metadataList.at(i).metadataName == "audio") return metadataList.at(i).port;

    return 0;
}

//Инициализация сокета для приема аудиопотока
void VoiceRecorder::bindRtpSocket(ResponseMgcpStruct mgcpDatagram  //Ответ на MGCP-датаграмму
                                  )
{
    rtpUdpSocket_ = new QUdpSocket(this);
    rtpUdpSocket_->bind(mgcpDatagram.ownerId.address, getAudioPort(mgcpDatagram.metadata));

    connect(rtpUdpSocket_, SIGNAL(readyRead()), this, SLOT(readRtpDatagrams()));
}

//Завершение приложения
void VoiceRecorder::closeApplication()
{
    //qDebug() << "Shutdown application CTRL+C.";
    deleteConnection("0.1", connectionId_);

    recFile_.close();

    restoreScreen();
}
