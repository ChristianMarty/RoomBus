#include "busDevice.h"
#include <QTimer>
#include <busProtocol.h>
#include "../../QuCLib/source/crc.h"

RoomBusDevice::RoomBusDevice(QObject *parent) : QObject(parent)
{
    _deviceName = "Unknown Device";
    _hardwareName = "Unknown Hardware";
    _applicationName = "Unknown Application";

    _btlWritePending = false;

    connect(&_btlRetryTimer, &QTimer::timeout, this, &RoomBusDevice::btlRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &RoomBusDevice::heartbeatTimeout);
}

RoomBusDevice::RoomBusDevice(uint8_t deviceAddress, QObject *parent): QObject(parent)
{
    _deviceAddress = deviceAddress;

    _deviceName = "Unknown Device";
    _hardwareName = "Unknown Hardware";
    _applicationName = "Unknown Application";

    _btlWritePending = false;

    connect(&_btlRetryTimer, &QTimer::timeout, this, &RoomBusDevice::btlRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &RoomBusDevice::heartbeatTimeout);
}

QString RoomBusDevice::deviceName() const
{
    return _deviceName;
}

QString RoomBusDevice::hardwareName() const
{
    return _hardwareName;
}

QString RoomBusDevice::applicationName() const
{
    return _applicationName;
}

uint8_t RoomBusDevice::deviceAddress() const
{
    return _deviceAddress;
}

QString RoomBusDevice::kernelVersionString(void)
{
    uint8_t high = (uint8_t)((_kernelVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_kernelVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString RoomBusDevice::hardwareVersionString(void)
{
    uint8_t high = (uint8_t)((_hwVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_hwVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString RoomBusDevice::deviceIdentificationString(void)
{
  /*  QString temp;

    uint8_t processor = (uint8_t)(_deviceIdentificationCode >> 28) & 0x0F;
    uint8_t family = (uint8_t)(_deviceIdentificationCode >> 23) & 0x1F;
    uint8_t series = (uint8_t)(_deviceIdentificationCode >> 16) & 0x3F;
    uint8_t die = (uint8_t)(_deviceIdentificationCode >> 12) & 0x0F;
    uint8_t revision = (uint8_t)(_deviceIdentificationCode >> 8) & 0x0F;
    uint8_t selection = (uint8_t)(_deviceIdentificationCode & 0xFF);*/

    switch(_deviceIdentificationCode)
    {
        case 0x61810502: return "ATSAME51J19A (0x61810502)";
        case 0x61810002: return "ATSAME51J19A (0x61810002)";
        case 0x61810303: return "ATSAME51J18A (0x61810303)";
        default: return "Unknown Device (0x"+QString::number(_deviceIdentificationCode,16).rightJustified(8, '0').toUpper()+")";
    }
}

QString RoomBusDevice::deviceSerialNumberString(void)
{
    QString temp;

    temp  = QString::number(_serialNumberWord0,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord1,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord2,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord3,16).rightJustified(8, '0').toUpper();

    return temp;
}

void RoomBusDevice::setDeviceAddress(const uint8_t &deviceAddress)
{
    _deviceAddress = deviceAddress;
}

void RoomBusDevice::sendEcho(QByteArray txData)
{
    if(txData.size() >63) txData.truncate(63);
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(static_cast<char>(DMP_SC_Echo));

    msg.data.append(txData);

    emit dataReady(msg);
}

QString RoomBusDevice::getCanErrorCode(RoomBusDevice::canDignostics_t::errorCode_t errorCode)
{
    switch (errorCode) {
        case RoomBusDevice::canDignostics_t::errorCode_t::NONE:  return "NONE";
        case RoomBusDevice::canDignostics_t::errorCode_t::STUFF:  return "STUFF";
        case RoomBusDevice::canDignostics_t::errorCode_t::FORM:  return "FORM";
        case RoomBusDevice::canDignostics_t::errorCode_t::ACK:  return "ACK";
        case RoomBusDevice::canDignostics_t::errorCode_t::BIT1:  return "BIT1";
        case RoomBusDevice::canDignostics_t::errorCode_t::BIT0:  return "BIT0";
        case RoomBusDevice::canDignostics_t::errorCode_t::CRC:  return "CRC";
        case RoomBusDevice::canDignostics_t::errorCode_t::NC:  return "NC";
        default: return "Unknown";
    }
}

void RoomBusDevice::startFirmwareUpload(QString hexPath)
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

void RoomBusDevice::appEraseComplete(void)
{
    emit bootloadStatusUpdate(10,false,"App Erase Complete");
    writeBinaryChunk();
}

void RoomBusDevice::btlRetry(void)
{
    if(_btlWritePending) emit dataReady(_btlLastWrite);
    else _btlRetryTimer.stop();

}

void RoomBusDevice::heartbeatTimeout()
{
    _timeoutStatus = true;
    _heartbeatTimer.stop();
}

void RoomBusDevice::writeBinaryChunk(void)
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
        msg.destinationAddress = _deviceAddress;
        msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
        msg.command = 0;
        msg.data.append(static_cast<char>(DMP_SC_Bootload));

        msg.data.append(static_cast<char>((_bootloadDataIndex>>24) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex>>16) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex>>8) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex) & 0xFF));

        for(uint8_t i= 0; i<32;i++)
        {
            if(_bootloadDataIndex >= data.length()) break;

            msg.data.append(static_cast<char>(data.at(static_cast<int>(_bootloadDataIndex))));
            _bootloadDataIndex++;
        }
        _btlWritePending = true;
        _btlLastWrite = msg;
        emit dataReady(msg);
        emit bootloadStatusUpdate((uint8_t)((uint64_t)_bootloadDataIndex*100/ data.count()),false,"Upload in Progress");
    }
    else
    {
        _appBinary.clear();
        _btlWritePending = false;
        emit bootloadStatusUpdate(100,false,"App Upload Complete");
        exitRootMode();
    }
}

void RoomBusDevice::eraseApp(void)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(static_cast<char>(DMP_SC_EraseApplication));

    emit dataReady(msg);
}

