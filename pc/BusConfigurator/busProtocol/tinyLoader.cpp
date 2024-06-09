#include "tinyLoader.h"

#include "../../QuCLib/source/crc.h"

tinyLoader::tinyLoader(busDevice *busDevice): _serialBridgeProtocol(busDevice)
{
    connect(&_busScanTimer,  &QTimer::timeout, this, &tinyLoader::on_busScanTimer);
    connect(&_serialBridgeProtocol,  &serialBridgeProtocol::receiveData, this, &tinyLoader::on_receiveData);

    connect(&_tcpBridgeServer, &QTcpServer::newConnection, this, &tinyLoader::on_tcpBridgeNewConnection);

    _devcieAddress = 0;
    _tcpBridgeCobsDecoder.clear();
}

void tinyLoader::startBusScan(uint16_t timeout)
{
    _busScanTimer.start(timeout);
    _busScanDevcieAddress = 0;

    emit busScanMessage("---- Start Scan ----", false);
}

void tinyLoader::abortBusScan()
{
    _busScanTimer.stop();
    emit busScanMessage("---- Scan Aborted ----", false);
}

void tinyLoader::writeDeviceAddress(uint8_t devcieAddress)
{
    if(devcieAddress > 0x0E) return;

    QByteArray data;
    data.append(devcieAddress);
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_SET_ADDRESS, data);
}

void tinyLoader::requestReboot(void)
{
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_REBOOT, QByteArray());
}

void tinyLoader::requestApplicationStart()
{
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_APP_START, QByteArray());
}

void tinyLoader::requestApplicationStop()
{
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_APP_STOP, QByteArray());
}

void tinyLoader::loadFirmware(QString hexPath)
{
    _hexImage.pharsHexFile(hexPath);
    emit progressChange(0);

    _hexFileInformation.applicationStartAddress = _hexImage.getOffset();
    _hexFileInformation.applicationSize = _hexImage.getSize();

    emit hexInfoChange();
}

void tinyLoader::startUpload()
{
    _appCrc16_write = QuCLib::Crc::crc16(_hexImage.binary);

    for(uint16_t i = 0; i < (_bootSystemInformation.applicationSize-_hexImage.binary.size())-2; i++)
    {
        _appCrc16_write = QuCLib::Crc::crc16_addByte(_appCrc16_write,(uint8_t)0xFF);
    }

  //  uint16_t tmp = _appCrc16;
  //  _appCrc16 = crc16_addByte(_appCrc16,(uint8_t)(tmp>>8)&0xFF);
  //  _appCrc16 = crc16_addByte(_appCrc16,(uint8_t)(tmp&0xFF));

    _eraseAppSection();
}

void tinyLoader::abortUpload()
{

}

void tinyLoader::sendCommand(uint8_t command)
{
    _sendFrame(_devcieAddress,command);
}

void tinyLoader::requestDeviceState()
{
    emit busScanMessage("---- Request System Information ----", false);
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_GET_DEVICE_STATE, QByteArray());
}

void tinyLoader::requestDeviceInformation()
{
    emit busScanMessage("---- Request Boot System Information ----", false);
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_GET_DEVICE_INFO, QByteArray());
}

void tinyLoader::requestApplicationCrc()
{
    emit busScanMessage("---- Request Application CRC ----", false);
    _sendKernelCommand(_devcieAddress,kernel_command_t::CMD_GET_APP_CRC, QByteArray());
}

void tinyLoader::setDeviceAddress(uint8_t devcieAddress)
{
    _devcieAddress = devcieAddress;
}

tinyLoader::bootSystemInformation_t tinyLoader::bootSystemInformation(void)
{
    return _bootSystemInformation;
}

tinyLoader::hexFileInformation_t tinyLoader::hexFileInformation()
{
    return _hexFileInformation;
}

void tinyLoader::tcpBridgeOpen(uint16_t port)
{
    if(!_tcpBridgeServer.listen(QHostAddress::Any, port))
    {
        emit tcpBridgeStatus("Server startup faild!",false,false);
    }
    else
    {
        emit tcpBridgeStatus("Server running. 0 active connections.",true,false);
    }
}

void tinyLoader::tcpBridgeClose()
{
    for(auto socket: _tcpBridgeSocket)
    {
        socket->flush();
        socket->close();
        disconnect(socket, &QTcpSocket::readyRead, this, &tinyLoader::on_tcpBridgeDataReceived);
    }
    _tcpBridgeSocket.clear();
    _tcpBridgeServer.close();
    emit tcpBridgeStatus("Server closed.",false,false);
}

void tinyLoader::on_tcpBridgeNewConnection(void)
{
    QTcpSocket *socket = _tcpBridgeServer.nextPendingConnection();
    socket->flush();
    connect(socket, &QTcpSocket::readyRead, this, &tinyLoader::on_tcpBridgeDataReceived);
    _tcpBridgeSocket.append(socket);

    emit tcpBridgeStatus("Server running. "+ QString::number(_tcpBridgeSocket.size()) +" active connections.",true,false);
}

