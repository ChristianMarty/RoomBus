#ifndef BUSDEVICE_H
#define BUSDEVICE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QMap>

#include "busMessage.h"
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

    typedef enum
    {
        cmd_heartbeat,
        cmd_systemInfo,
        cmd_hardwareName,
        cmd_applicationName,
        cmd_deviceName,
        cmd_heartbeatRequest,
        cmd_systemInformationRequest,
        cmd_heartbeatSettings,
        cmd_writeControl,
        cmd_setControl,
        cmd_clrControl,
        cmd_enterRootMode,
        cmd_setDeviceName,
        cmd_setAddress,

        cmd_canDiagnosticsRequest = 0xF0,
        cmd_canDiagnosticsReport = 0xF1,
        cmd_echo = 0xFA,
        cmd_reboot = 0xFB,
        cmd_eraseApp = 0xFC,
        cmd_eraseAppRsp = 0xFD,
        cmd_btl = 0xFE,
        cmd_btlRsp = 0xFF

    }command_t;

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

    void pushData(busMessage msg);
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
    void dataReady(busMessage msg);
    void bootloadStatusUpdate(uint8_t progress, bool error, QString message);
    void statusUpdate(void);

    void echoReceive(QByteArray rxData);

public slots:

    void btlRetry(void);
    void heartbeatTimeout(void);

private:

    void handleDeviceManagementProtocol(busMessage msg);

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
    busMessage _btlLastWrite;

    QTimer _btlRetryTimer;

    QTimer _heartbeatTimer;
    bool _timeoutStatus;

};

#endif // BUSDEVICE_H