void RoomBusDevice::enterRootMode(void)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_EnterRootMode);
    msg.data.append(static_cast<char>(0x12));
    msg.data.append(static_cast<char>(0x34));

    emit dataReady(msg);
}

void RoomBusDevice::exitRootMode(void)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_EnterRootMode);
    msg.data.append(static_cast<char>(0x00));

    emit dataReady(msg);
}

void RoomBusDevice::writeAddress(uint8_t address)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_SetAddress);
    msg.data.append(static_cast<char>(address));

    emit dataReady(msg);
}

void RoomBusDevice::writeDeviceName(QString name)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_SetDeviceName);
    msg.data.append(name.toLocal8Bit());

    emit dataReady(msg);
}

void RoomBusDevice::requestHeartbeat(void)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_HeartbeatRequest);

    emit dataReady(msg);
}

void RoomBusDevice::requestSystemInfo(void)
{    
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_SystemInformationRequest);

    emit dataReady(msg);
}

void RoomBusDevice::requestCanDiagnostics(void)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_CanDiagnosticsRequest);

    emit dataReady(msg);
}

void RoomBusDevice::requestSystemRestart()
{
    enterRootMode();

    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(DMP_SC_Reboot);

    emit dataReady(msg);
}

void RoomBusDevice::writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;

    msg.data.append(DMP_SC_HeartbeatSettings);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(heartbeatInterval>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(heartbeatInterval&0xFF)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(systemInfoInterval>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(systemInfoInterval&0xFF)));

    emit dataReady(msg);
}

