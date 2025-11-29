#include "serialBridgeProtocol.h"
#include "busDevice.h"

SerialBridgeProtocol::SerialBridgeProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _device->addProtocol(this);
}

void SerialBridgeProtocol::sendData(uint8_t port, QByteArray data)
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::SerialBridgeProtocol;
    msg.command = (uint8_t)Command::Data;
    //msg.data.append(port);
    //msg.data.append(static_cast<uint8_t>(sbp_status_t::ok));
    msg.data.append(data);

    sendMessage(msg);
}

void SerialBridgeProtocol::handleMessage(const MiniBus::Message &message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::SerialBridgeProtocol) return;

    if(message.command == (uint8_t)Command::Data){
        _parseData(message.data);
    }
}

QString SerialBridgeProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::Data : return "Data"; break;
        case Command::PortInfoReport : return "Port Info Report"; break;
        case Command::PortInfoRequest : return "Port Info Request"; break;
    }

    return "Unknown Command";
}

QString SerialBridgeProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    QString output = "Unknown Command";
    switch((Command)command){
    case Command::Data:{
        output = "Port: "+QString::number(data.at(0));
        output += " Status: "+QString::number(data.at(1),16);
        output += " Data (Hex): "+data.mid(2).toHex(' ').toUpper();
    } break;
    case Command::PortInfoReport:{
        output = "Port: "+QString::number(data.at(0));
        switch((SerialBridgeType)data.at(1)){
            case SBC_Type_UART: output += " Type: UART"; break;
            case SBC_Type_I2C: output += " Type: I2C"; break;
        }
    } break;
    case Command::PortInfoRequest: return ""; break;
    }
    return output;
}

void SerialBridgeProtocol::_parseData(QByteArray data)
{
    if(data.size() < 2) return;

    uint8_t port = static_cast<uint8_t>(data.at(0));
    sbp_status_t status = static_cast<sbp_status_t>(data.at(1));
    QByteArray rxData = data.remove(0,2);
    emit receiveData(port, status, rxData);
}
