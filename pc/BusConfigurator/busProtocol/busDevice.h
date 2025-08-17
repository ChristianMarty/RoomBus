#ifndef BUSDEVICE_H
#define BUSDEVICE_H

#include <QObject>

#include <QTimer>
#include <QMap>

#include "protocol/protocolBase.h"
#include "protocol/deviceManagementProtocol/deviceManagementProtocol.h"

class RoomBusDevice : public QObject
{
    Q_OBJECT
public:
    explicit RoomBusDevice(uint8_t deviceAddress, QObject *parent = nullptr);

    void setDeviceAddress(uint8_t deviceAddress);
    uint8_t deviceAddress() const;

    QString deviceName() const;
    QString applicationName() const;
    QString hardwareName() const;

    QString kernelVersionString(void) const;
    QString hardwareVersionString(void) const;
    QString deviceIdentificationString(void) const;
    QString deviceSerialNumberString(void) const;

    DeviceManagementProtocol::SystemStatus systemStatus() const;
    bool timeoutStatus(void) const;
    QDateTime lastHeartbeat() const;

    DeviceManagementProtocol &management(void);

    void addProtocol(ProtocolBase* protocol);
    void removeProtocol(ProtocolBase* protocol);

    void handleMessage(RoomBus::Message message);

    static RoomBus::Message busScan(void);

signals:
    void dataReady(RoomBus::Message message);

private:
    uint8_t _deviceAddress;

    DeviceManagementProtocol _deviceManagementProtocol;
    QList<ProtocolBase*> _protocols;
};

#endif // BUSDEVICE_H
