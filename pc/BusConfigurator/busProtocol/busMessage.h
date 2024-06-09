#ifndef BUSMESSAGE_H
#define BUSMESSAGE_H

#include <QObject>

#include "protocol/protocol.h"

class busMessage
{
public:

    uint8_t srcAddress;
    uint8_t dstAddress;

    Protocol protocol;
    uint8_t command;

    QByteArray data;
};

#endif // BUSMESSAGE_H
