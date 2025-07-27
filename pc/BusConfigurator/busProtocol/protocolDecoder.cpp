#include "protocolDecoder.h"
#include "protocol/serialBridgeProtocol.h"
#include "busDevice.h"


QString ProtocolDecoder::protocolName(RoomBus::Protocol protocol)
{
    switch(protocol){
    case RoomBus::Protocol::DeviceManagementProtocol: return "Device Management"; break;
        case RoomBus::Protocol::MessageLogProtocolId: return "Message Logging"; break;
        case RoomBus::Protocol::FileTransferProtocol: return "File Transfer"; break;
        case RoomBus::Protocol::TriggerSystemProtocol: return "Trigger"; break;
        case RoomBus::Protocol::EventSystemProtocol: return "Event"; break;
        case RoomBus::Protocol::StateSystemProtocol: return "State Report"; break;
        case RoomBus::Protocol::ValueSystemProtocol: return "Value Report"; break;
        case RoomBus::Protocol::SerialBridgeProtocol: return "Serial Bridge"; break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::commandName(RoomBus::Protocol protocol, int command)
{
    switch(protocol){
        case RoomBus::Protocol::DeviceManagementProtocol: return DeviceManagementCommandlName((RoomBus::DeviceManagementCommand)command); break;
        case RoomBus::Protocol::MessageLogProtocolId: return MessageLoggingCommandlName((RoomBus::MessageLoggingCommand)command); break;
        case RoomBus::Protocol::FileTransferProtocol: return FileTransferCommandlName((RoomBus::FileTransferCommand)command); break;
        case RoomBus::Protocol::TriggerSystemProtocol: return TriggerCommandlName((RoomBus::TriggerSystemCommand)command); break;
        case RoomBus::Protocol::EventSystemProtocol: return EventCommandlName((RoomBus::EventSystemCommand)command); break;
        case RoomBus::Protocol::StateSystemProtocol: return StateReportCommandlName((RoomBus::StateSystemCommand)command); break;
        case RoomBus::Protocol::ValueSystemProtocol: return ValueReportCommandlName((RoomBus::ValueSystemCommand)command); break;
        case RoomBus::Protocol::SerialBridgeProtocol: return SerialBridgeCommandlName((RoomBus::SerialBridgeCommand)command); break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::dataDecoded(RoomBus::Protocol protocol, int command, const QByteArray &data)
{
    switch(protocol){
        case RoomBus::Protocol::DeviceManagementProtocol: return DeviceManagementDataDecoder((RoomBus::DeviceManagementCommand)command, data); break;
        case RoomBus::Protocol::MessageLogProtocolId: return MessageLoggingDataDecoder((RoomBus::MessageLoggingCommand)command, data); break;
        case RoomBus::Protocol::FileTransferProtocol: return FileTransferDecoder((RoomBus::FileTransferCommand)command, data); break;
        case RoomBus::Protocol::TriggerSystemProtocol: return TriggerDecoder((RoomBus::TriggerSystemCommand)command, data); break;
        case RoomBus::Protocol::EventSystemProtocol: return EventDecoder((RoomBus::EventSystemCommand)command, data); break;
        case RoomBus::Protocol::StateSystemProtocol: return StateReportDecoder((RoomBus::StateSystemCommand)command, data); break;
        case RoomBus::Protocol::ValueSystemProtocol: return ValueReportDecoder((RoomBus::ValueSystemCommand)command, data); break;
        case RoomBus::Protocol::SerialBridgeProtocol: return SerialBridgeDecoder((RoomBus::SerialBridgeCommand)command, data); break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::DeviceManagementDataDecoder(RoomBus::DeviceManagementCommand command, const QByteArray &data)
{
    if(data.isEmpty()) return ""; // TODO: error handling

    DeviceManagementProtocol::DeviceManagementSubCommand subCommand = (DeviceManagementProtocol::DeviceManagementSubCommand)((uint8_t)data.at(0));
    QString output = "Unknown Command";

    switch(subCommand){
        case DeviceManagementProtocol::DeviceManagementSubCommand::Heartbeat: output = "Heartbeat"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::SystemInformation: output = "System Information"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::HardwareName: output = "Hardware Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::ApplicationName: output = "Application Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::DeviceName: output = "Device Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::HeartbeatRequest: output = "Heartbeat Request"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::SystemInformationRequest: output = "System Information Request"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::HeartbeatSettings: output = "Heartbeat Settings"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::WriteControl: output = "Write Control"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::SetControl: output = "Set Control"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::ClearControl: output = "Clear Control"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::EnterRootMode: output = "Enter Root Mode"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::SetDeviceName: output = "Set Device Name"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::SetAddress: output = "Set Address"; break;

        case DeviceManagementProtocol::DeviceManagementSubCommand::CanDiagnosticsRequest: output = "CAN Diagnostics Request"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::CanDiagnosticsReport: output = "CAN Diagnostics Report"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::Echo: output = "Echo"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::Reboot: output = "Reboot"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::EraseApplication: output = "Erase Application"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::EraseApplicationResponse: output = "Erase Application Response"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::Bootload: output = "Bootload"; break;
        case DeviceManagementProtocol::DeviceManagementSubCommand::BootloadResponse: output = "Bootload Response"; break;
    }

    return output;
}

QString ProtocolDecoder::MessageLoggingDataDecoder(RoomBus::MessageLoggingCommand command, const QByteArray &data)
{
    Q_UNUSED(command);
    return QString(data);
}

QString ProtocolDecoder::FileTransferDecoder(RoomBus::FileTransferCommand command, const QByteArray &data)
{
    return "Not implemented";
}

QString ProtocolDecoder::TriggerDecoder(RoomBus::TriggerSystemCommand command, const QByteArray &data)
{
    return "Not implemented";
}

QString ProtocolDecoder::EventDecoder(RoomBus::EventSystemCommand command, const QByteArray &data)
{
    return "Not implemented";
}

QString ProtocolDecoder::StateReportDecoder(RoomBus::StateSystemCommand command, const QByteArray &data)
{
    return "Not implemented";
}

QString ProtocolDecoder::ValueReportDecoder(RoomBus::ValueSystemCommand command, const QByteArray &data)
{
    return "Not implemented";
}

QString ProtocolDecoder::SerialBridgeDecoder(RoomBus::SerialBridgeCommand command, const QByteArray &data)
{
    QString output = "Unknown Command";
    switch(command){
    case RoomBus::SerialBridgeCommand::Data:{
            output = "Port: "+QString::number(data.at(0));
            output += " Status: "+QString::number(data.at(1),16);
            output += " Data (Hex): "+data.mid(2).toHex(' ').toUpper();
        } break;
    case RoomBus::SerialBridgeCommand::PortInfoReport:{
            output = "Port: "+QString::number(data.at(0));
            switch((SerialBridgeProtocol::SerialBridgeType)data.at(1)){
                case SerialBridgeProtocol::SBC_Type_UART: output += " Type: UART"; break;
                case SerialBridgeProtocol::SBC_Type_I2C: output += " Type: I2C"; break;
            }
        } break;
    case RoomBus::SerialBridgeCommand::PortInfoRequest: return ""; break;
    }
    return output;
}

QString ProtocolDecoder::DeviceManagementCommandlName(RoomBus::DeviceManagementCommand command)
{
    switch(command){
        case RoomBus::DeviceManagementCommand::DeviceToHost: return "Device To Host"; break;
        case RoomBus::DeviceManagementCommand::HostToDevice: return "Host To Device"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::MessageLoggingCommandlName(RoomBus::MessageLoggingCommand command)
{
    switch(command){
        case RoomBus::MessageLoggingCommand::SystemMessage: return "System Message"; break;
        case RoomBus::MessageLoggingCommand::SystemWarning: return "System Warning"; break;
        case RoomBus::MessageLoggingCommand::SystemError: return "System Error"; break;
        case RoomBus::MessageLoggingCommand::ApplicationMessage: return "Application Message"; break;
        case RoomBus::MessageLoggingCommand::ApplicationWarning: return "Application Warning"; break;
        case RoomBus::MessageLoggingCommand::ApplicationError: return "Application Error"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::FileTransferCommandlName(RoomBus::FileTransferCommand command)
{
    switch(command){
        case RoomBus::FileTransferCommand::Request: return "Request"; break;
        case RoomBus::FileTransferCommand::Response: return "Response"; break;
        case RoomBus::FileTransferCommand::Read: return "Read"; break;
        case RoomBus::FileTransferCommand::ReadAcknowledgment: return "Read Acknowledgment"; break;
        case RoomBus::FileTransferCommand::Write: return "Write"; break;
        case RoomBus::FileTransferCommand::WriteAcknowledgment: return "Write Acknowledgment"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::TriggerCommandlName(RoomBus::TriggerSystemCommand command)
{
    switch(command){
        case RoomBus::TriggerSystemCommand::Trigger : return "Trigger";
        case RoomBus::TriggerSystemCommand::Reserved0 :
        case RoomBus::TriggerSystemCommand::Reserved1 :
        case RoomBus::TriggerSystemCommand::Reserved2 : return "Reserved";

        case RoomBus::TriggerSystemCommand::SignalInformationReport : return "Signal Information Report";
        case RoomBus::TriggerSystemCommand::SlotInformationReport : return "Slot Information Report";
        case RoomBus::TriggerSystemCommand::SignalInformationRequest : return "Signal Information Request";
        case RoomBus::TriggerSystemCommand::SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::EventCommandlName(RoomBus::EventSystemCommand command)
{
    switch(command){
        case RoomBus::EventSystemCommand::Event : return "Event";
        case RoomBus::EventSystemCommand::Reserved0 :
        case RoomBus::EventSystemCommand::Reserved1 :
        case RoomBus::EventSystemCommand::Reserved2 : return "Reserved";

        case RoomBus::EventSystemCommand::SignalInformationReport : return "Signal Information Report";
        case RoomBus::EventSystemCommand::SlotInformationReport : return "Slot Information Report";
        case RoomBus::EventSystemCommand::SignalInformationRequest : return "Signal Information Request";
        case RoomBus::EventSystemCommand::SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::StateReportCommandlName(RoomBus::StateSystemCommand command)
{
    switch(command){
        case RoomBus::StateSystemCommand::State : return "State Report";
        case RoomBus::StateSystemCommand::StateRequest : return "State Report Request";
        case RoomBus::StateSystemCommand::Reserved0 :
        case RoomBus::StateSystemCommand::Reserved1 : return "Reserved";

        case RoomBus::StateSystemCommand::SignalInformationReport : return "Signal Information Report";
        case RoomBus::StateSystemCommand::SlotInformationReport : return "Slot Information Report";
        case RoomBus::StateSystemCommand::SignalInformationRequest : return "Signal Information Request";
        case RoomBus::StateSystemCommand::SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::ValueReportCommandlName(RoomBus::ValueSystemCommand command)
{
    switch(command){
        case RoomBus::ValueSystemCommand::ValueReport: return "Value Report"; break;
        case RoomBus::ValueSystemCommand::ValueRequest: return "Value Request"; break;
        case RoomBus::ValueSystemCommand::ValueCommand: return "Value Command"; break;
        case RoomBus::ValueSystemCommand::Reserved0 : return "Reserved";

        case RoomBus::ValueSystemCommand::SignalInformationReport : return "Signal Information Report";
        case RoomBus::ValueSystemCommand::SlotInformationReport : return "Slot Information Report";
        case RoomBus::ValueSystemCommand::SignalInformationRequest : return "Signal Information Request";
        case RoomBus::ValueSystemCommand::SlotInformationRequest : return "Slot Information Request";
    }

    return "Unknown Command";
}

QString ProtocolDecoder::SerialBridgeCommandlName(RoomBus::SerialBridgeCommand command)
{
    switch(command){
        case RoomBus::SerialBridgeCommand::Data : return "Data"; break;
        case RoomBus::SerialBridgeCommand::PortInfoReport : return "Port Info Report"; break;
        case RoomBus::SerialBridgeCommand::PortInfoRequest : return "Port Info Request"; break;
    }

    return "Unknown Command";
}

