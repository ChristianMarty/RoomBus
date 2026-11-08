#include "busMonitor.h"
#include "ui_busMonitor.h"

BusMonitor::BusMonitor(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::BusMonitor)
{
    ui->setupUi(this);

    ui->tableView->setModel(&_busMonitorModel);

    ui->tableView->setColumnWidth(0,100);
    ui->tableView->setColumnWidth(1,75);
    ui->tableView->setColumnWidth(2,75);
    ui->tableView->setColumnWidth(3,50);
    ui->tableView->setColumnWidth(4,150);
    ui->tableView->setColumnWidth(5,150);

    QHeaderView *verticalHeader = ui->tableView->verticalHeader();
    verticalHeader->setSectionResizeMode(QHeaderView::Fixed);
    verticalHeader->setDefaultSectionSize(38);
}

BusMonitor::~BusMonitor()
{
    delete ui;
}

void BusMonitor::append(const QList<MiniBus::Message> &message)
{
    _busMonitorModel.append(message);

    if(!ui->tableView->hasFocus()){
        ui->tableView->scrollToBottom();
    }

    ui->label_items->setText(QString("%1 items").arg(_busMonitorModel.rowCount()));
}

void BusMonitor::on_clearButton_clicked()
{
    _busMonitorModel.clear();
    ui->label_items->setText("0 items");
}
