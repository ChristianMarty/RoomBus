#include "deviceManagementProtocol.h"
#include "../../QuCLib/source/crc.h"

DeviceManagementProtocol::DeviceManagementProtocol(RoomBusDevice *device)
    : ProtocolBase(device)
{
    connect(&_bootloadRetryTimer, &QTimer::timeout, this, &DeviceManagementProtocol::on_bootloadRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &DeviceManagementProtocol::on_heartbeatTimeout);
}

void DeviceManagementProtocol::handleMessage(const MiniBus::Message &message)
{
    if(message.protocol != (MiniBus::Protocol)Protocol::DeviceManagementProtocol) return;

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
            _hardwareName = MiniBus::unpackString(data);
            emit statusUpdate();
            break;

        case DeviceManagementSubCommand::ApplicationName:
            _applicationName = MiniBus::unpackString(data);
            emit statusUpdate();
            break;

        case DeviceManagementSubCommand::DeviceName:
            _deviceName = MiniBus::unpackString(data);
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

    uint32_t status = MiniBus::unpackUint32(data,0);
    _systemStatus = *((SystemStatus*) &status);

    _hardwareVersion = MiniBus::unpackUint16(data,4);
    _kernelVersion = MiniBus::unpackUint16(data,6);
    _heartbeatInterval = MiniBus::unpackUint16(data,8);
    _extendedHeartbeatInterval = MiniBus::unpackUint16(data,10);
    _appCRC = MiniBus::unpackUint32(data,12);
    _appStartAddress = MiniBus::unpackUint32(data,16);
    _deviceIdentificationCode = MiniBus::unpackUint32(data,20);

    _serialNumber.word0 = MiniBus::unpackUint32(data,24);
    _serialNumber.word1 = MiniBus::unpackUint32(data,28);
    _serialNumber.word2 = MiniBus::unpackUint32(data,32);
    _serialNumber.word3 = MiniBus::unpackUint32(data,36);

    _heartbeatTimer.setInterval(_heartbeatInterval*2500);
    _timeoutStatus = false;
    _heartbeatTimer.start();

    emit statusUpdate();
}

void DeviceManagementProtocol::_decodeHeartbeat(const QByteArray &data)
{
    if(data.size() != 4) return;

    uint32_t status = MiniBus::unpackUint32(data,0);
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
        appBenchmark.avg = MiniBus::unpackUint32(data,4);
        appBenchmark.min = MiniBus::unpackUint32(data,8);
        appBenchmark.max = MiniBus::unpackUint32(data,12);
    }

    emit diagnosticsStatusUpdate();
}

Eeprom &DeviceManagementProtocol::eeprom()
{
    return _eeprom;
}

QString DeviceManagementProtocol::commandName(MiniBus::Command command)
{
    switch((Command)command){
        case Command::DeviceToHost: return "Device To Host";
        case Command::HostToDevice: return "Host To Device";

        case Command::Reserved0:
        case Command::Reserved1:
        case Command::Reserved2:
        case Command::Reserved3:
        case Command::Reserved4: return "Reserved";

        case Command::SystemTimeSynchronisation: return "System Time Synchronisation";
    }
    return "Unknown Command";
}

