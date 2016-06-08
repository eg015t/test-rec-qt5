#ifndef AUDIO_H
#define AUDIO_H

enum RtpProfile {
    PCMU        = 0,
    GSM         = 3,
    G723        = 4,
    DVI4_8000   = 5,
    DVI4_16000  = 6,
    LPC         = 7,
    PCMA        = 8,
    G722		= 9,
    L16_2		= 10,
    L16_1		= 11,
    QCELP       = 12,
    CN          = 13,
    MPA         = 14,
    G728		= 15,
    DVI4_11025  = 16,
    DVI4_22050  = 17,
    G729        = 18
};

#pragma pack (push, 1)

//Заголовок RTP-пакета, самый простой вариант, без учета CSRC, расширений...
typedef struct _RtpHeader
{

#if defined _LITTLE_ENDIAN
    quint8 sourceIdCount:4;    //Количество CSRC-идентификаторов
    quint8 extension:1;        //Расширения
    quint8 padding:1;          //Смещение
    quint8 version:2;          //Версия протокола (2)
#elif defined _BIG_ENDIAN
    quint8 version:2;          //Версия протокола (2)
    quint8 padding:1;          //Смещение
    quint8 extension:1;        //Расширения
    quint8 sourceIdCount:4;    //Количество CSRC-идентификаторов
#endif
#if defined _LITTLE_ENDIAN
    quint8 payloadType:7;      //Формат полезной нагрузки
    quint8 marker:1;           //Маркер полезности
#elif defined _BIG_ENDIAN
    quint8 marker:1;           //Маркер полезности
    quint8 payloadType:7;      //Формат полезной нагрузки
#endif
    quint16 sequenceNumber;    //Порядковый номер
    quint32 timeStamp;         //Метка времени
    quint32 synchSourceId;     //SSRC-идентификатор

} RtpHeader;

#pragma pack (pop)

#endif // AUDIO_H
