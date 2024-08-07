#ifndef PROTOCOL_H
#define PROTOCOL_H



enum Protocol {
    DeviceManagementProtocol = 0,
    MessageLogProtocolId = 1,
    FileTransferProtocol = 2,

    TriggerProtocol = 8,
    EventProtocol = 9,
    StateReportProtocol = 10,
    ValueReportProtocol = 11,
    SerialBridgeProtocol = 12
};

enum DeviceManagementCommand {
    DMP_DeviceToHost,
    DMP_HostToDevice
};

enum DeviceManagementSubCommand {
    DMP_SC_Heartbeat,
    DMP_SC_SystemInformation,
    DMP_SC_HardwareName,
    DMP_SC_ApplicationName,
    DMP_SC_DeviceName,
    DMP_SC_HeartbeatRequest,
    DMP_SC_SystemInformationRequest,
    DMP_SC_HeartbeatSettings,
    DMP_SC_WriteControl,
    DMP_SC_SetControl,
    DMP_SC_ClearControl,
    DMP_SC_EnterRootMode,
    DMP_SC_SetDeviceName,
    DMP_SC_SetAddress,

    DMP_SC_CanDiagnosticsRequest = 0xF0,
    DMP_SC_CanDiagnosticsReport = 0xF1,
    DMP_SC_Echo = 0xFA,
    DMP_SC_Reboot = 0xFB,
    DMP_SC_EraseApplication = 0xFC,
    DMP_SC_EraseApplicationResponse = 0xFD,
    DMP_SC_Bootload = 0xFE,
    DMP_SC_BootloadResponse = 0xFF
};

enum MessageLoggingCommand {
    MLP_SystemMessage = 0x00,
    MLP_SystemWarning = 0x01,
    MLP_SystemError = 0x02,

    MLP_ApplicationMessage = 0x04,
    MLP_ApplicationWarning = 0x05,
    MLP_ApplicationError = 0x06
};

enum FileTransferCommand {
    FTP_Request = 0x00,
    FTP_Response = 0x01,

    FTP_Read = 0x04,
    FTP_ReadAcknowledgment = 0x05,
    FTP_Write = 0x06,
    FTP_WriteAcknowledgment = 0x07
};

enum TriggerCommand {
    TP_Trigger,
    TP_Reserved0,
    TP_Reserved1,
    TP_Reserved2,

    TP_SignalInformationReport,
    TP_SlotInformationReport,
    TP_SignalInformationRequest,
    TP_SlotInformationRequest
};

enum EventCommand {
    EP_Event,
    EP_Reserved0,
    EP_Reserved1,
    EP_Reserved2,

    EP_SignalInformationReport,
    EP_SlotInformationReport,
    EP_SignalInformationRequest,
    EP_SlotInformationRequest
};

enum StateReportCommand {
    SRP_State,
    SRP_StateRequest,
    SRP_Reserved0,
    SRP_Reserved1,

    SRP_SignalInformationReport,
    SRP_SlotInformationReport,
    SRP_SignalInformationRequest,
    SRP_SlotInformationRequest
};

enum ValueReportCommand {
    VRP_ValueReport = 0x00,
    VRP_ValueCommand = 0x01,
    VRP_MetaDataReport = 0x02,
    VRP_ValueRequest = 0x03,
    VRP_MetaDataRequest= 0x04
};

enum SerialBridgeCommand {
    SBC_Data = 0x00,

    SBC_PortMetaReport = 0x04,
    SBC_PortMetaRequest = 0x05
};

enum SerialBridgeType {
    SBC_Type_UART = 0x00,
    SBC_Type_I2C = 0x01
};

#endif // PROTOCOL_H
