#include "serialBridge.h"

SerialBridge::SerialBridge(busDevice *busDevice): _serialBridgeProtocol(busDevice)
{
    connect(&_serialBridgeProtocol,  &serialBridgeProtocol::receiveData, this, &SerialBridge::on_receiveData);
    connect(&_tcpBridgeServer, &QTcpServer::newConnection, this, &SerialBridge::on_tcpBridgeNewConnection);

    _tcpBridgeCobsDecoder.clear();
}

void SerialBridge::tcpBridgeOpen(uint16_t port)
{
    if(!_tcpBridgeServer.listen(QHostAddress::Any, port))
    {
        emit tcpBridgeStatus("Server startup faild!",false,false);
    }
    else
    {
        emit tcpBridgeStatus("Server running. 0 active connections.",true,false);
    }
}

void SerialBridge::tcpBridgeClose()
{
    for(auto socket: _tcpBridgeSocket)
    {
        socket->flush();
        socket->close();
        disconnect(socket, &QTcpSocket::readyRead, this, &SerialBridge::on_tcpBridgeDataReceived);
    }
    _tcpBridgeSocket.clear();
    _tcpBridgeServer.close();
    emit tcpBridgeStatus("Server closed.",false,false);
}

void SerialBridge::on_tcpBridgeNewConnection(void)
{
    QTcpSocket *socket = _tcpBridgeServer.nextPendingConnection();
    socket->flush();
    connect(socket, &QTcpSocket::readyRead, this, &SerialBridge::on_tcpBridgeDataReceived);
    _tcpBridgeSocket.append(socket);

    emit tcpBridgeStatus("Server running. "+ QString::number(_tcpBridgeSocket.size()) +" active connections.",true,false);
}

void SerialBridge::on_tcpBridgeDataReceived()
{
    QByteArrayList frames = _tcpBridgeCobsDecoder.streamDecode(_tcpBridgeSocket.at(0)->readAll());// Todo: Fix this

    for(int i= 0; i< frames.length(); i++)
    {
        QByteArray frame = frames.at(i);
        _sendFrame((frame.at(0)>>4)&0x0F, frame.at(0)&0x0F, frame.mid(1,-1));
    }
}

void SerialBridge::on_receiveData(uint8_t port, serialBridgeProtocol::sbp_status_t status, QByteArray data)
{
    bool error = false;
    if(status != serialBridgeProtocol::sbp_status_t::ok) error = true;

    emit busMessage(data.toHex().prepend("0x"),error);

    for(auto socket: _tcpBridgeSocket)
    {
        socket->write(QuCLib::Cobs::encode(data));
    }
}

void SerialBridge::_sendFrame(uint8_t address, uint8_t command, QByteArray data)
{
    uint8_t cmdByte = (address << 4) | (command & 0x0F);
    _serialBridgeProtocol.sendData(0,data.prepend(cmdByte));
}
