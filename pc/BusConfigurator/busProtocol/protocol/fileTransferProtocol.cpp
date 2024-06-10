#include "fileTransferProtocol.h"
#include "../../QuCLib/source/crc.h"

fileTransferProtocol::fileTransferProtocol(busDevice *device):BusProtocol(device)
{
}

void fileTransferProtocol::pushData(busMessage msg)
{
    if(msg.protocol != Protocol::FileTransferProtocol) return;

    if(msg.command == cmd_response)
    {
        uint8_t response_id = msg.data.at(0);
        switch(response_id)
        {
            case rsp_fileInfo:
            {
                file_t file;
                if(msg.data.at(2) == 0) file.type = file_t::file;
                else if(msg.data.at(2) == 1) file.type = file_t::dir;
                else file.type = file_t::error;

                file.size = getUint32(msg.data,3);
                file.name = msg.data.remove(0,7);

                _files[file.name] = file;
                emit fileList_change(_files);
            }
            break;


            case rsp_startFileRead:  handle_readStart(msg); break;

            case rsp_startFileWrite: handle_writeStart(msg); break;
            case rsp_completeWrite:  handle_writeComplete(msg); break;
        }
    }
    else if(msg.command == cmd_request)
    {
        uint8_t response_id = msg.data.at(0);
        switch(response_id)
        {
            case req_endFileRead:  handle_readEnd(msg); break;
        }
    }
    else if(msg.command == cmd_read)
    {
        handle_read(msg);
    }
    else if(msg.command == cmd_read_ack)
    {

    }
    else if(msg.command == cmd_write)
    {
        // This should not happen
    }
    else if(msg.command == cmd_write_ack)
    {
        handle_writeAck(msg);

    }
}



QList<Protocol> fileTransferProtocol::protocol(void)
{
    QList<Protocol> temp;
    temp.append(Protocol::FileTransferProtocol);
    return temp;
}

void fileTransferProtocol::list(QString path)
{
    _files.clear();
    busMessage msg;

    msg.protocol = Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(fileTransferProtocol::cmd_request);
    msg.data.append(static_cast<char>(fileTransferProtocol::req_list));

    sendMessage(msg);
}

void fileTransferProtocol::makeFile(QString path)
{
    busMessage msg;

    msg.protocol = Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(fileTransferProtocol::cmd_request);
    msg.data.append(static_cast<char>(fileTransferProtocol::req_makeFile));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

void fileTransferProtocol::deleteFile(QString path)
{
    busMessage msg;

    msg.protocol = Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(fileTransferProtocol::cmd_request);
    msg.data.append(static_cast<char>(fileTransferProtocol::req_deleteFile));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}


/*****************************************************************************
 * File Read
 * ***************************************************************************/
void fileTransferProtocol::readFile(QString path, QString localPath)
{

    _fromDeviceTransfer.progress = 0;

    _fromDeviceTransfer.offset = 0;
    _fromDeviceTransfer.data.clear();

    _fromDeviceTransfer.remotePath = path;
    _fromDeviceTransfer.localPath = localPath;

    busMessage msg;

    msg.protocol = Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(fileTransferProtocol::cmd_request);
    msg.data.append(static_cast<char>(fileTransferProtocol::req_startFileRead));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

void fileTransferProtocol::handle_readStart(busMessage msg)
{
    _fromDeviceTransfer.status = start;
    _fromDeviceTransfer.crc = getUint32(msg.data,1);
    _fromDeviceTransfer.size = getUint32(msg.data,5);

    emit readTransfereStatus_change(_fromDeviceTransfer.status, _fromDeviceTransfer.progress);

    busMessage txMsg;
    txMsg.protocol = Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(fileTransferProtocol::cmd_read_ack);
    txMsg.data.append(static_cast<char>(rsps_ok));
    sendMessage(txMsg);
}

void fileTransferProtocol::handle_read(busMessage msg)
{
    busMessage txMsg;
    txMsg.protocol = Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(fileTransferProtocol::cmd_read_ack);

    uint32_t offset = getUint32(msg.data,0);

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

void fileTransferProtocol::handle_readEnd(busMessage msg)
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
void fileTransferProtocol::writeFile(QString path, QString localPath)
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

    busMessage msg;

    msg.protocol = Protocol::FileTransferProtocol;
    msg.command = static_cast<char>(fileTransferProtocol::cmd_request);
    msg.data.append(static_cast<char>(fileTransferProtocol::req_startFileWrite));
    msg.data.append(packUint32(_toDeviceTransfer.crc));
    msg.data.append(packUint32(_toDeviceTransfer.size));
    msg.data.append(path.toUtf8());

    sendMessage(msg);
}

void fileTransferProtocol::handle_writeStart(busMessage msg)
{
    // Write first part of data
    busMessage txMsg;

    txMsg.protocol = Protocol::FileTransferProtocol;
    txMsg.command = static_cast<char>(fileTransferProtocol::cmd_write);
    txMsg.data.append(packUint32(_toDeviceTransfer.offset));
    txMsg.data.append(_toDeviceTransfer.data.mid(_toDeviceTransfer.offset,56));

    sendMessage(txMsg);

    _toDeviceTransfer.offset += (txMsg.data.size()-4);

    _toDeviceTransfer.status = start;
    _toDeviceTransfer.progress = 0;
    emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);
}

void fileTransferProtocol::handle_writeAck(busMessage msg)
{
    uint8_t status = msg.data.at(0);

    if(status == rsps_ok)
    {
        if(_toDeviceTransfer.offset < _toDeviceTransfer.size)
        {
            _toDeviceTransfer.status = transfere;
            _toDeviceTransfer.progress = (uint8_t)(((float)_toDeviceTransfer.offset/(float)_toDeviceTransfer.size)*100);

            busMessage txMsg;

            txMsg.protocol = Protocol::FileTransferProtocol;
            txMsg.command = static_cast<char>(fileTransferProtocol::cmd_write);
            txMsg.data.append(packUint32(_toDeviceTransfer.offset));
            txMsg.data.append(_toDeviceTransfer.data.mid(_toDeviceTransfer.offset,56));

            sendMessage(txMsg);

            _toDeviceTransfer.offset += (txMsg.data.size()-4);

        }
        else
        {
            _toDeviceTransfer.status = verify;
            _toDeviceTransfer.progress = 100;

            busMessage txMsg;

            txMsg.protocol = Protocol::FileTransferProtocol;
            txMsg.command = static_cast<char>(fileTransferProtocol::cmd_request);
            txMsg.data.append(static_cast<char>(fileTransferProtocol::req_endFileWrite));

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

void fileTransferProtocol::handle_writeComplete(busMessage msg)
{
    if(msg.data.at(1) ==  rsps_writeSuccessful) _toDeviceTransfer.status = complete;
    else _toDeviceTransfer.status = error;

    _toDeviceTransfer.progress = 100;
    emit writeTransfereStatus_change(_toDeviceTransfer.status, _toDeviceTransfer.progress);

    _toDeviceTransfer.data.clear();
}