QString DeviceManagementProtocol::dataDecoder(MiniBus::Command command, const QByteArray &data)
{
    if(data.isEmpty()) return ""; // TODO: error handling

    DeviceManagementSubCommand subCommand = (DeviceManagementSubCommand)((uint8_t)data.at(0));
    QString output = "Unknown Command";

    switch(subCommand){
        case DeviceManagementSubCommand::Heartbeat: output = "Heartbeat"; break;
        case DeviceManagementSubCommand::SystemInformation: output = "System Information"; break;
        case DeviceManagementSubCommand::HardwareName: output = "Hardware Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementSubCommand::ApplicationName: output = "Application Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementSubCommand::DeviceName: output = "Device Name: "+QString(data.mid(1)) ; break;
        case DeviceManagementSubCommand::HeartbeatRequest: output = "Heartbeat Request"; break;
        case DeviceManagementSubCommand::SystemInformationRequest: output = "System Information Request"; break;
        case DeviceManagementSubCommand::HeartbeatSettings: output = "Heartbeat Settings"; break;
        case DeviceManagementSubCommand::WriteControl: output = "Write Control"; break;
        case DeviceManagementSubCommand::SetControl: output = "Set Control"; break;
        case DeviceManagementSubCommand::ClearControl: output = "Clear Control"; break;
        case DeviceManagementSubCommand::EnterAdministrationMode: output = "Enter Administration Mode"; break;
        case DeviceManagementSubCommand::ExitAdministrationMode: output = "Exit Administration Mode"; break;
        case DeviceManagementSubCommand::SetDeviceName: output = "Set Device Name"; break;
        case DeviceManagementSubCommand::SetAddress: output = "Set Address"; break;
        case DeviceManagementSubCommand::SetAdministrationModeKey: output = "Set Administration Mode Key"; break;

        case DeviceManagementSubCommand::EepromReadRequest: output = "Eeprom Read Request"; break;
        case DeviceManagementSubCommand::EepromReadReport: output = "Eeprom Read Report"; break;

        case DeviceManagementSubCommand::CanDiagnosticsRequest: output = "CAN Diagnostics Request"; break;
        case DeviceManagementSubCommand::CanDiagnosticsReport: output = "CAN Diagnostics Report"; break;

        case DeviceManagementSubCommand::Echo: output = "Echo"; break;
        case DeviceManagementSubCommand::Reboot: output = "Reboot"; break;
        case DeviceManagementSubCommand::EraseApplication: output = "Erase Application"; break;
        case DeviceManagementSubCommand::EraseApplicationResponse: output = "Erase Application Response"; break;
        case DeviceManagementSubCommand::Bootload: output = "Bootload"; break;
        case DeviceManagementSubCommand::BootloadResponse: output = "Bootload Response"; break;
    }

    return output;
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

    MiniBus::Message msg;
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
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EraseApplication);

    sendMessage(msg);
}

void DeviceManagementProtocol::enterAdministrationMode(QString key)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterAdministrationMode);
    //msg.data.append(RoomBus::packString(key));
    msg.data.append(MiniBus::packUint16(0x1234));

    sendMessage(msg);
}

void DeviceManagementProtocol::exitAdministrationMode(void)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::EnterAdministrationMode);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeAddress(uint8_t address)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetAddress);
    msg.data.append((uint8_t)address);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeDeviceName(QString name)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetDeviceName);
    msg.data.append(name.toLocal8Bit());

    sendMessage(msg);
}

void DeviceManagementProtocol::writeAdministrationModeKey(QString key)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetAdministrationModeKey);
    msg.data.append(key.toLocal8Bit());

    sendMessage(msg);
}

void DeviceManagementProtocol::requestHeartbeat(void)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::HeartbeatRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestSystemInfo(void)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SystemInformationRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestCanDiagnostics(void)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::CanDiagnosticsRequest);

    sendMessage(msg);
}

void DeviceManagementProtocol::requestSystemRestart()
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::Reboot);

    sendMessage(msg);
}

void DeviceManagementProtocol::writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::HeartbeatSettings);
    msg.data.append(MiniBus::packUint16(heartbeatInterval));
    msg.data.append(MiniBus::packUint16(systemInfoInterval));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeControl(SystemControl sysControl)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::WriteControl);
    msg.data.append(MiniBus::packUint32(sysControl.reg));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeSetControl(SystemControl sysControl)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::SetControl);
    msg.data.append(MiniBus::packUint32(sysControl.reg));

    sendMessage(msg);
}

void DeviceManagementProtocol::writeClearControl(SystemControl sysControl)
{
    MiniBus::Message msg;
    msg.data.append((uint8_t)DeviceManagementSubCommand::ClearControl);
    msg.data.append(MiniBus::packUint32(sysControl.reg));

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

void DeviceManagementProtocol::sendMessage(MiniBus::Message message)
{
    message.protocol = (MiniBus::Protocol)Protocol::DeviceManagementProtocol;
    message.command = (MiniBus::Command)Command::HostToDevice;
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
        MiniBus::Message msg;
        msg.data.append((uint8_t)DeviceManagementSubCommand::Bootload);
        msg.data.append(MiniBus::packUint32(_bootloadDataIndex));

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
