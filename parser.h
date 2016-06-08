#ifndef PARSER_H
#define PARSER_H

#include <QString>
#include <QHostAddress>
#include <QList>

//Идентификаторы создателя/владельца и сессии (o=)
typedef struct _OwnerIdStruct
{

    QString user;               //Идентификатор пользователя
    int sessionId;              //Идентификатор сессии
    int version;                //Версия сессии
    QString protocol;           //Сетевой протокол создателя сессии
    QString typeIp;             //Тип IP-адреса создателя сессии
    QHostAddress address;       //Адрес инициатора сессии

} OwnerIdStruct;

//Информация для соединения (c=)
typedef struct _ConnectionInfoStruct
{

    QString protocol;           //Сетевой протокол создателя сессии
    QString typeIp;             //Тип IP-адреса создателя сессии
    QHostAddress address;       //Адрес инициатора сессии

} ConnectionInfoStruct;

//Тип медиаданных и транспортный адрес устройства (m=)
typedef struct _MetadataStruct
{

    QString metadataName;       //Название медиаданных
    int port;                   //Транспортный адрес устройства
    QString setParameters;      //Набор параметров
    QList <short> payloadTypes; //Методы кодирования по номерам

} MetadataStruct;

//Структура для ответа
typedef struct _ResponseMgcpStruct
{

    //Команда
    int code;
    int transId;
    QString comment;
    //Основные параметры
    QString connectionId;        //Идентификатор подключения (I)
    QString connectionParameters;//Параметры соединения (P)
    //Параметры SDP-сессии
    OwnerIdStruct ownerId;
    ConnectionInfoStruct connectionInfo;
    QList <MetadataStruct> metadata;

} ResponseMgcpStruct;

ResponseMgcpStruct parseMgcpReturn(QByteArray mgcpReturnArray);

OwnerIdStruct getOwnerId(QString o);
ConnectionInfoStruct getConnectionInfo(QString c);
MetadataStruct getMetadata(QString m);

#endif // PARSER_H
