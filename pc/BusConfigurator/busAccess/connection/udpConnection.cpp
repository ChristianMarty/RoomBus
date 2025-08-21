#include "udpConnection.h"
#include "busAccess.h"

UdpConnection::UdpConnection(MiniBusAccess *parrent, QString ip, uint16_t port)
    :_ip{ip}
    ,_port{port}
{
    _parrent = parrent;

    _canSerial.setBaudrate(CANbeSerial::Baud500k);
    _canSerial.setDataBaudrate(CANbeSerial::Baud500k);
    _canSerial.setTxPaddingEnable(true, 0x00);

    connect(&_canSerial, &CANbeSerial::writeReady, this, &UdpConnection::on_canBeSerial_writeReady);
    connect(&_canSerial, &CANbeSerial::readReady, this, &UdpConnection::on_canBeSerial_readReady);

    connect(&_udpClient, &QUdpSocket::readyRead, this, &UdpConnection::on_readyRead);
    connect(&_udpClient, &QUdpSocket::connected,this, &UdpConnection::on_udpConnect);
    connect(&_udpClient, &QUdpSocket::disconnected,this, &UdpConnection::on_udpDisconnect);
    connect(&_udpClient, &QUdpSocket::stateChanged, this , &UdpConnection::on_stateChanged);
}

UdpConnection::~UdpConnection()
{
    close();
}

void UdpConnection::open()
{
    _udpHost.setAddress(_ip);
    emit _parrent->connectionChanged();
}

void UdpConnection::close()
{
    _udpClient.disconnectFromHost();
}


QString UdpConnection::getConnectionName()
{
    return "UDP";
}

QString UdpConnection::getConnectionPath()
{
    return "IP "+_ip+":"+QString::number(_port);
}

void UdpConnection::on_canBeSerial_writeReady(QByteArray data)
{
    if(!_isConnected) return;

    _udpClient.write(data,data.size());
}

void UdpConnection::on_canBeSerial_readReady(CanBusFrame frame)
{
    _parrent->_handleMessageReceived(MiniBus::toMessage(frame.identifier, frame.data));
}

void UdpConnection::on_udpConnect(void)
{
    _isConnected = _udpClient.isOpen();

    if(_isConnected){
        _canSerial.setEnabled(true);
    }

    emit _parrent->connectionChanged();
}

void UdpConnection::on_udpDisconnect(void)
{
    _isConnected = _udpClient.isOpen();
    emit _parrent->connectionChanged();
}

void UdpConnection::on_stateChanged(QAbstractSocket::SocketState socketState)
{
    Q_UNUSED(socketState);

    _isConnected = _udpClient.isOpen();
    emit _parrent->connectionChanged();
}

bool UdpConnection::write(MiniBus::Message message)
{
    if(!_isConnected){
        return false;
    }

    CanBusFrame frame;
    frame.identifier = MiniBus::toCanIdentifier(message);
    frame.extended = true;
    frame.fd = true;
    frame.data = message.data;
    frame.rtr = false;

    _canSerial.write(frame);

    return true;
}

void UdpConnection::on_readyRead(void)
{
    while (_udpClient.hasPendingDatagrams())
    {
        QHostAddress sender;
        uint16_t port;
        QByteArray datagram;
        datagram.resize(_udpClient.pendingDatagramSize());
        _udpClient.readDatagram(datagram.data(),datagram.size(), &sender, &port);


        //uint32_t inPort = _udpClient.();
        //port == inPort
        if(_udpHost.isEqual(sender)){
            _canSerial.receive(datagram);
        }
    }
}
