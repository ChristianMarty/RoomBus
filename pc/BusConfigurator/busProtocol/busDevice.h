#ifndef BUSDEVICE_H
#define BUSDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMap>

#include "busProtocol.h"
#include "../../QuCLib/source/hexFileParser.h"


class busDevice : public QObject
{
    Q_OBJECT
public:

    typedef struct {
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
    } sysStatus_t;

    typedef union {
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
        }bit;
        uint32_t reg;
     } sysControl_t;

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


    explicit busDevice(QObject *parent = nullptr);
    explicit busDevice(uint8_t deviceAddress, QObject *parent = nullptr);

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

    void writeControl(sysControl_t sysControl);
    void writeSetControl(sysControl_t sysControl);
    void writeClearControl(sysControl_t sysControl);

    sysStatus_t sysStatus();

    bool timeoutStatus(void);

    uint16_t heartbeatInterval() const;
    uint16_t systemInfoInterval() const;

    void addProtocol(BusProtocol* protocol);
    void removeProtocol(BusProtocol* protocol);

    void startFirmwareUpload(QString hexPath);

    void writeBinaryChunk(void);
    void eraseApp(void);
    void appEraseComplete(void);

    void sendEcho(QByteArray txData);

    struct canDignostics_t
    {
        uint8_t txErrorCounter;
        uint8_t rxErrorCounter;
        uint8_t errorLogCounter;
        bool receiveErrorPassive;
        enum errorCode_t{
            NONE, STUFF, FORM, ACK, BIT1, BIT0, CRC, NC
        } lastErrorCode, dataLastErrorCode;
    } canDignostics;

    static QString getCanErrorCode(canDignostics_t::errorCode_t errorCode);

    struct appBenchmark_t
    {
        uint16_t min;
        uint16_t max;
        uint16_t avg;
    }appBenchmark;

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

    QList< BusProtocol* > _protocols;

    QString _deviceName;
    QString _applicationName;
    QString _hardwareName;
    uint8_t _deviceAddress;
    QDateTime _lastHeartbeat;

    sysStatus_t _sysStatus;

    uint16_t _hwVersion;
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