void tinyLoader::on_tcpBridgeDataReceived()
{
    QByteArrayList frames = _tcpBridgeCobsDecoder.streamDecode(_tcpBridgeSocket.at(0)->readAll());// Todo: Fix this

    for(int i= 0; i< frames.length(); i++)
    {
        QByteArray frame = frames.at(i);
        _sendFrame((frame.at(0)>>4)&0x0F, frame.at(0)&0x0F, frame.mid(1,-1));
    }
}

uint16_t tinyLoader::offset()
{
    return _hexImage.getOffset();
}

uint16_t tinyLoader::size()
{
    return _hexImage.getSize();
}

void tinyLoader::on_busScanTimer()
{
    if(_busScanDevcieAddress >= 15)
    {
        _busScanTimer.stop();
        emit busScanMessage("---- Scan Finished ----", false);
        return;
    }

    emit busScanMessage("Pinging Address "+QString::number(_busScanDevcieAddress), false);

    _sendKernelCommand(_busScanDevcieAddress, kernel_command_t::CMD_GET_DEVICE_STATE, QByteArray());// Get status

    _busScanDevcieAddress++;
}

void tinyLoader::on_receiveData(uint8_t port, serialBridgeProtocol::sbp_status_t status, QByteArray data)
{
    bool error = false;
    if(status != serialBridgeProtocol::sbp_status_t::ok) error = true;

    emit busScanMessage(data.toHex().prepend("0x"),error);

    for(auto socket: _tcpBridgeSocket)
    {
        socket->write(QuCLib::Cobs::encode(data));
    }

    return;

    if(data.isEmpty()) return;
    uint8_t command = data.at(0) & 0x0F;

    if(command == 15) // Bootloader
    {
        if(data.size()<2) return;

        bool response = (bool) (data.at(1) & 0x80);
        if(!response) return;

        uint8_t subcommand = data.at(1)&0x7F;
        switch(subcommand)
        {
            case kernel_command_t::CMD_GET_DEVICE_STATE : _decodeDeviceState(data.remove(0,2)); break;
            case kernel_command_t::CMD_GET_DEVICE_INFO : _decodeDeviceInformation(data.remove(0,2)); break;
            case kernel_command_t::CMD_GET_APP_CRC  : _decodeAppCrc(data.remove(0,2)); break;

            case kernel_command_t::CMD_ERASE_APP: _writeNextPage(true); break;
            case kernel_command_t::CMD_WRITE_PAGE: _writeNextPage(false); break;
        }
    }
}

void tinyLoader::_sendFrame(uint8_t address, uint8_t command)
{
    _sendFrame(address,command,QByteArray());
}

void tinyLoader::_sendFrame(uint8_t address, uint8_t command, QByteArray data)
{
    uint8_t cmdByte = (address << 4) | (command & 0x0F);
    _serialBridgeProtocol.sendData(0,data.prepend(cmdByte));
}

void tinyLoader::_sendKernelCommand(uint8_t address, kernel_command_t command, QByteArray data)
{
    QByteArray tx;
    tx.append((uint8_t)command);
    tx.append(data);

    _sendFrame(address, 15, tx);
}
void tinyLoader::_decodeDeviceState(QByteArray data)
{
    if(data.size() != 1) return;
    _bootSystemInformation.deviceState = static_cast<device_state_t>(data.at(0)&0x0F);
    _bootSystemInformation.deviceAddress = (static_cast<uint8_t>(data.at(0))>>4)&0x0F;
    emit infoChange();
}

void tinyLoader::_decodeDeviceInformation(QByteArray data)
{
    if(data.size() != 18) return;

    _bootSystemInformation.deviceState = static_cast<device_state_t>(data.at(0)&0x0F);
    _bootSystemInformation.deviceAddress = (static_cast<uint8_t>(data.at(0))>>4)&0x0F;

    _bootSystemInformation.bootloaderRevision = static_cast<uint8_t>(data.at(1));
    _bootSystemInformation.controllerId = static_cast<uint8_t>(data.at(2));
    _bootSystemInformation.deviceId = static_cast<uint8_t>(data.at(3));
    _bootSystemInformation.deviceHardwareRevision = static_cast<uint8_t>(data.at(4));

    _bootSystemInformation.applicationStartAddress  = static_cast<uint8_t>(data.at(6));
    _bootSystemInformation.applicationStartAddress |= static_cast<uint8_t>(data.at(5))<<8;

    _bootSystemInformation.applicationSize  = static_cast<uint8_t>(data.at(8));
    _bootSystemInformation.applicationSize |= static_cast<uint8_t>(data.at(7))<<8;

    _bootSystemInformation.applicationEEPROMStartAddress  = static_cast<uint8_t>(data.at(10));
    _bootSystemInformation.applicationEEPROMStartAddress |= static_cast<uint8_t>(data.at(9))<<8;

    _bootSystemInformation.applicationEEPROMSize  = static_cast<uint8_t>(data.at(12));
    _bootSystemInformation.applicationEEPROMSize |= static_cast<uint8_t>(data.at(11))<<8;

    _bootSystemInformation.applicationRamStartAddress  = static_cast<uint8_t>(data.at(14));
    _bootSystemInformation.applicationRamStartAddress |= static_cast<uint8_t>(data.at(13))<<8;

    _bootSystemInformation.applicationRamSize  = static_cast<uint8_t>(data.at(16));
    _bootSystemInformation.applicationRamSize |= static_cast<uint8_t>(data.at(15))<<8;

    _bootSystemInformation.flashPageSize  = static_cast<uint8_t>(data.at(17));

    emit infoChange();
}

