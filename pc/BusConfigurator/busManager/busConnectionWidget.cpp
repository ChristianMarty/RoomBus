#include "busConnectionWidget.h"
#include "ui_busConnectionWidget.h"

busConnectionWidget::busConnectionWidget(busAccess *com,QWidget *parent) :
    QWidget(parent),
    ui(new Ui::busConnectionWidget)
{
    ui->setupUi(this);
    _com = com;

    setStatusLabel(_com->getIsConnected());
    ui->nameLabel->setText(_com->getConnectionName());
    ui->connectionPathLabel->setText(_com->getConnectionPath());


     connect(_com, &busAccess::connectionChanged, this, &busConnectionWidget::on_connectionChanged);
}

busConnectionWidget::~busConnectionWidget()
{
    disconnect(_com, &busAccess::connectionChanged, this, &busConnectionWidget::on_connectionChanged);
    delete ui;
}

void busConnectionWidget::on_connectionChanged(bool connected)
{
    setStatusLabel(connected);
    ui->nameLabel->setText(_com->getConnectionName());
    ui->connectionPathLabel->setText(_com->getConnectionPath());
}

void busConnectionWidget::setStatusLabel(bool open)
{
    if(open)
    {
        ui->comStatusLabel->setText("Open");
        ui->comStatusLabel->setStyleSheet("color: green;");
    }
    else
    {
        ui->comStatusLabel->setText("Closed");
        ui->comStatusLabel->setStyleSheet("color: red;");
    }
}
