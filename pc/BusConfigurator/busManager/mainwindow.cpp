#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    settings.readFile("../settings.xml");

    addConnection();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::addConnection()
{
    busAccess *comTemp = nullptr;

    if(settings.getSettings().connection.interface == settings_t::connection_t::tcp)
    {
        comTemp = new busAccess(busAccess::tcp);
        comTemp->openTcpConnection(settings.getSettings().connection.ip,settings.getSettings().connection.port.toInt());
    }
    else if(settings.getSettings().connection.interface == settings_t::connection_t::serial)
    {
        comTemp = new busAccess(busAccess::serial);
        comTemp->openSerialConnection(settings.getSettings().connection.port);
    }

    else if(settings.getSettings().connection.interface == settings_t::connection_t::udp)
    {
        comTemp = new busAccess(busAccess::udp);
        comTemp->openUdpConnection(settings.getSettings().connection.ip,settings.getSettings().connection.port.toInt());
    }
    else
    {
        return;
    }

    connect(comTemp, &busAccess::newData,this, &MainWindow::on_newData);
    comTemp->setPriority(busAccess::busPriority::low);
    _busConnection.append(comTemp);

    busConnectionWidget *widgetTemp = new busConnectionWidget(comTemp);
    _busConnectionWidget.append(widgetTemp);
    ui->connectionGrid->insertWidget(0,widgetTemp);
}

void MainWindow::on_newData(void)
{
    while(_busConnection.at(0)->rxMsgBuffer.size())
    {
        busMessage temp = _busConnection.at(0)->rxMsgBuffer.first();
        _busConnection.at(0)->rxMsgBuffer.removeFirst();

        if(_monitorWindow != nullptr) _monitorWindow->on_newMessage(temp);

        busDevice *device = getDevice(temp.srcAddress);
        device->pushData(temp);
    }

    updateDevices();
}

void MainWindow::on_deviceTx(busMessage msg)
{
    msg.srcAddress = 0;
    _busConnection.at(0)->write(msg,busAccess::busPriority::normal);
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



busDevice *MainWindow::getDevice(uint8_t address)
{
    for(int i = 0; i<_busDeviceList.size(); i++)
    {
        if(_busDeviceList.at(i)->deviceAddress() == address)
        {
            return _busDeviceList.at(i);
        }
    }

    busDevice *temp = addDevice(address);
    connect(temp,&busDevice::dataReady,this,&MainWindow::on_deviceTx);

    temp->setDeviceAddress(address);
    return temp;
}

busDevice *MainWindow::addDevice(uint8_t address)
{
    busDevice *temp2 = new busDevice(address);

    busDeviceWidget *temp = new busDeviceWidget(temp2);


    int nrOfDevice = _busDeviceList.size();


    int i = 0;
    if(nrOfDevice)
    {
        foreach( busDevice *device, _busDeviceList )
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
    busMessage msg;
    msg.dstAddress = 0x7F;
    msg.protocol = Protocol::DeviceManagementProtocol;
    msg.command = 0x02;
    msg.data.append(busDevice::cmd_systemInformationRequest);
    _busConnection.at(0)->write(msg,busAccess::busPriority::normal);
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

void MainWindow::on_busDeviceWindowShow(busDevice *device)
{
    if(!_busDeviceWindowMap.contains(device))
    {
        _busDeviceWindowMap.insert(device, new busDeviceWindow(device, this));
    }

    this->addDockWidget(Qt::RightDockWidgetArea,_busDeviceWindowMap[device]);
    _busDeviceWindowMap[device]->show();
}
