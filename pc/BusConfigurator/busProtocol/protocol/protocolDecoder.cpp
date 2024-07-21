#include "protocolDecoder.h"


QString ProtocolDecoder::protocolName(Protocol protocol)
{
    switch(protocol){
        case DeviceManagementProtocol: return "Device Management"; break;
        case MessageLogProtocolId: return "Message Logging"; break;
        case FileTransferProtocol: return "File Transfer"; break;
        case TriggerProtocol: return "Trigger"; break;
        case EventProtocol: return "Event"; break;
        case StateReportProtocol: return "State Report"; break;
        case ValueReportProtocol: return "Value Report"; break;
        case SerialBridgeProtocol: return "Serial Bridge"; break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::commandName(Protocol protocol, int command)
{
    switch(protocol){
        case DeviceManagementProtocol: return DeviceManagementCommandlName((DeviceManagementCommand)command); break;
        case MessageLogProtocolId: return MessageLoggingCommandlName((MessageLoggingCommand)command); break;
        case FileTransferProtocol: return FileTransferCommandlName((FileTransferCommand)command); break;
        case TriggerProtocol: return TriggerCommandlName((TriggerCommand)command); break;
        case EventProtocol: return EventCommandlName((EventCommand)command); break;
        case StateReportProtocol: return StateReportCommandlName((StateReportCommand)command); break;
        case ValueReportProtocol: return ValueReportCommandlName((ValueReportCommand)command); break;
        case SerialBridgeProtocol: return SerialBridgeCommandlName((SerialBridgeCommand)command); break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::dataDecoded(Protocol protocol, int command, QByteArray data)
{
    switch(protocol){
        case DeviceManagementProtocol: return DeviceManagementDataDecoder((DeviceManagementCommand)command, data); break;
        case MessageLogProtocolId: return MessageLoggingDataDecoder((MessageLoggingCommand)command, data); break;
        case FileTransferProtocol: return FileTransferDecoder((FileTransferCommand)command, data); break;
        case TriggerProtocol: return TriggerDecoder((TriggerCommand)command, data); break;
        case EventProtocol: return EventDecoder((EventCommand)command, data); break;
        case StateReportProtocol: return StateReportDecoder((StateReportCommand)command, data); break;
        case ValueReportProtocol: return ValueReportDecoder((ValueReportCommand)command, data); break;
        case SerialBridgeProtocol: return SerialBridgeDecoder((SerialBridgeCommand)command, data); break;
    }
    return "Unknown Protocol";
}

QString ProtocolDecoder::DeviceManagementDataDecoder(DeviceManagementCommand command, QByteArray data)
{
    if(data.isEmpty()) return ""; // TODO: error handling

    DeviceManagementSubCommand subcommand = (DeviceManagementSubCommand)data.at(0);
    QString output = "Unknown Command";

    switch(subcommand){
        case DMP_SC_Heartbeat: output = "Heartbeat"; break;
        case DMP_SC_SystemInformation: output = "System Information"; break;
        case DMP_SC_HardwareName: output = "Hardware Name: "+QString(data.mid(1)) ; break;
        case DMP_SC_ApplicationName: output = "Application Name: "+QString(data.mid(1)) ; break;
        case DMP_SC_DeviceName: output = "Device Name: "+QString(data.mid(1)) ; break;
        case DMP_SC_HeartbeatRequest: output = "Heartbeat Request"; break;
        case DMP_SC_SystemInformationRequest: output = "System Information Request"; break;
        case DMP_SC_HeartbeatSettings: output = "Heartbeat Settings"; break;
        case DMP_SC_WriteControl: output = "Write Control"; break;
        case DMP_SC_SetControl: output = "Set Control"; break;
        case DMP_SC_ClearControl: output = "Clear Control"; break;
        case DMP_SC_EnterRootMode: output = "Enter Root Mode"; break;
        case DMP_SC_SetDeviceName: output = "Set Device Name"; break;
        case DMP_SC_SetAddress: output = "Set Address"; break;

        case DMP_SC_CanDiagnosticsRequest: output = "CAN Diagnostics Request"; break;
        case DMP_SC_CanDiagnosticsReport: output = "CAN Diagnostics Report"; break;
        case DMP_SC_Echo: output = "Echo"; break;
        case DMP_SC_Reboot: output = "Reboot"; break;
        case DMP_SC_EraseApplication: output = "Erase Application"; break;
        case DMP_SC_EraseApplicationResponse: output = "Erase Application Response"; break;
        case DMP_SC_Bootload: output = "Bootload"; break;
        case DMP_SC_BootloadResponse: output = "Bootload Response"; break;
    }

    return output;
}

QString ProtocolDecoder::MessageLoggingDataDecoder(MessageLoggingCommand command, QByteArray data)
{
    Q_UNUSED(command);
    return QString(data);
}

QString ProtocolDecoder::FileTransferDecoder(FileTransferCommand command, QByteArray data)
{
    return "Not implemented";
}

QString ProtocolDecoder::TriggerDecoder(TriggerCommand command, QByteArray data)
{
    return "Not implemented";
}

QString ProtocolDecoder::EventDecoder(EventCommand command, QByteArray data)
{
    return "Not implemented";
}

QString ProtocolDecoder::StateReportDecoder(StateReportCommand command, QByteArray data)
{
    return "Not implemented";
}

QString ProtocolDecoder::ValueReportDecoder(ValueReportCommand command, QByteArray data)
{
    return "Not implemented";
}

QString ProtocolDecoder::SerialBridgeDecoder(SerialBridgeCommand command, QByteArray data)
{
    QString output = "Unknown Command";
    switch(command){
    case SBC_Data:{
            output = "Port: "+QString::number(data.at(0));
            output += " Status: "+QString::number(data.at(1),16);
            output += " Data (Hex): "+data.mid(2).toHex(' ').toUpper();
        } break;
        case SBC_PortMetaReport:{
            output = "Port: "+QString::number(data.at(0));
            switch((SerialBridgeType)data.at(1)){
                case SBC_Type_UART: output += " Type: UART"; break;
                case SBC_Type_I2C: output += " Type: I2C"; break;
            }
        } break;
        case SBC_PortMetaRequest: return ""; break;
    }
    return output;
}

QString ProtocolDecoder::DeviceManagementCommandlName(DeviceManagementCommand command)
{
    switch(command){
        case DMP_DeviceToHost: return "Device To Host"; break;
        case DMP_HostToDevice: return "Host To Device"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::MessageLoggingCommandlName(MessageLoggingCommand command)
{
    switch(command){
        case MLP_SystemMessage: return "System Message"; break;
        case MLP_SystemWarning: return "System Warning"; break;
        case MLP_SystemError: return "System Error"; break;
        case MLP_ApplicationMessage: return "Application Message"; break;
        case MLP_ApplicationWarning: return "Application Warning"; break;
        case MLP_ApplicationError: return "Application Error"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::FileTransferCommandlName(FileTransferCommand command)
{
    switch(command){
        case FTP_Request: return "Request"; break;
        case FTP_Response: return "Response"; break;
        case FTP_Read: return "Read"; break;
        case FTP_ReadAcknowledgment: return "Read Acknowledgment"; break;
        case FTP_Write: return "Write"; break;
        case FTP_WriteAcknowledgment: return "Write Acknowledgment"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::TriggerCommandlName(TriggerCommand command)
{
    switch(command){
        case TP_Trigger : return "Trigger";
        case TP_Reserved0 :
        case TP_Reserved1 :
        case TP_Reserved2 : return "Reserved";

        case TP_SignalInformationReport : return "Signal Information Report";
        case TP_SlotInformationReport : return "Slot Information Report";
        case TP_SignalInformationRequest : return "Signal Information Request";
        case TP_SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::EventCommandlName(EventCommand command)
{
    switch(command){
        case EP_Event : return "Event";
        case EP_Reserved0 :
        case EP_Reserved1 :
        case EP_Reserved2 : return "Reserved";

        case EP_SignalInformationReport : return "Signal Information Report";
        case EP_SlotInformationReport : return "Slot Information Report";
        case EP_SignalInformationRequest : return "Signal Information Request";
        case EP_SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::StateReportCommandlName(StateReportCommand command)
{
    switch(command){
        case SRP_State : return "State Report";
        case SRP_StateRequest : return "State Report Request";
        case SRP_Reserved0 :
        case SRP_Reserved1 : return "Reserved";

        case SRP_SignalInformationReport : return "Signal Information Report";
        case SRP_SlotInformationReport : return "Slot Information Report";
        case SRP_SignalInformationRequest : return "Signal Information Request";
        case SRP_SlotInformationRequest : return "Slot Information Request";
    }
    return "Unknown Command";
}

QString ProtocolDecoder::ValueReportCommandlName(ValueReportCommand command)
{
    switch(command){
        case VRP_ValueReport: return "Value Report"; break;
        case VRP_ValueCommand: return "Value Command"; break;
        case VRP_MetaDataReport: return "Meta Report"; break;
        case VRP_ValueRequest: return "Value Request"; break;
        case VRP_MetaDataRequest: return "Meta Request"; break;
    }

    return "Unknown Command";
}

QString ProtocolDecoder::SerialBridgeCommandlName(SerialBridgeCommand command)
{
    switch(command){
        case SBC_Data: return "Data"; break;
        case SBC_PortMetaReport: return "Port Meta Report"; break;
        case SBC_PortMetaRequest: return "Port Meta Request"; break;
    }

    return "Unknown Command";
}

