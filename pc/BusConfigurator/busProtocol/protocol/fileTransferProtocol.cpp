#include "fileTransferProtocol.h"
#include "../../QuCLib/source/crc.h"
#include "busDevice.h"

FileTransferProtocol::FileTransferProtocol(RoomBusDevice *device):ProtocolBase(device)
{
    _device->addProtocol(this);
}

void FileTransferProtocol::handleMessage(const MiniBus::Message &message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::FileTransferProtocol){
        return;
    }

    Command command = (Command)message.command;

    if(command == Command::Response)
    {
        uint8_t response_id = message.data.at(0);
        switch(response_id)
        {
            case rsp_fileInfo:
            {
                file_t file;
                if(message.data.at(2) == 0) file.type = file_t::file;
                else if(message.data.at(2) == 1) file.type = file_t::dir;
                else file.type = file_t::error;

                file.size = MiniBus::unpackUint32(message.data,3);
                file.name = QByteArray(message.data).remove(0,7);

                _files[file.name] = file;
                emit fileList_change(_files);
            }
            break;


            case rsp_startFileRead:  handle_readStart(message); break;

            case rsp_startFileWrite: handle_writeStart(message); break;
            case rsp_completeWrite:  handle_writeComplete(message); break;
        }
    }
    else if(command == Command::Request)
    {
        uint8_t response_id = message.data.at(0);
        switch(response_id)
        {
            case req_endFileRead:  handle_readEnd(message); break;
        }
    }
    else if(command == Command::Read)
    {
        handle_read(message);
    }
    else if(command == Command::ReadAcknowledgment)
    {

    }
    else if(command == Command::Write)
    {
        // This should not happen
    }
    else if(command == Command::WriteAcknowledgment)
    {
        handle_writeAck(message);

    }
}

void FileTransferProtocol::list(QString path)
{
    _files.clear();
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(Command::Request);
    msg.data.append(static_cast<char>(FileTransferProtocol::req_list));

    sendMessage(msg);
}

void FileTransferProtocol::makeFile(QString path)
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(Command::Request);
    msg.data.append(static_cast<char>(FileTransferProtocol::req_makeFile));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

void FileTransferProtocol::deleteFile(QString path)
{
    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(Command::Request);
    msg.data.append(static_cast<char>(FileTransferProtocol::req_deleteFile));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}


/*****************************************************************************
 * File Read
 * ***************************************************************************/
void FileTransferProtocol::readFile(QString path, QString localPath)
{

    _fromDeviceTransfer.progress = 0;

    _fromDeviceTransfer.offset = 0;
    _fromDeviceTransfer.data.clear();

    _fromDeviceTransfer.remotePath = path;
    _fromDeviceTransfer.localPath = localPath;

    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(Command::Request);
    msg.data.append(static_cast<char>(FileTransferProtocol::req_startFileRead));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

QString FileTransferProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::Request: return "Request"; break;
        case Command::Response: return "Response"; break;
        case Command::Read: return "Read"; break;
        case Command::ReadAcknowledgment: return "Read Acknowledgment"; break;
        case Command::Write: return "Write"; break;
        case Command::WriteAcknowledgment: return "Write Acknowledgment"; break;
    }

    return "Unknown Command";
}

QString FileTransferProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    Q_UNUSED(command);
    Q_UNUSED(data);
    return "Not implemented";
}

void FileTransferProtocol::handle_readStart(MiniBus::Message msg)
{
    _fromDeviceTransfer.status = start;
    _fromDeviceTransfer.crc = MiniBus::unpackUint32(msg.data,1);
    _fromDeviceTransfer.size = MiniBus::unpackUint32(msg.data,5);

    emit readTransfereStatus_change(_fromDeviceTransfer.status, _fromDeviceTransfer.progress);

    MiniBus::Message txMsg;
    txMsg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(Command::ReadAcknowledgment);
    txMsg.data.append(static_cast<char>(rsps_ok));
    sendMessage(txMsg);
}

void FileTransferProtocol::handle_read(MiniBus::Message msg)
{
    MiniBus::Message txMsg;
    txMsg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(Command::ReadAcknowledgment);

    uint32_t offset = MiniBus::unpackUint32(msg.data,0);

    if(offset == _fromDeviceTransfer.offset)
    {
        txMsg.data.append(static_cast<char>(rsps_ok));

        _fromDeviceTransfer.data.append(msg.data.remove(0,4));
        _fromDeviceTransfer.offset += msg.data.size();
        _fromDeviceTransfer.status = transfere;
    }
    else
    {
        txMsg.data.append(static_cast<char>(rsps_readSequencingError));
        _fromDeviceTransfer.status = error;
    }
    sendMessage(txMsg);

    _fromDeviceTransfer.progress = (uint8_t)(((float)_fromDeviceTransfer.offset/(float)_fromDeviceTransfer.size)*100);
    emit readTransfereStatus_change(_fromDeviceTransfer.status, _fromDeviceTransfer.progress);
}

