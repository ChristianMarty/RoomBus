#include "busDevice.h"
#include <QTimer>
#include <busProtocol.h>
#include "../../QuCLib/source/crc.h"

busDevice::busDevice(QObject *parent) : QObject(parent)
{
    _deviceName = "Unknown Device";
    _hardwareName = "Unknown Hardware";
    _applicationName = "Unknown Application";

    _btlWritePending = false;

    connect(&_btlRetryTimer, &QTimer::timeout, this, &busDevice::btlRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &busDevice::heartbeatTimeout);
}

busDevice::busDevice(uint8_t deviceAddress, QObject *parent): QObject(parent)
{
    _deviceAddress = deviceAddress;

    _deviceName = "Unknown Device";
    _hardwareName = "Unknown Hardware";
    _applicationName = "Unknown Application";

    _btlWritePending = false;

    connect(&_btlRetryTimer, &QTimer::timeout, this, &busDevice::btlRetry );
    connect(&_heartbeatTimer,  &QTimer::timeout, this, &busDevice::heartbeatTimeout);
}

QString busDevice::deviceName() const
{
    return _deviceName;
}

QString busDevice::hardwareName() const
{
    return _hardwareName;
}

QString busDevice::applicationName() const
{
    return _applicationName;
}

uint8_t busDevice::deviceAddress() const
{
    return _deviceAddress;
}

QString busDevice::kernelVersionString(void)
{
    uint8_t high = (uint8_t)((_kernelVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_kernelVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString busDevice::hardwareVersionString(void)
{
    uint8_t high = (uint8_t)((_hwVersion >> 8) & 0xFF);
    uint8_t low = (uint8_t)(_hwVersion & 0xFF);

    return QString::number(high,10).rightJustified(2, '0')+"."+QString::number(low,10).rightJustified(2, '0');
}

QString busDevice::deviceIdentificationString(void)
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

QString busDevice::deviceSerialNumberString(void)
{
    QString temp;

    temp  = QString::number(_serialNumberWord0,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord1,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord2,16).rightJustified(8, '0').toUpper()+"-";
    temp += QString::number(_serialNumberWord3,16).rightJustified(8, '0').toUpper();

    return temp;
}

void busDevice::setDeviceAddress(const uint8_t &deviceAddress)
{
    _deviceAddress = deviceAddress;
}

void busDevice::sendEcho(QByteArray txData)
{
    if(txData.size() >63) txData.truncate(63);
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(static_cast<char>(cmd_echo));

    msg.data.append(txData);

    emit dataReady(msg);
}

QString busDevice::getCanErrorCode(busDevice::canDignostics_t::errorCode_t errorCode)
{
    switch (errorCode) {
        case busDevice::canDignostics_t::errorCode_t::NONE:  return "NONE";
        case busDevice::canDignostics_t::errorCode_t::STUFF:  return "STUFF";
        case busDevice::canDignostics_t::errorCode_t::FORM:  return "FORM";
        case busDevice::canDignostics_t::errorCode_t::ACK:  return "ACK";
        case busDevice::canDignostics_t::errorCode_t::BIT1:  return "BIT1";
        case busDevice::canDignostics_t::errorCode_t::BIT0:  return "BIT0";
        case busDevice::canDignostics_t::errorCode_t::CRC:  return "CRC";
        case busDevice::canDignostics_t::errorCode_t::NC:  return "NC";
        default: return "Unknown";
    }
}

void busDevice::startFirmwareUpload(QString hexPath)
{
    _bootloadDataIndex = 0;

    _appBinary.clear();
    _appBinary.setMemorySize(0x10000,0x30000);
    _appBinary.setAddressGapSize(0x100);
    _appBinary.setAddressAlignment(1);
    _appBinary.load(hexPath);

    if(_appBinary.errorCount())
    {
        foreach(const QuCLib::HexFileParser::fileError &error, _appBinary.errors() )
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

    uint32_t size = _appBinary.binary().at(0).data.count()-4;
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

void busDevice::appEraseComplete(void)
{
    emit bootloadStatusUpdate(10,false,"App Erase Complete");
    writeBinaryChunk();
}

void busDevice::btlRetry(void)
{
    if(_btlWritePending) emit dataReady(_btlLastWrite);
    else _btlRetryTimer.stop();

}

void busDevice::heartbeatTimeout()
{
    _timeoutStatus = true;
    _heartbeatTimer.stop();
}

void busDevice::writeBinaryChunk(void)
{
    if(_appBinary.binary().count() == 0)
    {
        emit bootloadStatusUpdate(0,true,"No Firmwar Image Loaded");
        return;
    }

    _btlRetryTimer.start(1000);

    QByteArray data = _appBinary.binary().at(0).data;

    if(_bootloadDataIndex < data.count())
    {
        BusMessage msg;
        msg.destinationAddress = _deviceAddress;
        msg.protocol = Protocol::DeviceManagementProtocol;
        msg.command = 0;
        msg.data.append(static_cast<char>(cmd_btl));

        msg.data.append(static_cast<char>((_bootloadDataIndex>>24) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex>>16) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex>>8) & 0xFF));
        msg.data.append(static_cast<char>((_bootloadDataIndex) & 0xFF));

        for(uint8_t i= 0; i<32;i++)
        {
            if(_bootloadDataIndex >= data.count()) break;

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

void busDevice::eraseApp(void)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(static_cast<char>(cmd_eraseApp));

    emit dataReady(msg);
}

void busDevice::enterRootMode(void)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_enterRootMode);
    msg.data.append(static_cast<char>(0x12));
    msg.data.append(static_cast<char>(0x34));

    emit dataReady(msg);
}

void busDevice::exitRootMode(void)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_enterRootMode);
    msg.data.append(static_cast<char>(0x00));

    emit dataReady(msg);
}

void busDevice::writeAddress(uint8_t address)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_setAddress);
    msg.data.append(static_cast<char>(address));

    emit dataReady(msg);
}

