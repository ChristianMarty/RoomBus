#ifndef TINYLOADER_H
#define TINYLOADER_H

#include <QObject>
#include "hexFileParser_old.h"

#include <QTimer>
#include <QTcpSocket>
#include <QTcpServer>

#include "busDevice.h"

#include "protocol/serialBridgeProtocol.h"

#include "../../QuCLib/source/hexFileParser.h"
#include "../../QuCLib/source/cobs.h"

class tinyLoader: public QObject
{
    Q_OBJECT
public:
    tinyLoader(busDevice *busDevice);

    enum  kernel_command_t:uint8_t {
        CMD_GET_DEVICE_STATE = 0,
        CMD_GET_DEVICE_INFO,
        CMD_GET_APP_CRC,
        CMD_ERASE_APP,
        CMD_WRITE_PAGE,

        CMD_REBOOT = 0x08,
        CMD_APP_START,
        CMD_APP_STOP,

        CMD_SET_ADDRESS = 15
    };

     enum device_state_t:uint8_t {
        APP_UNKNOWN,
        APP_CHECK_CRC,
        APP_CRC_ERROR,
        APP_START,
        APP_RUNNING,
        APP_SHUTDOWN,
        APP_STOPPED
    } ;

    struct bootSystemInformation_t {
        device_state_t deviceState;
        uint8_t deviceAddress;
        uint8_t bootloaderRevision;
        uint8_t controllerId;
        uint8_t deviceId;
        uint8_t deviceHardwareRevision;
        uint16_t applicationStartAddress;
        uint16_t applicationSize;
        uint16_t applicationEEPROMStartAddress;
        uint16_t applicationEEPROMSize;
        uint16_t applicationRamStartAddress;
        uint16_t applicationRamSize;
        uint8_t flashPageSize;
    };

    struct hexFileInformation_t {
        uint16_t applicationStartAddress;
        uint16_t applicationSize;
    };

    void startBusScan(uint16_t timeout);
    void abortBusScan(void);

    void writeDeviceAddress(uint8_t devcieAddress);
    void requestReboot(void);
    void requestApplicationStart(void);
    void requestApplicationStop(void);

    void loadFirmware(QString hexPath);
    void startUpload(void);
    void abortUpload(void);

    void sendCommand(uint8_t command);

    void requestDeviceState(void);
    void requestDeviceInformation(void);

    void requestApplicationCrc(void);

    void setDeviceAddress(uint8_t devcieAddress);

    bootSystemInformation_t bootSystemInformation(void);
    hexFileInformation_t hexFileInformation(void);

    void tcpBridgeOpen(uint16_t port);
    void tcpBridgeClose(void);

    uint16_t offset();
    uint16_t size();

    enum loaderState_t {
        state_bootload,
        state_app
    }loaderState;

    uint16_t crc() const;
    QString deviceStateString() const;

signals:
    void progressChange(uint8_t progress);

    void busScanMessage(QString message, bool error);

    void infoChange(void);

    void hexInfoChange(void);

    void tcpBridgeStatus(QString message, bool open, bool error);

public slots:
    void on_busScanTimer(void);
    void on_receiveData(uint8_t port, serialBridgeProtocol::sbp_status_t status, QByteArray data);

    void on_tcpBridgeNewConnection(void);
    void on_tcpBridgeDataReceived(void);

private:

    //busDevice *_busDevice = nullptr;

     uint8_t _devcieAddress;

    targetMemory _hexImage;
    uint16_t _imageAddress;
    uint16_t _appCrc16_write;

    QTimer _busScanTimer;
    uint8_t _busScanDevcieAddress;

    QTcpServer _tcpBridgeServer;
    QList<QTcpSocket*> _tcpBridgeSocket;
    QuCLib::Cobs _tcpBridgeCobsDecoder;

    void _sendFrame(uint8_t address, uint8_t command);
    void _sendFrame(uint8_t address, uint8_t command, QByteArray data);
    void _sendKernelCommand(uint8_t address, kernel_command_t command, QByteArray data);

    serialBridgeProtocol _serialBridgeProtocol;

    void _decodeDeviceInformation(QByteArray data);
    void _decodeAppCrc(QByteArray data);
    void _decodeDeviceState(QByteArray data);

    void _eraseAppSection(void);
    void _writePage(uint16_t address, QByteArray data);

    void _writeNextPage(bool firstPage);

    bootSystemInformation_t _bootSystemInformation;
    hexFileInformation_t _hexFileInformation;
    uint16_t _crc = 0xFFFF;
};

#endif // TINYLOADER_H