void RoomBusDevice::writeControl(sysControl_t sysControl)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(DMP_SC_WriteControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

void RoomBusDevice::writeSetControl(sysControl_t sysControl)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(DMP_SC_SetControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

void RoomBusDevice::writeClearControl(sysControl_t sysControl)
{
    RoomBus::Message msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(DMP_SC_ClearControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

RoomBusDevice::sysStatus_t RoomBusDevice::sysStatus()
{
    return _sysStatus;
}

bool RoomBusDevice::timeoutStatus()
{
    return _timeoutStatus;
}

uint16_t RoomBusDevice::heartbeatInterval() const
{
    return _heartbeatInterval;
}

uint16_t RoomBusDevice::systemInfoInterval() const
{
    return _extendedHeartbeatInterval;
}

void RoomBusDevice::addProtocol(BusProtocol* protocol)
{
    _protocols.append(protocol);
}

void RoomBusDevice::removeProtocol(BusProtocol* protocol)
{
    _protocols.removeOne(protocol);
}

void RoomBusDevice::pushData(RoomBus::Message msg)
{
    if(msg.sourceAddress != _deviceAddress) return;

    for (uint8_t i = 0; i<_protocols.size();i++)
    {
        if(_protocols.at(i) != nullptr)
        {
            QList<RoomBus::Protocol> temp = _protocols.at(i)->protocol();
            for (uint8_t j = 0; j<temp.size();j++)
            {
                if(msg.protocol == temp.at(j))
                {
                    _protocols.at(i)->pushData(msg);
                }
            }
        }
    }

    if(msg.protocol == RoomBus::Protocol::DeviceManagementProtocol)
    {
        handleDeviceManagementProtocol(msg);
    }
}

void RoomBusDevice::handleDeviceManagementProtocol(RoomBus::Message msg)
{
    _lastHeartbeat = QDateTime::currentDateTime();
    QByteArray data = msg.data;

    if(data.size() == 0) return;

    switch(static_cast<uint8_t>(data.at(0)))
    {

    case DMP_SC_SystemInformation:

        if(data.size() != 41) return;

        _hwVersion = RoomBus::unpackUint16(data,5);
        _kernelVersion = RoomBus::unpackUint16(data,7);
        _heartbeatInterval = RoomBus::unpackUint16(data,9);
        _extendedHeartbeatInterval = RoomBus::unpackUint16(data,11);
        _appCRC = RoomBus::unpackUint32(data,13);
        _appStartAddress = RoomBus::unpackUint32(data,17);
        _deviceIdentificationCode = RoomBus::unpackUint32(data,21);
        _serialNumberWord0 = RoomBus::unpackUint32(data,25);
        _serialNumberWord1 = RoomBus::unpackUint32(data,29);
        _serialNumberWord2 = RoomBus::unpackUint32(data,33);
        _serialNumberWord3 = RoomBus::unpackUint32(data,37);

        _heartbeatTimer.setInterval(_heartbeatInterval*2500);

    case DMP_SC_Heartbeat:
        {
            if(data.size() != 5) break;
            uint32_t status = RoomBus::unpackUint32(data,1);
            _sysStatus = *((sysStatus_t*) &status);

            _timeoutStatus = false;
            _heartbeatTimer.start();
        }
        break;

    case DMP_SC_HardwareName:
        _hardwareName = "";
        for(uint8_t i= 1; i<(data.size());i++)
        {
            char temp = data.at(i);
            if(temp == 0) break;
            _hardwareName += temp;
        }
        break;

    case DMP_SC_ApplicationName:
        _applicationName ="";
        for(uint8_t i= 0; i<(data.size()-1);i++)
        {
            char temp = data.at(1+i);
            if(temp == 0) break;
            _applicationName += temp;
        }
        break;

    case DMP_SC_DeviceName:
        _deviceName = "";
        for(uint8_t i= 0; i<(data.size()-1);i++)
        {
            char temp = data.at(1+i);
            if(temp == 0) break;
            _deviceName += temp;
        }
        break;

    case DMP_SC_CanDiagnosticsReport:
        {
            canDignostics.errorLogCounter = static_cast<uint8_t>(data.at(1));
            uint8_t temp = static_cast<uint8_t>(data.at(2));
            canDignostics.receiveErrorPassive = static_cast<bool>((temp>>7));
            canDignostics.rxErrorCounter = (temp & 0x7F);
            canDignostics.txErrorCounter = static_cast<uint8_t>(data.at(3));
            temp = static_cast<uint8_t>(data.at(4));
            canDignostics.lastErrorCode = static_cast<canDignostics_t::errorCode_t>(temp & 0x07);
            canDignostics.dataLastErrorCode = static_cast<canDignostics_t::errorCode_t>((temp >> 4)& 0x07);

            if(data.size() > 5)
            {
                appBenchmark.avg = RoomBus::unpackUint32(data,5);
                appBenchmark.min = RoomBus::unpackUint32(data,9);
                appBenchmark.max = RoomBus::unpackUint32(data,13);
            }

            emit statusUpdate();
        }
        break;

    case DMP_SC_Echo:
        emit echoReceive(data.remove(0,1));
        break;

    case DMP_SC_EraseApplicationResponse:
        appEraseComplete();
        break;

    case DMP_SC_BootloadResponse:
        writeBinaryChunk();
        break;
    }
}

QDateTime RoomBusDevice::lastHeartbeat() const
{
    return _lastHeartbeat;
}


