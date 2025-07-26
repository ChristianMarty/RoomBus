#include "connectionWidget.h"
#include "ui_connectionWidget.h"

ConnectionWidget::ConnectionWidget(RoomBusAccess &roomBusAccess, QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::ConnectionWidget)
    ,_roomBusAccess{roomBusAccess}
{
    ui->setupUi(this);
    _updateUi();

    connect(&_roomBusAccess, &RoomBusAccess::connectionChanged, this, &ConnectionWidget::on_connectionChanged);
}

ConnectionWidget::~ConnectionWidget()
{
    delete ui;
}

void ConnectionWidget::on_connectionChanged()
{
    _updateUi();
}

void ConnectionWidget::_updateUi()
{
    if(_roomBusAccess.isConnected()){
        ui->comStatusLabel->setText("Open");
        ui->comStatusLabel->setStyleSheet("color: green;");
        ui->pushButton_openClose->setText("Close");
    }else{
        ui->comStatusLabel->setText("Closed");
        ui->comStatusLabel->setStyleSheet("color: red;");
        ui->pushButton_openClose->setText("Open");
    }

    ui->nameLabel->setText(_roomBusAccess.getConnectionName());
    ui->connectionPathLabel->setText(_roomBusAccess.getConnectionPath());
}

void ConnectionWidget::on_pushButton_openClose_clicked()
{
    if(_roomBusAccess.isConnected()){
        _roomBusAccess.closeConnection();
    }else{
        _roomBusAccess.openConnection();
    }
}

