#include <QChar>
#include <QStringList>

#include "parser.h"

//Парсинг полученной датаграммы на отправку MGCP-пакета
ResponseMgcpStruct parseMgcpReturn(QByteArray mgcpReturnArray)
{
    short linesCount = 0;
    ResponseMgcpStruct retParsed;
    QStringList cmdList;
    bool ok;

    QTextStream in(mgcpReturnArray);

    while (!in.atEnd())
    {
        linesCount ++;
        QString line = in.readLine();

        if (line.length() == 0) continue;

        //Ответ на команду
        if (line.at(0).isDigit()) {
            cmdList = line.split(' ', QString::SkipEmptyParts);

            retParsed.code = QString(cmdList.at(0)).toInt(&ok,10);
            retParsed.transId = QString(cmdList.at(1)).toInt(&ok,10);
            retParsed.comment.clear();
            for (short i = 2; i < cmdList.count(); i++)
                retParsed.comment.append(cmdList.at(i));
        } else

            //Основные параметры
            if (line.at(0).isUpper()){
                cmdList = line.split(": ", QString::SkipEmptyParts);

                if (cmdList.at(0) == "I") retParsed.connectionId = cmdList.at(1);
                else
                    if (cmdList.at(0) == "P") retParsed.connectionParameters = cmdList.at(1);
            } else

                //Параметры SDP-сессии
                if (line.at(0).isLower()){
                    QChar key = line.at(0);

                    if (key == 'o') retParsed.ownerId = getOwnerId(line.remove(0,2));
                    else
                        if (key == 'c') retParsed.connectionInfo = getConnectionInfo(line.remove(0,2));
                        else
                            if (key == 'm') retParsed.metadata.append(getMetadata(line.remove(0,2)));
                }
    }

    return retParsed;
}

//Вычленение создателя/владельца и сессии
OwnerIdStruct getOwnerId(QString o)
{
    OwnerIdStruct retO;

    QStringList oList = o.split(' ', QString::SkipEmptyParts);

    retO.user = oList.at(0);
    retO.sessionId  = oList.at(1).toInt();
    retO.version  = oList.at(2).toInt();
    retO.protocol = oList.at(3);
    retO.typeIp = oList.at(4);
    retO.address = QHostAddress(oList.at(5));

    return retO;
}

//Вычленение информации для соединения
ConnectionInfoStruct getConnectionInfo(QString c)
{
    ConnectionInfoStruct retC;

    QStringList cList = c.split(' ', QString::SkipEmptyParts);

    retC.protocol = cList.at(0);
    retC.typeIp = cList.at(1);
    retC.address = QHostAddress(cList.at(2));

    return retC;
}

//Вычленение типа медиаданных
MetadataStruct getMetadata(QString m)
{
    MetadataStruct retM;

    QStringList mList = m.split(' ', QString::SkipEmptyParts);

    retM.metadataName = mList.at(0);
    retM.port = mList.at(1).toInt();
    retM.setParameters = mList.at(2);
    for (short i = 3; i < mList.count(); i++)
        retM.payloadTypes.append(mList.at(i).toInt());

    return retM;
}