void busDevice::writeDeviceName(QString name)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_setDeviceName);
    msg.data.append(name.toLocal8Bit());

    emit dataReady(msg);
}

void busDevice::requestHeartbeat(void)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_heartbeatRequest);

    emit dataReady(msg);
}

void busDevice::requestSystemInfo(void)
{    
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_systemInformationRequest);

    emit dataReady(msg);
}

void busDevice::requestCanDiagnostics(void)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_canDiagnosticsRequest);

    emit dataReady(msg);
}

void busDevice::requestSystemRestart()
{
    enterRootMode();

    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;
    msg.data.append(cmd_reboot);

    emit dataReady(msg);
}

void busDevice::writeHeartbeatInterval(uint16_t heartbeatInterval, uint16_t systemInfoInterval)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;

    msg.data.append(cmd_heartbeatSettings);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(heartbeatInterval>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(heartbeatInterval&0xFF)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(systemInfoInterval>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(systemInfoInterval&0xFF)));

    emit dataReady(msg);
}

void busDevice::writeControl(sysControl_t sysControl)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(cmd_writeControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

void busDevice::writeSetControl(sysControl_t sysControl)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(cmd_setControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

void busDevice::writeClearControl(sysControl_t sysControl)
{
    BusMessage msg;
    msg.destinationAddress = _deviceAddress;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0;

    uint32_t temp = sysControl.reg;
    msg.data.append(cmd_clrControl);
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>24)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>16)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp>>8)));
    msg.data.append(static_cast<char>(static_cast<uint8_t>(temp)));

    emit dataReady(msg);
}

busDevice::sysStatus_t busDevice::sysStatus()
{
    return _sysStatus;
}

bool busDevice::timeoutStatus()
{
    return _timeoutStatus;
}