void tinyLoader::_decodeAppCrc(QByteArray data)
{
    _crc  = ((uint16_t)data.at(0))<<8;
    _crc |= ((uint16_t)data.at(1)) & 0x00FF;

    emit infoChange();
}

void tinyLoader::_eraseAppSection()
{
    emit busScanMessage("---- Erase App Section ----", false);
    _sendKernelCommand(_devcieAddress, kernel_command_t::CMD_ERASE_APP, QByteArray());
}

void tinyLoader::_writePage(uint16_t address, QByteArray data)
{
    QString msg = "---- Write Page "+QString::number(_imageAddress / _bootSystemInformation.flashPageSize)+"----";
    emit busScanMessage(msg, false);

    if(data.size() < 16)
    {
        for(int i = data.size(); i < 16; i++ )
        {
            data.append((uint8_t)0xFF);
        }
    }

    QByteArray txData;
    txData.append((uint8_t)(address>>8));
    txData.append((uint8_t)address);
    txData.append(data);
    _sendKernelCommand(_devcieAddress, kernel_command_t::CMD_WRITE_PAGE, txData);
}

void tinyLoader::_writeNextPage(bool firstPage)
{
    uint32_t flashSize = _bootSystemInformation.applicationStartAddress+_bootSystemInformation.applicationSize;

    if(firstPage)
    {
        _imageAddress = 0;
        emit progressChange(0);
    }

    if(_imageAddress < _hexImage.getSize())
    {
        emit progressChange(_imageAddress*100/_hexImage.getSize());

        _writePage(_imageAddress + _hexImage.getOffset(), _hexImage.binary.mid(_imageAddress,16));
        _imageAddress += 16;
    }
    // fill last written page
    else if(_imageAddress % _bootSystemInformation.flashPageSize != 0 && _imageAddress <  _bootSystemInformation.flashPageSize*((_hexImage.getSize()/_bootSystemInformation.flashPageSize)+1))
    {
        QByteArray data;
        for(uint8_t i = 0; i<16; i++)
        {
            data.append((uint8_t)0xFF);
        }
        _writePage(_imageAddress + _hexImage.getOffset(), data);
        _imageAddress += 16;
    }
    // scipt emty pages
    else if(_imageAddress < _bootSystemInformation.applicationSize - _bootSystemInformation.flashPageSize)
    {
        _imageAddress = (_bootSystemInformation.applicationSize - _bootSystemInformation.flashPageSize);

        QByteArray data;
        for(uint8_t i = 0; i<16; i++)
        {
            data.append((uint8_t)0xFF);
        }
        _writePage(_imageAddress + _hexImage.getOffset(), data);
        _imageAddress += 16;
    }
    // Write last flash page
    else if(_imageAddress < _bootSystemInformation.applicationSize - 16)
    {
        QByteArray data;
        for(uint8_t i = 0; i<16; i++)
        {
            data.append((uint8_t)0xFF);
        }
        _writePage(_imageAddress + _hexImage.getOffset(), data);
        _imageAddress += 16;
    }
    // send CRC in last package
    else if(_imageAddress <= _bootSystemInformation.applicationSize)
    {
        QByteArray data;
        for(uint8_t i = 0; i<14; i++)
        {
            data.append((uint8_t)0xFF);
        }
        data.append((uint8_t)(_appCrc16_write>>8));
        data.append((uint8_t)_appCrc16_write);
        _writePage(flashSize-16, data);

        _imageAddress = 0xFFFF;

        emit progressChange(100);
    }
}

uint16_t tinyLoader::crc() const
{
    return _crc;
}

QString tinyLoader::deviceStateString() const
{
    switch(_bootSystemInformation.deviceState){
    case APP_UNKNOWN: return ""; break;
    case APP_CHECK_CRC: return "Checking CRC"; break;
    case APP_CRC_ERROR: return "CRC Error"; break;
    case APP_START: return "Starting Up"; break;
    case APP_RUNNING: return "Running"; break;
    case APP_SHUTDOWN: return "Shutting Down"; break;
    case APP_STOPPED: return "Stopped"; break;
    }
}


