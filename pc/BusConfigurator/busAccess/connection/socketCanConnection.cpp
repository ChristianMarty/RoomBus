#include "socketCanConnection.h"
#include <QCanBus>
#include <QVariant>

SocketCanConnection::SocketCanConnection(QString port)
    :_port{port}
{

}

SocketCanConnection::~SocketCanConnection()
{
    close();
}

void SocketCanConnection::open()
{
    if(_device!= nullptr){
        delete _device;
    }

    QString errorString;
    _device = QCanBus::instance()->createDevice("socketcan", _port, &errorString);
    if (!_device) {
        _device = nullptr;
    } else {
        _device->setConfigurationParameter(QCanBusDevice::CanFdKey, QVariant(true));
        _device->setConfigurationParameter(QCanBusDevice::BitRateKey, QVariant(500000));
        _device->setConfigurationParameter(QCanBusDevice::DataBitRateKey, QVariant(500000));
        _device->connectDevice();
        connect(_device, &QCanBusDevice::framesReceived, this, &SocketCanConnection::on_framesReceived);
        connect(_device, &QCanBusDevice::errorOccurred, this, &SocketCanConnection::on_errorOccurred);
    }
}

void SocketCanConnection::close()
{
    if(_device == nullptr) return;

    _device->disconnectDevice();
    disconnect(_device, &QCanBusDevice::framesReceived, this, &SocketCanConnection::on_framesReceived);
    disconnect(_device, &QCanBusDevice::errorOccurred, this, &SocketCanConnection::on_errorOccurred);

    _device->deleteLater();
    _device = nullptr;
}

bool SocketCanConnection::write(RoomBus::Message message)
{
    if(_device == nullptr){
        return false;
    }

    QCanBusFrame frame;
    frame.setExtendedFrameFormat(true);
    frame.setFlexibleDataRateFormat(true);
    frame.setBitrateSwitch(false);
    frame.setFrameType(QCanBusFrame::DataFrame);

    frame.setFrameId(RoomBus::toCanIdentifier(message));
    frame.setPayload(message.data);

    return _device->writeFrame(frame);
}

QString SocketCanConnection::getConnectionName()
{
    return "SocketCAN";
}

QString SocketCanConnection::getConnectionPath()
{
    return _port;
}

void SocketCanConnection::on_framesReceived()
{
    QList<QCanBusFrame> frames = _device->readAllFrames();

    for(const QCanBusFrame &frame : frames){
        emit received(RoomBus::toMessage(frame.frameId(), frame.payload()));
    }
}

void SocketCanConnection::on_errorOccurred(QCanBusDevice::CanBusError)
{

}
