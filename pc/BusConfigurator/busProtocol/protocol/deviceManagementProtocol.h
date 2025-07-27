#ifndef DEVICEMANAGEMENTPROTOCOL_H
#define DEVICEMANAGEMENTPROTOCOL_H

#include "protocolBase.h"
#include "../../QuCLib/source/hexFileParser.h"

#include <QDateTime>
#include <QTimer>

class DeviceManagementProtocol : public ProtocolBase
{
    Q_OBJECT
public:
    DeviceManagementProtocol(RoomBusDevice *device);

    void handleMessage(RoomBus::Message msg) override;

    struct SystemStatus {
        uint32_t appRuning : 1;
        uint32_t appCrcError : 1;
        uint32_t appRunOnStartup : 1;
        uint32_t ledOnOff : 1;
        uint32_t identify : 1;
        uint32_t setupModeEn : 1;
        uint32_t remoteDebugger : 1;
        uint32_t systemError : 1;
        uint32_t watchdogError : 1;
        uint32_t reserved1 : 7;

        uint32_t appSpecific : 16;
    };

    union SystemControl {
        struct {
            uint32_t appRun : 1;
            uint32_t appCheckCrc : 1;
            uint32_t appRunOnStartup : 1;
            uint32_t ledOnOff : 1;
            uint32_t identify : 1;
            uint32_t reserved0 : 1;
            uint32_t remoteDebuggerEnable : 1;
            uint32_t clearError : 1;
            uint32_t rebootApp : 1;
            uint32_t rebootSystem : 1;
            uint32_t reserved1 : 6;

            uint32_t appSpecific : 16;
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
        EnterRootMode,
        SetDeviceName,
        SetAddress,

        CanDiagnosticsRequest = 0xF0,
        CanDiagnosticsReport = 0xF1,
        Echo = 0xFA,
        Reboot = 0xFB,
        EraseApplication = 0xFC,
        EraseApplicationResponse = 0xFD,
        Bootload = 0xFE,
        BootloadResponse = 0xFF
    };

    uint16_t heartbeatInterval() const;
    uint16_t systemInfoInterval() const;

    void startFirmwareUpload(QString hexPath);

    void writeBinaryChunk(void);
    void eraseApp(void);
    void appEraseComplete(void);

    void requestHeartbeat(void);
    void requestSystemInfo(void);
    void requestCanDiagnostics(void);
    void requestSystemRestart(void);

    void enterRootMode(void);
    void exitRootMode(void);

    void writeAddress(uint8_t address);
    void writeDeviceName(QString name);

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

signals:
    void bootloadStatusUpdate(uint8_t progress, bool error, QString message);
    void statusUpdate(void);

    void echoReceive(QByteArray rxData);

public slots:
    void btlRetry(void);
    void on_heartbeatTimeout(void);

private:

    QuCLib::HexFileParser _appBinary;
    uint32_t _bootloadDataIndex;

    QString _deviceName;
    QString _applicationName;
    QString _hardwareName;


    SystemStatus _sysStatus;

    uint16_t _hardwareVersion;
    uint16_t _kernelVersion;
    uint16_t _heartbeatInterval = 0;
    uint16_t _extendedHeartbeatInterval;
    uint32_t _appCRC;
    uint32_t _appStartAddress;
    uint32_t _deviceIdentificationCode;

    struct MicrocontrollerSerialNumber{
        uint32_t word0;
        uint32_t word1;
        uint32_t word2;
        uint32_t word3;
    } _serialNumber;

    bool _btlWritePending;
    RoomBus::Message _btlLastWrite;

    QTimer _btlRetryTimer;

    QDateTime _lastHeartbeat;
    QTimer _heartbeatTimer;
    bool _timeoutStatus;


};

#endif // DEVICEMANAGEMENTPROTOCOL_H
