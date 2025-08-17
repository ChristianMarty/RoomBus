#ifndef DEVICEMANAGEMENTPROTOCOL_H
#define DEVICEMANAGEMENTPROTOCOL_H

#include "protocol/protocolBase.h"
#include "../../QuCLib/source/hexFileParser.h"

#include "eeprom.h"

#include <QDateTime>
#include <QTimer>

class DeviceManagementProtocol : public ProtocolBase
{
    Q_OBJECT
public:
    struct SystemStatus {
        uint32_t applicationRuning : 1;
        uint32_t applicationCrcError : 1;
        uint32_t applicationRunOnStartup : 1;
        uint32_t userLedEnabled : 1;
        uint32_t identify : 1;
        uint32_t administrationMode : 1;
        uint32_t messageLogEnabled : 1;
        uint32_t reserved0 : 1;

        uint32_t systemError : 1;
        uint32_t watchdogWarning : 1;
        uint32_t watchdogError : 1;
        uint32_t txBufferOverrun: 1;
        uint32_t txMessageOverrun: 1;
        uint32_t rxBufferOverrun: 1;
        uint32_t reserved1 : 1;
        uint32_t applicationError : 1;

        uint32_t applicationSpecific : 16;
    };

    union SystemControl {
        struct {
            uint32_t applicationRun : 1;
            uint32_t applicationCheckCrc : 1;
            uint32_t applicationRunOnStartup : 1;
            uint32_t userLedEnabled : 1;
            uint32_t identify : 1;
            uint32_t reserved0 : 1;
            uint32_t messageLogEnabled : 1;
            uint32_t reserved1 : 1;

            uint32_t clearSystemError : 1;
            uint32_t clearWatchdogWarning : 1;
            uint32_t clearWatchdogError : 1;
            uint32_t clearTxBufferOverrun : 1;
            uint32_t clearTxMessageOverrun : 1;
            uint32_t clearRxBufferOverrun : 1;
            uint32_t reserved2 : 1;
            uint32_t clearApplicationError : 1;

            uint32_t applicationSpecific : 16;
        } bit;
        uint32_t reg;
    };

    enum class DeviceManagementSubCommand {
        Heartbeat,
        SystemInformation,
        HardwareName,
        ApplicationName,
        DeviceName,
        HeartbeatRequest,
        SystemInformationRequest,
        HeartbeatSettings,

        WriteControl,
        SetControl,
        ClearControl,
        EnterAdministrationMode,
        ExitAdministrationMode,
        SetDeviceName,
        SetAddress,
        SetAdministrationModeKey,

        CanDiagnosticsRequest = 0xF0,
        CanDiagnosticsReport = 0xF1,

        EepromReadRequest = 0xF8,
        EepromReadReport = 0xF9,

        Echo = 0xFA,
        Reboot = 0xFB,
        EraseApplication = 0xFC,
        EraseApplicationResponse = 0xFD,
        Bootload = 0xFE,
        BootloadResponse = 0xFF
    };

    DeviceManagementProtocol(RoomBusDevice *device);

    void handleMessage(RoomBus::Message message) override;

    uint16_t heartbeatInterval() const;
    uint16_t systemInfoInterval() const;

    void startFirmwareUpload(QString hexPath);

    void writeBinaryChunk(void);
    void eraseApp(void);
    void handleAppEraseComplete(void);

    void requestHeartbeat(void);
    void requestSystemInfo(void);
    void requestCanDiagnostics(void);
    void requestSystemRestart(void);

    void enterAdministrationMode(QString key);
    void exitAdministrationMode(void);

    void writeAddress(uint8_t address);
    void writeDeviceName(QString name);

    void writeAdministrationModeKey(QString key);

    void writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval);

    void writeControl(DeviceManagementProtocol::SystemControl sysControl);
    void writeSetControl(DeviceManagementProtocol::SystemControl sysControl);
    void writeClearControl(DeviceManagementProtocol::SystemControl sysControl);

    void sendEcho(QByteArray txData);

    struct CanDignostics{
        uint8_t txErrorCounter;
        uint8_t rxErrorCounter;
        uint8_t errorLogCounter;
        bool receiveErrorPassive;
        enum class ErrorCode{
            NONE, STUFF, FORM, ACK, BIT1, BIT0, CRC, NC
        } lastErrorCode, dataLastErrorCode;
    } canDignostics;

    static QString getCanErrorCode(CanDignostics::ErrorCode errorCode);

    struct AppBenchmark{
        uint16_t min;
        uint16_t max;
        uint16_t avg;
    } appBenchmark;

    friend RoomBusDevice;

    void sendMessage(RoomBus::Message message);

    Eeprom &eeprom();

signals:
    void bootloadStatusUpdate(uint8_t progress, bool error, QString message);
    void diagnosticsStatusUpdate(void);
    void statusUpdate(void);

    void echoReceive(QByteArray rxData);

    void eepromDataChanged(void);

public slots:
    void on_bootloadRetry(void);
    void on_heartbeatTimeout(void);

private:
    QString _deviceName = "Unknown Device";
    QString _hardwareName = "Unknown Hardware";
    QString _applicationName = "Unknown Application";

    SystemStatus _systemStatus;

    uint16_t _hardwareVersion = 0;
    uint16_t _kernelVersion = 0;
    uint16_t _heartbeatInterval = 0;
    uint16_t _extendedHeartbeatInterval = 0;
    uint32_t _appCRC = 0;
    uint32_t _appStartAddress = 0;
    uint32_t _deviceIdentificationCode = 0;

    struct MicrocontrollerSerialNumber{
        uint32_t word0;
        uint32_t word1;
        uint32_t word2;
        uint32_t word3;
    } _serialNumber;

    QDateTime _lastMessage;

    QTimer _heartbeatTimer;
    bool _timeoutStatus;

    void _decodeSystemInformation(const QByteArray &data);
    void _decodeHeartbeat(const QByteArray &data);
    void _decodeCanDiagnosticsReport(const QByteArray &data);


    // bootloader
    QuCLib::HexFileParser _appBinary;
    uint32_t _bootloadDataIndex;
    bool _bootloadWritePending = false;
    RoomBus::Message _bootloadLastWrite;
    QTimer _bootloadRetryTimer;

    Eeprom _eeprom{this};
};

#endif // DEVICEMANAGEMENTPROTOCOL_H
