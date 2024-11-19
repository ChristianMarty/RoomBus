#include "busConnectionWidget.h"
#include "ui_busConnectionWidget.h"

busConnectionWidget::busConnectionWidget(RoomBusAccess &roomBusAccess, QWidget *parent)
    :QWidget(parent)
    ,ui(new Ui::busConnectionWidget)
    ,_roomBusAccess{roomBusAccess}
{
    ui->setupUi(this);
    _updateUi();

    connect(&_roomBusAccess, &RoomBusAccess::connectionChanged, this, &busConnectionWidget::on_connectionChanged);
}

busConnectionWidget::~busConnectionWidget()
{
    delete ui;
}

void busConnectionWidget::on_connectionChanged()
{
    _updateUi();
}

void busConnectionWidget::_updateUi()
{
    if(_roomBusAccess.isConnected()){
        ui->comStatusLabel->setText("Open");
        ui->comStatusLabel->setStyleSheet("color: green;");
    }else{
        ui->comStatusLabel->setText("Closed");
        ui->comStatusLabel->setStyleSheet("color: red;");
    }

    ui->nameLabel->setText(_roomBusAccess.getConnectionName());
    ui->connectionPathLabel->setText(_roomBusAccess.getConnectionPath());
}
