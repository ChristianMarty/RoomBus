#ifndef BUSDEVICE_H
#define BUSDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMap>

#include "protocol/protocolBase.h"
#include "../../QuCLib/source/hexFileParser.h"

class RoomBusDevice : public QObject
{
    Q_OBJECT
public:

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


    explicit RoomBusDevice(QObject *parent = nullptr);
    explicit RoomBusDevice(uint8_t deviceAddress, QObject *parent = nullptr);

    QString deviceName() const;
    QString applicationName() const;
    QString hardwareName() const;
    uint8_t deviceAddress() const;
    void setDeviceAddress(const uint8_t &deviceAddress);

    QString kernelVersionString(void);
    QString hardwareVersionString(void);
    QString deviceIdentificationString(void);
    QString deviceSerialNumberString(void);

    void pushData(RoomBus::Message msg);
    QDateTime lastHeartbeat() const;

    void requestHeartbeat(void);
    void requestSystemInfo(void);
    void requestCanDiagnostics(void);
    void requestSystemRestart(void);

    void enterRootMode(void);
    void exitRootMode(void);

    void writeAddress(uint8_t address);
    void writeDeviceName(QString name);

    void writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval);

    void writeControl(SystemControl sysControl);
    void writeSetControl(SystemControl sysControl);
    void writeClearControl(SystemControl sysControl);

    SystemStatus sysStatus();

    bool timeoutStatus(void);

    uint16_t heartbeatInterval() const;
    uint16_t systemInfoInterval() const;

    void addProtocol(ProtocolBase* protocol);
    void removeProtocol(ProtocolBase* protocol);

    void startFirmwareUpload(QString hexPath);

    void writeBinaryChunk(void);
    void eraseApp(void);
    void appEraseComplete(void);

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

    static RoomBus::Message busScan(void);

signals:
    void dataReady(RoomBus::Message msg);
    void bootloadStatusUpdate(uint8_t progress, bool error, QString message);
    void statusUpdate(void);

    void echoReceive(QByteArray rxData);

public slots:

    void btlRetry(void);
    void heartbeatTimeout(void);

private:

    void handleDeviceManagementProtocol(RoomBus::Message msg);

    QuCLib::HexFileParser _appBinary;
    uint32_t _bootloadDataIndex;

    QList<ProtocolBase*> _protocols;

    QString _deviceName;
    QString _applicationName;
    QString _hardwareName;
    uint8_t _deviceAddress;
    QDateTime _lastHeartbeat;

    SystemStatus _sysStatus;

    uint16_t _hardwareVersion;
    uint16_t _kernelVersion;
    uint16_t _heartbeatInterval;
    uint16_t _extendedHeartbeatInterval;
    uint32_t _appCRC;
    uint32_t _appStartAddress;
    uint32_t _deviceIdentificationCode;
    uint32_t _serialNumberWord0;
    uint32_t _serialNumberWord1;
    uint32_t _serialNumberWord2;
    uint32_t _serialNumberWord3;

    bool _btlWritePending;
    RoomBus::Message _btlLastWrite;

    QTimer _btlRetryTimer;

    QTimer _heartbeatTimer;
    bool _timeoutStatus;

};

#endif // BUSDEVICE_H
