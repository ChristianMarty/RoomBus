#include "deviceManagementProtocol.h"
#include "../../QuCLib/source/crc.h"

DeviceManagementProtocol::DeviceManagementProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    _deviceName = "Unknown Device";
    _hardwareName = "Unknown Hardware";
    _applicationName = "Unknown Application";

    _btlWritePending = false;

    connect(&_btlRetryTimer, &QTimer::timeout, this, &DeviceManagementProtocol::btlRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &DeviceManagementProtocol::on_heartbeatTimeout);
}

uint16_t DeviceManagementProtocol::heartbeatInterval() const
{
    return _heartbeatInterval;
}

uint16_t DeviceManagementProtocol::systemInfoInterval() const
{
    return _extendedHeartbeatInterval;
}

void DeviceManagementProtocol::handleMessage(RoomBus::Message message)
{
    if(message.protocol != RoomBus::Protocol::DeviceManagementProtocol) return;

    _lastHeartbeat = QDateTime::currentDateTime();

    QByteArray data = message.data;

    if(data.size() == 0) return;

    DeviceManagementSubCommand subCommand = (DeviceManagementSubCommand)((uint8_t)data.at(0));

    switch(subCommand)
    {
    case DeviceManagementSubCommand::SystemInformation: {

        if(data.size() != 41) return;

        uint32_t status = RoomBus::unpackUint32(data,1);
        _sysStatus = *((SystemStatus*) &status);
        _hardwareVersion = RoomBus::unpackUint16(data,5);
        _kernelVersion = RoomBus::unpackUint16(data,7);
        _heartbeatInterval = RoomBus::unpackUint16(data,9);
        _extendedHeartbeatInterval = RoomBus::unpackUint16(data,11);
        _appCRC = RoomBus::unpackUint32(data,13);
        _appStartAddress = RoomBus::unpackUint32(data,17);
        _deviceIdentificationCode = RoomBus::unpackUint32(data,21);
        _serialNumber.word0 = RoomBus::unpackUint32(data,25);
        _serialNumber.word1 = RoomBus::unpackUint32(data,29);
        _serialNumber.word2 = RoomBus::unpackUint32(data,33);
        _serialNumber.word3 = RoomBus::unpackUint32(data,37);

        _heartbeatTimer.setInterval(_heartbeatInterval*2500);
        _timeoutStatus = false;
        _heartbeatTimer.start();

        break;
    }

    case DeviceManagementSubCommand::Heartbeat: {

        if(data.size() != 5) break;

        uint32_t status = RoomBus::unpackUint32(data,1);
        _sysStatus = *((SystemStatus*) &status);

        if(_heartbeatInterval){
            _timeoutStatus = false;
            _heartbeatTimer.start();
        }

        break;
    }

    case DeviceManagementSubCommand::HardwareName: {
        _hardwareName = RoomBus::unpackString(data,1);
        break;
    }

    case DeviceManagementSubCommand::ApplicationName: {
        _applicationName = RoomBus::unpackString(data,1);
        break;
    }

    case DeviceManagementSubCommand::DeviceName:
        _deviceName = RoomBus::unpackString(data,1);
        break;

    case DeviceManagementSubCommand::CanDiagnosticsReport:
    {
        canDignostics.errorLogCounter = static_cast<uint8_t>(data.at(1));
        uint8_t temp = static_cast<uint8_t>(data.at(2));
        canDignostics.receiveErrorPassive = static_cast<bool>((temp>>7));
        canDignostics.rxErrorCounter = (temp & 0x7F);
        canDignostics.txErrorCounter = static_cast<uint8_t>(data.at(3));
        temp = static_cast<uint8_t>(data.at(4));
        canDignostics.lastErrorCode = static_cast<CanDignostics::ErrorCode>(temp & 0x07);
        canDignostics.dataLastErrorCode = static_cast<CanDignostics::ErrorCode>((temp >> 4)& 0x07);

        if(data.size() > 5) {
            appBenchmark.avg = RoomBus::unpackUint32(data,5);
            appBenchmark.min = RoomBus::unpackUint32(data,9);
            appBenchmark.max = RoomBus::unpackUint32(data,13);
        }

        emit statusUpdate();
    }
    break;

    case DeviceManagementSubCommand::Echo:
        emit echoReceive(data.remove(0,1));
        break;

    case DeviceManagementSubCommand::EraseApplicationResponse:
        appEraseComplete();
        break;

    case DeviceManagementSubCommand::BootloadResponse:
        writeBinaryChunk();
        break;
    }
}

void DeviceManagementProtocol::on_heartbeatTimeout()
{
    _timeoutStatus = true;
    _heartbeatTimer.stop();
}

void DeviceManagementProtocol::btlRetry(void)
{
    if(_btlWritePending) emit sendMessage(_btlLastWrite);
    else _btlRetryTimer.stop();
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

    enterRootMode();
    eraseApp();
}

void DeviceManagementProtocol::appEraseComplete(void)
{
    emit bootloadStatusUpdate(10,false,"App Erase Complete");
    writeBinaryChunk();
}


void DeviceManagementProtocol::eraseApp(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EraseApplication);

    sendMessage(msg);
}

void DeviceManagementProtocol::enterRootMode(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterRootMode);
    msg.data.append(RoomBus::packUint16(0x1234));

    sendMessage(msg);
}

void DeviceManagementProtocol::exitRootMode(void)
{
    RoomBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterRootMode);
    msg.data.append((uint8_t)0x00);

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
    enterRootMode();

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
    default: return "Unknown";
    }
}

void DeviceManagementProtocol::sendMessage(RoomBus::Message message)
{
    message.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    message.command = (RoomBus::Command)RoomBus::DeviceManagementCommand::HostToDevice;
    ProtocolBase::sendMessage(message);
}

void DeviceManagementProtocol::writeBinaryChunk(void)
{
    if(_appBinary.binary().count() == 0)
    {
        emit bootloadStatusUpdate(0,true,"No Firmwar Image Loaded");
        return;
    }

    _btlRetryTimer.start(1000);

    QByteArray data = _appBinary.binary().at(0).data;

    if(_bootloadDataIndex < data.length())
    {
        RoomBus::Message msg;
        msg.data.append((uint8_t)DeviceManagementSubCommand::Bootload);
        msg.data.append(RoomBus::packUint32(_bootloadDataIndex));

        for(uint8_t i= 0; i<32;i++){
            if(_bootloadDataIndex >= data.length()) break;

            msg.data.append(static_cast<char>(data.at(static_cast<int>(_bootloadDataIndex))));
            _bootloadDataIndex++;
        }
        _btlWritePending = true;
        _btlLastWrite = msg;
        sendMessage(msg);
        emit bootloadStatusUpdate((uint8_t)((uint64_t)_bootloadDataIndex*100/ data.length()),false,"Upload in Progress");
    }
    else
    {
        _appBinary.clear();
        _btlWritePending = false;
        emit bootloadStatusUpdate(100,false,"App Upload Complete");
        exitRootMode();
    }
}
