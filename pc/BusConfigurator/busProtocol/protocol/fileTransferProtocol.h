#ifndef FILETRANSFERPROTOCOL_H
#define FILETRANSFERPROTOCOL_H

#include <QObject>
#include <QMap>
#include <QDateTime>
#include <QFile>

#include "busProtocol.h"


class fileTransferProtocol : public BusProtocol
{
    Q_OBJECT
public:

    enum commands_t{
        cmd_request  = 0,
        cmd_response = 1,

        cmd_read = 4,
        cmd_read_ack = 5,
        cmd_write = 6,
        cmd_write_ack = 7
    };

    enum reqest_t{
        req_list = 0,
        req_makeFile = 8,
        req_deleteFile = 9,

        req_startFileRead = 0x20,
        req_endFileRead = 0x21,

        req_startFileWrite = 0x30,
        req_endFileWrite = 0x31,
        req_completeWrite = 0x32
    };

    enum response_t{
        rsp_fileInfo = 0,

        rsp_startFileRead = 0x20,

        rsp_startFileWrite = 0x30,
        rsp_completeWrite = 0x31
    };

    enum responseStatus_t{
        rsps_ok = 0,
        rsps_writeSuccessful = 0x20,
        rsps_writeCrcError = 0x21,
        rsps_writeSequencingError = 0x22,

        rsps_readSuccessful = 0x30,
        rsps_readCrcError = 0x31,
        rsps_readSequencingError = 0x32
    };

    enum transfereStatus_t{
        idel,
        start,
        transfere,
        verify,
        complete,
        error
    };

    struct file_t {
        enum{file = 0, dir = 1, error=0xFF} type;
        uint32_t size;
        QString name;
    };

    fileTransferProtocol(busDevice *device);

    void pushData(BusMessage msg);
    QList<Protocol> protocol(void);

    void list(QString path);
    void makeFile(QString path);
    void deleteFile(QString path);

    void writeFile(QString path, QString localPath);
    void readFile(QString path, QString localPath);

signals:

    void fileList_change(QMap<QString, file_t> files);

    void writeTransfereStatus_change(transfereStatus_t status, uint8_t progress);
    void readTransfereStatus_change(transfereStatus_t status, uint8_t progress);

private:

    QMap<QString, file_t> _files;

    struct transfer_t
    {
        QByteArray data;
        uint32_t size;
        uint32_t offset;
        uint32_t crc;

        QString localPath;
        QString remotePath;

        transfereStatus_t status;
        uint8_t progress;

    }_fromDeviceTransfer, _toDeviceTransfer;


    void handle_readStart(BusMessage msg);
    void handle_read(BusMessage msg);
    void handle_readEnd(BusMessage msg);

    // Wite functions
    void handle_writeStart(BusMessage msg);
    void handle_writeAck(BusMessage msg);
    void handle_writeComplete(BusMessage msg);


};

#endif // FILETRANSFERPROTOCOL_H
