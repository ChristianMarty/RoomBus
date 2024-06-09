#include "qualityOfServiceWindow.h"
#include "ui_qualityOfServiceWindow.h"

qualityOfServiceWindow::qualityOfServiceWindow(QList<busDevice*> *deviceList, QWidget *parent) :
    QDockWidget(parent),
    ui(new Ui::qualityOfServiceWindow)
{
    ui->setupUi(this);
    _deviceList = deviceList;

    for(int i = 0; i< _deviceList->size(); i++)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,0,new QTableWidgetItem(QString::number(_deviceList->at(i)->deviceAddress())));
        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,1,new QTableWidgetItem(_deviceList->at(i)->deviceName()));

        connect(_deviceList->at(i),&busDevice::statusUpdate, this, &qualityOfServiceWindow::on_statusUpdate);
    }

    connect(&_autoReadTimer, &QTimer::timeout, this, &qualityOfServiceWindow::on_readTimer);
    _autoReadTimer.setInterval(1000);
}

qualityOfServiceWindow::~qualityOfServiceWindow()
{
    delete ui;
}


void qualityOfServiceWindow::on_statusUpdate(void)
{
    for(int i = 0; i< _deviceList->size(); i++)
    {
        ui->tableWidget->setItem(i,2,new QTableWidgetItem(QString::number(_deviceList->at(i)->canDignostics.errorLogCounter)));
        ui->tableWidget->setItem(i,3,new QTableWidgetItem(QString::number(_deviceList->at(i)->canDignostics.txErrorCounter)));
        ui->tableWidget->setItem(i,4,new QTableWidgetItem(QString::number(_deviceList->at(i)->canDignostics.rxErrorCounter)));

        ui->tableWidget->setItem(i,5,new QTableWidgetItem(busDevice::getCanErrorCode(_deviceList->at(i)->canDignostics.lastErrorCode)));
        ui->tableWidget->setItem(i,6,new QTableWidgetItem(busDevice::getCanErrorCode(_deviceList->at(i)->canDignostics.dataLastErrorCode)));

        ui->tableWidget->setItem(i,7,new QTableWidgetItem(QString::number(_deviceList->at(i)->appBenchmark.avg)));
        ui->tableWidget->setItem(i,8,new QTableWidgetItem(QString::number(_deviceList->at(i)->appBenchmark.min)));
        ui->tableWidget->setItem(i,9,new QTableWidgetItem(QString::number(_deviceList->at(i)->appBenchmark.max)));
    }
}

void qualityOfServiceWindow::on_readButton_clicked()
{
    read();
}

void qualityOfServiceWindow::on_autoReadCheckBox_stateChanged(int arg1)
{
    if(arg1 != false) _autoReadTimer.start();
    else _autoReadTimer.stop();
}

void qualityOfServiceWindow::on_readTimer(void)
{
    read();
}

void qualityOfServiceWindow::read()
{
    for(int i = 0; i< _deviceList->size(); i++)
    {
        _deviceList->at(i)->requestCanDiagnostics();
    }
}

void qualityOfServiceWindow::on_readTimeBox_valueChanged(int arg1)
{
    _autoReadTimer.setInterval(arg1*100);
}

void qualityOfServiceWindow::on_qualityOfServiceWindow_visibilityChanged(bool visible)
{
    if(visible == false)
    {
        _autoReadTimer.stop();
        ui->autoReadCheckBox->setChecked(false);
    }
}

