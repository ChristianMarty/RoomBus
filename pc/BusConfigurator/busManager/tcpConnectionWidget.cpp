#include "tcpConnectionWidget.h"
#include "ui_tcpConnectionWidget.h"

tcpConnectionWidget::tcpConnectionWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::tcpConnectionWidget)
{
    ui->setupUi(this);
}

tcpConnectionWidget::~tcpConnectionWidget()
{
    delete ui;
}

void tcpConnectionWidget::on_connectButton_clicked()
{

}
