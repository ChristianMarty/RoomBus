#include "deviceManagementProtocol.h"
#include "../../QuCLib/source/crc.h"

DeviceManagementProtocol::DeviceManagementProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    connect(&_bootloadRetryTimer, &QTimer::timeout, this, &DeviceManagementProtocol::on_bootloadRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &DeviceManagementProtocol::on_heartbeatTimeout);
}

void DeviceManagementProtocol::handleMessage(RoomBus::Message message)
{
    if(message.protocol != RoomBus::Protocol::DeviceManagementProtocol) return;

    _lastMessage = QDateTime::currentDateTime();

    if(message.data.size() == 0) return;

    DeviceManagementSubCommand subCommand = (DeviceManagementSubCommand)((uint8_t)message.data.at(0));
    QByteArray data = message.data.mid(1);

    switch(subCommand){
        case DeviceManagementSubCommand::SystemInformation:
            _decodeSystemInformation(data);
            break;

        case DeviceManagementSubCommand::Heartbeat:
            _decodeHeartbeat(data);
            break;

        case DeviceManagementSubCommand::HardwareName:
            _hardwareName = RoomBus::unpackString(data);
            emit statusUpdate();
            break;

        case DeviceManagementSubCommand::ApplicationName:
            _applicationName = RoomBus::unpackString(data);
            emit statusUpdate();
            break;

        case DeviceManagementSubCommand::DeviceName:
            _deviceName = RoomBus::unpackString(data);
            emit statusUpdate();
            break;

        case DeviceManagementSubCommand::CanDiagnosticsReport:
            _decodeCanDiagnosticsReport(data);
            break;

        case DeviceManagementSubCommand::Echo:
            emit echoReceive(data);
            break;

        case DeviceManagementSubCommand::EraseApplicationResponse:
            handleAppEraseComplete();
            break;

        case DeviceManagementSubCommand::BootloadResponse:
            writeBinaryChunk();
            break;

        case DeviceManagementSubCommand::EepromReadReport:
            _eeprom._decodeEepromReadReport(data);
            break;
    }
}

void DeviceManagementProtocol::_decodeSystemInformation(const QByteArray &data)
{
    if(data.size() != 40) return;

    uint32_t status = RoomBus::unpackUint32(data,0);
    _systemStatus = *((SystemStatus*) &status);

    _hardwareVersion = RoomBus::unpackUint16(data,4);
    _kernelVersion = RoomBus::unpackUint16(data,6);
    _heartbeatInterval = RoomBus::unpackUint16(data,8);
    _extendedHeartbeatInterval = RoomBus::unpackUint16(data,10);
    _appCRC = RoomBus::unpackUint32(data,12);
    _appStartAddress = RoomBus::unpackUint32(data,16);
    _deviceIdentificationCode = RoomBus::unpackUint32(data,20);

    _serialNumber.word0 = RoomBus::unpackUint32(data,24);
    _serialNumber.word1 = RoomBus::unpackUint32(data,28);
    _serialNumber.word2 = RoomBus::unpackUint32(data,32);
    _serialNumber.word3 = RoomBus::unpackUint32(data,36);

    _heartbeatTimer.setInterval(_heartbeatInterval*2500);
    _timeoutStatus = false;
    _heartbeatTimer.start();

    emit statusUpdate();
}

void DeviceManagementProtocol::_decodeHeartbeat(const QByteArray &data)
{
    if(data.size() != 4) return;

    uint32_t status = RoomBus::unpackUint32(data,0);
    _systemStatus = *((SystemStatus*) &status);

    if(_heartbeatInterval){
        _timeoutStatus = false;
        _heartbeatTimer.start();
    }

    emit statusUpdate();
}