uint16_t busDevice::heartbeatInterval() const
{
    return _heartbeatInterval;
}

uint16_t busDevice::systemInfoInterval() const
{
    return _extendedHeartbeatInterval;
}

void busDevice::addProtocol(BusProtocol* protocol)
{
    _protocols.append(protocol);
}

void busDevice::removeProtocol(BusProtocol* protocol)
{
    _protocols.removeOne(protocol);
}

void busDevice::pushData(BusMessage msg)
{
    if(msg.sourceAddress != _deviceAddress) return;

    for (uint8_t i = 0; i<_protocols.size();i++)
    {
        if(_protocols.at(i) != nullptr)
        {
            QList<Protocol> temp = _protocols.at(i)->protocol();
            for (uint8_t j = 0; j<temp.size();j++)
            {
                if(msg.protocol == temp.at(j))
                {
                    _protocols.at(i)->pushData(msg);
                }
            }
        }
    }

    if(msg.protocol == Protocol::DeviceManagementProtocol)
    {
        handleDeviceManagementProtocol(msg);
    }
}

void busDevice::handleDeviceManagementProtocol(BusMessage msg)
{
    _lastHeartbeat = QDateTime::currentDateTime();
    QByteArray data = msg.data;

    if(data.size() == 0) return;

    switch(static_cast<uint8_t>(data.at(0)))
    {

    case cmd_systemInfo:

        if(data.size() != 41) return;

        _hwVersion = BusProtocol::getUint16(data,5);
        _kernelVersion = BusProtocol::getUint16(data,7);
        _heartbeatInterval = BusProtocol::getUint16(data,9);
        _extendedHeartbeatInterval = BusProtocol::getUint16(data,11);
        _appCRC = BusProtocol::getUint32(data,13);
        _appStartAddress = BusProtocol::getUint32(data,17);
        _deviceIdentificationCode = BusProtocol::getUint32(data,21);
        _serialNumberWord0 = BusProtocol::getUint32(data,25);
        _serialNumberWord1 = BusProtocol::getUint32(data,29);
        _serialNumberWord2 = BusProtocol::getUint32(data,33);
        _serialNumberWord3 = BusProtocol::getUint32(data,37);

        _heartbeatTimer.setInterval(_heartbeatInterval*2500);

    case cmd_heartbeat:
        {
            if(data.size() != 5) break;
            uint32_t status = BusProtocol::getUint32(data,1);
            _sysStatus = *((sysStatus_t*) &status);

            _timeoutStatus = false;
            _heartbeatTimer.start();
        }
        break;

    case cmd_hardwareName:
        _hardwareName = "";
        for(uint8_t i= 1; i<(data.size());i++)
        {
            char temp = data.at(i);
            if(temp == 0) break;
            _hardwareName += temp;
        }
        break;

    case cmd_applicationName:
        _applicationName ="";
        for(uint8_t i= 0; i<(data.size()-1);i++)
        {
            char temp = data.at(1+i);
            if(temp == 0) break;
            _applicationName += temp;
        }
        break;

    case cmd_deviceName:
        _deviceName = "";
        for(uint8_t i= 0; i<(data.size()-1);i++)
        {
            char temp = data.at(1+i);
            if(temp == 0) break;
            _deviceName += temp;
        }
        break;

    case cmd_canDiagnosticsReport:
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
                appBenchmark.avg = BusProtocol::getUint32(data,5);
                appBenchmark.min = BusProtocol::getUint32(data,9);
                appBenchmark.max = BusProtocol::getUint32(data,13);
            }

            emit statusUpdate();
        }
        break;

    case cmd_echo:
        emit echoReceive(data.remove(0,1));
        break;

    case cmd_eraseAppRsp:
        appEraseComplete();
        break;

    case cmd_btlRsp:
        writeBinaryChunk();
        break;
    }
}

QDateTime busDevice::lastHeartbeat() const
{
    return _lastHeartbeat;
}


