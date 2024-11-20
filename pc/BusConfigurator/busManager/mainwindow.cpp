#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings.readFile("C:/Users/Christian/Raumsteuerung/RoomBus/pc/BusConfigurator/settings.xml");

    connect(&_busConnection, &RoomBusAccess::newData,this, &MainWindow::on_newData);
    _busConnection.setDefaultPriority(RoomBusAccess::Priority::Low);

    addConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addConnection()
{
    if(settings.getSettings().connection.interface == settings_t::connection_t::tcp)
    {
        _busConnection.openTcpConnection(settings.getSettings().connection.ip,settings.getSettings().connection.port.toInt());
    }
    else if(settings.getSettings().connection.interface == settings_t::connection_t::serial)
    {
        _busConnection.openSerialConnection(settings.getSettings().connection.port);
    }
    else if(settings.getSettings().connection.interface == settings_t::connection_t::udp)
    {
        _busConnection.openTcpConnection(settings.getSettings().connection.ip,settings.getSettings().connection.port.toInt());
    }
    else
    {
        return;
    }

    busConnectionWidget *widgetTemp = new busConnectionWidget(_busConnection);
    _busConnectionWidget.append(widgetTemp);
    ui->connectionGrid->insertWidget(0,widgetTemp);
}

void MainWindow::on_newData(void)
{
    while(_busConnection.rxMsgBuffer.size())
    {
        RoomBus::Message temp = _busConnection.rxMsgBuffer.first();
        _busConnection.rxMsgBuffer.removeFirst();

        if(_monitorWindow != nullptr){
            _monitorWindow->on_newMessage(temp);
        }

        RoomBusDevice *device = getDevice(temp.sourceAddress);
        device->pushData(temp);
    }

    updateDevices();
}

void MainWindow::on_deviceTx(RoomBus::Message msg)
{
    msg.sourceAddress = 0;
    _busConnection.write(msg,RoomBusAccess::Priority::Normal);
}

void MainWindow::updateDevices(void)
{
    for(int i = 0; i<_busDeviceWidgetLsit.size(); i++)
    {
        _busDeviceWidgetLsit.at(i)->updateData();
    }

    foreach(const auto& value, _busDeviceWindowMap.values())
    {
        value->updateData();
    }
}

RoomBusDevice *MainWindow::getDevice(uint8_t address)
{
    for(int i = 0; i<_busDeviceList.size(); i++)
    {
        if(_busDeviceList.at(i)->deviceAddress() == address)
        {
            return _busDeviceList.at(i);
        }
    }

    RoomBusDevice *temp = addDevice(address);
    connect(temp,&RoomBusDevice::dataReady,this,&MainWindow::on_deviceTx);

    temp->setDeviceAddress(address);
    return temp;
}

RoomBusDevice *MainWindow::addDevice(uint8_t address)
{
    RoomBusDevice *temp2 = new RoomBusDevice(address);

    busDeviceWidget *temp = new busDeviceWidget(temp2);


    int nrOfDevice = _busDeviceList.size();


    int i = 0;
    if(nrOfDevice)
    {
        foreach( RoomBusDevice *device, _busDeviceList )
        {
            if(device->deviceAddress() > address)
            {
                _busDeviceList.insert(i,temp2);
                _busDeviceWidgetLsit.insert(i,temp);
                break;
            }
            i++;

            if(i==nrOfDevice)
            {
                _busDeviceList.append(temp2);
                _busDeviceWidgetLsit.append(temp);
            }
        }
    }
    else
    {
        _busDeviceList.append(temp2);
        _busDeviceWidgetLsit.append(temp);
    }

    ui->deviceGrid->insertWidget(i,temp);

    temp->updateData();

    connect(temp, &busDeviceWidget::busDeviceShow, this, &MainWindow::on_busDeviceWindowShow);

    return temp2;
}

void MainWindow::on_scanButton_clicked()
{
    RoomBus::Message msg;
    msg.destinationAddress = 0x7F;
    msg.protocol = RoomBus::Protocol::DeviceManagementProtocol;
    msg.command = 0x02;
    msg.data.append(RoomBusDevice::DMP_SC_SystemInformationRequest);
    _busConnection.write(msg, RoomBusAccess::Priority::Normal);
}

void MainWindow::on_monitorButton_clicked()
{
    if(_monitorWindow == nullptr) _monitorWindow = new busMonitor();
    _monitorWindow->show();
}

void MainWindow::on_qosButton_clicked()
{
    if(_qosWindow == nullptr) _qosWindow = new qualityOfServiceWindow(&_busDeviceList);
    _qosWindow->show();

    this->addDockWidget(Qt::BottomDockWidgetArea,_qosWindow);
}

void MainWindow::on_busDeviceWindowShow(RoomBusDevice *device)
{
    if(!_busDeviceWindowMap.contains(device))
    {
        _busDeviceWindowMap.insert(device, new busDeviceWindow(device, this));
    }

    this->addDockWidget(Qt::RightDockWidgetArea,_busDeviceWindowMap[device]);
    _busDeviceWindowMap[device]->show();
}