void DeviceManagementProtocol::_decodeCanDiagnosticsReport(const QByteArray &data)
{
    canDignostics.errorLogCounter = static_cast<uint8_t>(data.at(0));
    uint8_t temp = static_cast<uint8_t>(data.at(1));
    canDignostics.receiveErrorPassive = static_cast<bool>((temp>>7));
    canDignostics.rxErrorCounter = (temp & 0x7F);
    canDignostics.txErrorCounter = static_cast<uint8_t>(data.at(2));
    temp = static_cast<uint8_t>(data.at(3));
    canDignostics.lastErrorCode = static_cast<CanDignostics::ErrorCode>(temp & 0x07);
    canDignostics.dataLastErrorCode = static_cast<CanDignostics::ErrorCode>((temp >> 4)& 0x07);

    if(data.size() > 5) {
        appBenchmark.avg = RoomBus::unpackUint32(data,4);
        appBenchmark.min = RoomBus::unpackUint32(data,8);
        appBenchmark.max = RoomBus::unpackUint32(data,12);
    }

    emit diagnosticsStatusUpdate();
}

Eeprom &DeviceManagementProtocol::eeprom()
{
    return _eeprom;
}

uint16_t DeviceManagementProtocol::heartbeatInterval() const
{
    return _heartbeatInterval;
}

uint16_t DeviceManagementProtocol::systemInfoInterval() const
{
    return _extendedHeartbeatInterval;
}

void DeviceManagementProtocol::on_heartbeatTimeout()
{
    _timeoutStatus = true;
    _heartbeatTimer.stop();

    emit statusUpdate();
}

void DeviceManagementProtocol::on_bootloadRetry(void)
{
    if(_bootloadWritePending) sendMessage(_bootloadLastWrite);
    else _bootloadRetryTimer.stop();
}

void DeviceManagementProtocol::sendEcho(QByteArray txData)
{
    if(txData.size() > 63){
        txData.truncate(63);
    }

    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::Echo);
    msg.data.append(txData);

    sendMessage(msg);
}

void DeviceManagementProtocol::startFirmwareUpload(QString hexPath)
{
    _bootloadDataIndex = 0;

    _appBinary.clear();
    _appBinary.setMemorySize(0x10000,0x30000);
    _appBinary.setAddressGapSize(0x100);
    _appBinary.setAddressAlignment(1);
    _appBinary.load(hexPath);

    if(_appBinary.errorCount())
    {
        for(const QuCLib::HexFileParser::FileError &error: _appBinary.errors() )
        {
            QString errorMessage = "Line "+QString::number(error.lineIndex)+": "+QuCLib::HexFileParser::errorMessage(error);
            emit bootloadStatusUpdate(0,true,errorMessage);
        }
        return;
    }

    if(_appBinary.binary().count() > 1)
    {
        emit bootloadStatusUpdate(0,true,"Address space error: More than one address range");
        return;
    }

    uint32_t size = _appBinary.binary().at(0).data.length()-4;
    QByteArray sizeArray;
    sizeArray.append((char)(size&0xFF));
    sizeArray.append((char)((size>>8)&0xFF));
    sizeArray.append((char)((size>>16)&0xFF));
    sizeArray.append((char)((size>>24)&0xFF));

    _appBinary.replace(0x00010004,sizeArray);

    uint32_t crc =  QuCLib::Crc::crc32(_appBinary.binary().at(0).data.mid(4));
    QByteArray crcArray;
    crcArray.append((char)(crc&0xFF));
    crcArray.append((char)((crc>>8)&0xFF));
    crcArray.append((char)((crc>>16)&0xFF));
    crcArray.append((char)((crc>>24)&0xFF));

    _appBinary.replace(0x00010000,crcArray);

    emit bootloadStatusUpdate(0,false,"File Loaded");

    eraseApp();
}

void DeviceManagementProtocol::handleAppEraseComplete(void)
{
    emit bootloadStatusUpdate(10, false, "App Erase Complete");
    writeBinaryChunk();
}

void DeviceManagementProtocol::eraseApp(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EraseApplication);

    sendMessage(msg);
}

