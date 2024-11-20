#include "fileTransferWidget.h"
#include "ui_fileTransferWidget.h"
#include <QFileDialog>
fileTransferWidget::fileTransferWidget(RoomBusDevice *busDevice, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::fileTransferWidget),
    _fileTransferProtocol(busDevice)
{
    ui->setupUi(this);

    connect(&_fileTransferProtocol, &FileTransferProtocol::fileList_change, this, &fileTransferWidget::on_fileList_change);

    connect(&_fileTransferProtocol, &FileTransferProtocol::readTransfereStatus_change, this, &fileTransferWidget::on_transfereStatus_change);
    connect(&_fileTransferProtocol, &FileTransferProtocol::writeTransfereStatus_change, this, &fileTransferWidget::on_transfereStatus_change);

    ui->treeView->setModel(&fileTree);

    fileTree.setHorizontalHeaderLabels(QStringList({"Type","Name","Size"}));
}

fileTransferWidget::~fileTransferWidget()
{
    delete ui;
}


void fileTransferWidget::on_fileList_change(QMap<QString, FileTransferProtocol::file_t> files)
{

    for(int i=0; i<fileItems.count(); ++i)
    {
        for(int j=0; j<fileItems[i].count(); ++j)
        {
            QStandardItem *temp = fileItems[i][j];
            delete temp;
        }

    }
    fileItems.clear();

    fileTree.clear();
    fileTree.setHorizontalHeaderLabels(QStringList({"Type","Name","Size"}));


    foreach (const QString &str, files.keys())
    {
        QList<QStandardItem *> temp;

        if(files.value(str).type == FileTransferProtocol::file_t::dir) temp.append(new QStandardItem("DIR"));
        if(files.value(str).type == FileTransferProtocol::file_t::file) temp.append(new QStandardItem("FILE"));

        temp.append(new QStandardItem(files.value(str).name));
        temp.append(new QStandardItem(QString::number(files.value(str).size, 10)));

        fileItems.append(temp);
        fileTree.appendRow(temp);
    }

}

void fileTransferWidget::on_makeFileButton_clicked()
{
    _fileTransferProtocol.makeFile(ui->makeEdit->text());
}

void fileTransferWidget::on_deleteButton_clicked()
{
    _fileTransferProtocol.deleteFile(_selectedRemotePath);
}

void fileTransferWidget::on_readButton_clicked()
{
    _fileTransferProtocol.readFile(_selectedRemotePath, ui->localPathEdit->text());
}

void fileTransferWidget::on_writeButton_clicked()
{
    _fileTransferProtocol.writeFile(_selectedRemotePath, ui->localPathEdit->text());
}

void fileTransferWidget::on_treeView_clicked(const QModelIndex &index)
{
    QModelIndex temp = fileTree.index(index.row(),1);

    QString path = fileTree.itemFromIndex(temp)->text();

    _selectedRemotePath = path;
}

void fileTransferWidget::on_reloadButton_clicked()
{
    _fileTransferProtocol.list("/");
}

void fileTransferWidget::on_localPathToolButton_clicked()
{
     ui->localPathEdit->setText(QFileDialog::getOpenFileName(this,tr("Select File Path"), "", tr("All files (*.*)")));
}

void fileTransferWidget::on_transfereStatus_change(FileTransferProtocol::transfereStatus_t status, uint8_t progress)
{
    ui->progressBar->setValue(progress);

    QString statusText;

    switch(status)
    {
        case FileTransferProtocol::idel:      statusText = "Idel"; break;
        case FileTransferProtocol::start:     statusText = "Start"; break;
        case FileTransferProtocol::transfere: statusText = "Transferring"; break;
        case FileTransferProtocol::verify:    statusText = "Verify"; break;
        case FileTransferProtocol::complete:  statusText = "Completed"; break;
        case FileTransferProtocol::error:     statusText = "Error"; break;
    }

    ui->statusLabel->setText(statusText);
}

