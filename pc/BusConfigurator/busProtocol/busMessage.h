#ifndef BUSMESSAGE_H
#define BUSMESSAGE_H

#include <QObject>

#include "protocol/protocol.h"

struct BusMessage
{
    uint8_t sourceAddress;
    uint8_t destinationAddress;
    Protocol protocol;
    uint8_t command;
    QByteArray data;
};

#endif // BUSMESSAGE_H
