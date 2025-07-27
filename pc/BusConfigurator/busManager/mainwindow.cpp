#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    _settings.readFile("C:/Users/Christian/Raumsteuerung/RoomBus/pc/BusConfigurator/settings.xml");

    connect(&_busConnection, &RoomBusAccess::newData,this, &MainWindow::on_newData);

    addConnection();
    ui->connectionDock->setWidget(_busConnectionWidget);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addConnection()
{
    if(_settings.getSettings().connection.interface == Settings::Data::Connection::Tcp){
        _busConnection.setTcpConnection(_settings.getSettings().connection.ip,_settings.getSettings().connection.port.toInt());
    }else if(_settings.getSettings().connection.interface == Settings::Data::Connection::Serial){
        _busConnection.setSerialConnection(_settings.getSettings().connection.port);
    }else if(_settings.getSettings().connection.interface == Settings::Data::Connection::Udp){
        _busConnection.setTcpConnection(_settings.getSettings().connection.ip,_settings.getSettings().connection.port.toInt());
    }else{
        return;
    }
    _busConnection.openConnection();
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
        device->handleMessage(temp);
    }

    updateDevices();
}

void MainWindow::on_deviceTx(RoomBus::Message msg)
{
    msg.sourceAddress = 0;
    msg.priority = RoomBus::Priority::Normal;
    _busConnection.write(msg);
}

void MainWindow::updateDevices(void)
{
    for(BusDeviceWidget *value: _busDeviceWidgetList){
        value->updateData();
    }

    for(BusDeviceWindow *value: _busDeviceWindowMap){
        value->updateData();
    }
}

RoomBusDevice *MainWindow::getDevice(uint8_t address)
{
    for(RoomBusDevice *device: _busDeviceList){
        if(device->deviceAddress() == address){
            return device;
        }
    }

    RoomBusDevice *roomBusDevice = addDevice(address);
    connect(roomBusDevice, &RoomBusDevice::dataReady, this, &MainWindow::on_deviceTx);

    roomBusDevice->setDeviceAddress(address);
    return roomBusDevice;
}

RoomBusDevice *MainWindow::addDevice(uint8_t address)
{
    int index = 0;
    for(RoomBusDevice *device: _busDeviceList){
        if(device->deviceAddress() > address){
            break;
        }
        index++;
    }

    RoomBusDevice *roomBusDevice = new RoomBusDevice(address);
    BusDeviceWidget *busDeviceWidget = new BusDeviceWidget(roomBusDevice, this);

    _busDeviceList.insert(index,roomBusDevice);
    _busDeviceWidgetList.insert(index, busDeviceWidget);

    QListWidgetItem *itemWidget = new QListWidgetItem();
    itemWidget->setSizeHint(busDeviceWidget->sizeHint());

    ui->listWidget_device->insertItem(index, itemWidget);
    ui->listWidget_device->setItemWidget(itemWidget, busDeviceWidget);

    busDeviceWidget->updateData();
    connect(busDeviceWidget, &BusDeviceWidget::busDeviceShow, this, &MainWindow::on_busDeviceWindowShow);

    return roomBusDevice;
}

void MainWindow::on_scanButton_clicked()
{
    _busConnection.write(RoomBusDevice::busScan());
}

void MainWindow::on_monitorButton_clicked()
{
    if(_monitorWindow == nullptr){
        _monitorWindow = new busMonitor();
    }
    _monitorWindow->show();
}

void MainWindow::on_qosButton_clicked()
{
    if(_qosWindow == nullptr){
        _qosWindow = new qualityOfServiceWindow(&_busDeviceList);
    }
    _qosWindow->show();

    this->addDockWidget(Qt::BottomDockWidgetArea, _qosWindow);
}

void MainWindow::on_busDeviceWindowShow(RoomBusDevice *device)
{
    if(!_busDeviceWindowMap.contains(device)){
        _busDeviceWindowMap.insert(device, new BusDeviceWindow(device, this));
    }

    this->addDockWidget(Qt::RightDockWidgetArea, _busDeviceWindowMap[device]);
    _busDeviceWindowMap[device]->show();
}