void DeviceManagementProtocol::enterAdministrationMode(QString key)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterAdministrationMode);
    //msg.data.append(RoomBus::packString(key));
    msg.data.append(RoomBus::packUint16(0x1234));

    sendMessage(msg);
}

void DeviceManagementProtocol::exitAdministrationMode(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterAdministrationMode);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeAddress(uint8_t address)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetAddress);
    msg.data.append((uint8_t)address);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeDeviceName(QString name)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetDeviceName);
    msg.data.append(name.toLocal8Bit());

    sendMessage(msg);
}

void DeviceManagementProtocol::writeAdministrationModeKey(QString key)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetAdministrationModeKey);
    msg.data.append(key.toLocal8Bit());

    sendMessage(msg);
}

void DeviceManagementProtocol::requestHeartbeat(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::HeartbeatRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestSystemInfo(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SystemInformationRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestCanDiagnostics(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::CanDiagnosticsRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestSystemRestart()
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::Reboot);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::HeartbeatSettings);
    msg.data.append(RoomBus::packUint16(heartbeatInterval));
    msg.data.append(RoomBus::packUint16(systemInfoInterval));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeControl(SystemControl sysControl)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::WriteControl);
    msg.data.append(RoomBus::packUint32(sysControl.reg));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeSetControl(SystemControl sysControl)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetControl);
    msg.data.append(RoomBus::packUint32(sysControl.reg));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeClearControl(SystemControl sysControl)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::ClearControl);
    msg.data.append(RoomBus::packUint32(sysControl.reg));

    sendMessage(msg);
}

QString DeviceManagementProtocol::getCanErrorCode(DeviceManagementProtocol::CanDignostics::ErrorCode errorCode)
{
    switch (errorCode) {
        case DeviceManagementProtocol::CanDignostics::ErrorCode::NONE:  return "NONE";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::STUFF:  return "STUFF";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::FORM:  return "FORM";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::ACK:  return "ACK";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::BIT1:  return "BIT1";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::BIT0:  return "BIT0";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::CRC:  return "CRC";
        case DeviceManagementProtocol::CanDignostics::ErrorCode::NC:  return "NC";
    }

    return "Unknown";
}

void DeviceManagementProtocol::sendMessage(RoomBus::Message message)
{
    message.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    message.command = (RoomBus::Command)RoomBus::DeviceManagementCommand::HostToDevice;
    ProtocolBase::sendMessage(message);
}

void DeviceManagementProtocol::writeBinaryChunk(void)
{
    if(_appBinary.binary().count() == 0) {
        emit bootloadStatusUpdate(0, true, "No Firmwar Image Loaded");
        return;
    }

    _bootloadRetryTimer.start(1000);

    QByteArray data = _appBinary.binary().at(0).data;

    if(_bootloadDataIndex < data.length()){
        RoomBus::Message msg;
        msg.data.append((uint8_t)DeviceManagementSubCommand::Bootload);
        msg.data.append(RoomBus::packUint32(_bootloadDataIndex));

        for(uint8_t i= 0; i<32;i++){
            if(_bootloadDataIndex >= data.length()) break;

            msg.data.append(static_cast<char>(data.at(_bootloadDataIndex)));
            _bootloadDataIndex++;
        }
        _bootloadWritePending = true;
        _bootloadLastWrite = msg;
        sendMessage(msg);

        uint8_t progress = (uint8_t)((uint64_t)_bootloadDataIndex*100/data.length());
        QString textMessage = "Upload in Progress - ";
        textMessage += QString::number(_bootloadDataIndex)+"/"+QString::number(data.length())+" ";
        textMessage += QString::number(progress)+"%";
        emit bootloadStatusUpdate(progress,false, textMessage);
    }else{
        _appBinary.clear();
        _bootloadWritePending = false;
        emit bootloadStatusUpdate(100,false,"App Upload Complete");
    }
}
