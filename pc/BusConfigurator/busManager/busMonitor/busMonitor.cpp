#include "busMonitor.h"
#include "ui_busMonitor.h"
#include <QTableWidget>
#include <QDateTime>
#include "busMessage.h"

#include "busMonitorLine.h"

busMonitor::busMonitor(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::busMonitor)
{
    ui->setupUi(this);
}

busMonitor::~busMonitor()
{
    delete ui;
}

void busMonitor::on_newMessage(busMessage msg)
{
    QListWidgetItem *item = new QListWidgetItem(ui->listWidget);
    BusMonitorLine *lineWidget = new BusMonitorLine(msg, ui->listWidget);
    item->setSizeHint(lineWidget->sizeHint());
    ui->listWidget->setItemWidget(item,lineWidget);

    if(!ui->listWidget->hasFocus()){
        ui->listWidget->scrollToBottom();
    }


}

void busMonitor::on_clearButton_clicked()
{
    ui->listWidget->clear();
}