void FileTransferProtocol::handle_readEnd(MiniBus::Message msg)
{
    uint32_t crc = QuCLib::Crc::crc32(_fromDeviceTransfer.data);

    if(_fromDeviceTransfer.crc == crc)
    {
        QFile file(_fromDeviceTransfer.localPath);
        file.open(QIODevice::WriteOnly);
        file.write(_fromDeviceTransfer.data);
        file.close();
        _fromDeviceTransfer.status = complete;
    }
    else
    {
        _fromDeviceTransfer.status = error;
    }

    _fromDeviceTransfer.progress = 100;
    emit readTransfereStatus_change(_fromDeviceTransfer.status, _fromDeviceTransfer.progress);
}

/*****************************************************************************
 * File Write
 * ***************************************************************************/
void FileTransferProtocol::writeFile(QString path, QString localPath)
{

    _toDeviceTransfer.offset = 0;
    _toDeviceTransfer.data.clear();
    _toDeviceTransfer.remotePath = path;
    _toDeviceTransfer.localPath = localPath;

    QFile file(_toDeviceTransfer.localPath);
    file.open(QIODevice::ReadOnly);
    _toDeviceTransfer.data.append(file.readAll());
    file.close();

    _toDeviceTransfer.size = static_cast<uint32_t>(_toDeviceTransfer.data.size());

    _toDeviceTransfer.crc = QuCLib::Crc::crc32(_toDeviceTransfer.data);

    MiniBus::Message msg;

    msg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(Command::Request);
    msg.data.append(static_cast<char>(FileTransferProtocol::req_startFileWrite));
    msg.data.append(MiniBus::packUint32(_toDeviceTransfer.crc));
    msg.data.append(MiniBus::packUint32(_toDeviceTransfer.size));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

void FileTransferProtocol::handle_writeStart(MiniBus::Message msg)
{
    // Write first part of data
    MiniBus::Message txMsg;

    txMsg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(Command::Write);
    txMsg.data.append(MiniBus::packUint32(_toDeviceTransfer.offset));
    txMsg.data.append(_toDeviceTransfer.data.mid(_toDeviceTransfer.offset,56));

    sendMessage(txMsg);

    _toDeviceTransfer.offset += (txMsg.data.size()-4);

    _toDeviceTransfer.status = start;
    _toDeviceTransfer.progress = 0;
    emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);
}

void FileTransferProtocol::handle_writeAck(MiniBus::Message msg)
{
    uint8_t status = msg.data.at(0);

    if(status == rsps_ok)
    {
        if(_toDeviceTransfer.offset < _toDeviceTransfer.size)
        {
            _toDeviceTransfer.status = transfere;
            _toDeviceTransfer.progress = (uint8_t)(((float)_toDeviceTransfer.offset/(float)_toDeviceTransfer.size)*100);

            MiniBus::Message txMsg;

            txMsg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
            txMsg.command = static_cast<char>(Command::Write);
            txMsg.data.append(MiniBus::packUint32(_toDeviceTransfer.offset));
            txMsg.data.append(_toDeviceTransfer.data.mid(_toDeviceTransfer.offset,56));

            sendMessage(txMsg);

            _toDeviceTransfer.offset += (txMsg.data.size()-4);

        }
        else
        {
            _toDeviceTransfer.status = verify;
            _toDeviceTransfer.progress = 100;

            MiniBus::Message txMsg;

            txMsg.protocol = (MiniBus::Protocol)Protocol::FileTransferProtocol;
            txMsg.command = static_cast<char>(Command::Request);
            txMsg.data.append(static_cast<char>(FileTransferProtocol::req_endFileWrite));

            sendMessage(txMsg);
        }

        emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);
    }
    else if(status == rsps_writeSequencingError)
    {
        // todo: resolve
    }
    else
    {
        _toDeviceTransfer.status = error;
        emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);
    }
}

void FileTransferProtocol::handle_writeComplete(MiniBus::Message msg)
{
    if(msg.data.at(1) ==  rsps_writeSuccessful) _toDeviceTransfer.status = complete;
    else _toDeviceTransfer.status = error;

    _toDeviceTransfer.progress = 100;
    emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);

    _toDeviceTransfer.data.clear();
}


