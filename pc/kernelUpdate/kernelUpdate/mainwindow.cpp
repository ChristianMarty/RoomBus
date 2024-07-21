#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->updater_lineEdit->setText("C:\\Users\\Christian\\Raumsteuerung\\uc\\kernelUpdater\\kernelUpdater\\Debug\\kernelUpdater.hex");
    ui->kernel_lineEdit->setText("C:\\Users\\Christian\\Raumsteuerung\\uc\\kernel\\kernel\\Debug\\kernel.hex");

    _savePath = "C:\\Users\\Christian\\Raumsteuerung\\uc\\kernel\\kernel\\Debug\\kernelupdate.hex";

    _loadUpdater();
    _loadKernel();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_merge_button_clicked()
{
    uint32_t _newKernelOffset = 0x10C00;

    QByteArray kernelStart;
    kernelStart.append((char)(_newKernelOffset&0xFF));
    kernelStart.append((char)((_newKernelOffset>>8)&0xFF));
    kernelStart.append((char)((_newKernelOffset>>16)&0xFF));
    kernelStart.append((char)((_newKernelOffset>>24)&0xFF));

    _updaterBinary.replace(0x00010064,kernelStart);

    uint32_t size = _kernelBinary.binary().at(0).data.count()-4;
    QByteArray sizeArray;
    sizeArray.append((char)(size&0xFF));
    sizeArray.append((char)((size>>8)&0xFF));
    sizeArray.append((char)((size>>16)&0xFF));
    sizeArray.append((char)((size>>24)&0xFF));

    _updaterBinary.replace(0x00010068,sizeArray);


    _output.clear();
    _output.insert(_updaterBinary.binary().at(0));

    QuCLib::HexFileParser::binaryChunk kernel = _kernelBinary.binary().at(0);
    kernel.offset += _newKernelOffset;

    _output.insert(kernel);
    _output.saveToFile(_savePath);
}

void MainWindow::on_selectUpdater_button_clicked()
{
    ui->updater_lineEdit->setText(QFileDialog::getOpenFileName(this,tr("Open HEX File"), "", tr("Hex Files (*.hex)|All files (*.*)")));
    _loadUpdater();
}

void MainWindow::on_selectKernel_button_clicked()
{
     ui->kernel_lineEdit->setText(QFileDialog::getOpenFileName(this,tr("Open HEX File"), "", tr("Hex Files (*.hex)|All files (*.*)")));
    _loadKernel();
}

void MainWindow::_updateValues()
{
    ui->uploaderStart_label->setText("0x"+QString::number(_updaterBinary.addressFileMinimum(),16));
    ui->uploaderEnd_label->setText("0x"+QString::number(_updaterBinary.addressFileMaximum(),16));

    ui->kernelStart_label->setText("0x"+QString::number(_kernelBinary.addressFileMinimum(),16));
    ui->kernelEnd_label->setText("0x"+QString::number(_kernelBinary.addressFileMaximum(),16));
}

void MainWindow::_loadUpdater()
{
    _updaterBinary.clear();
    _updaterBinary.setAddressGapSize(0x1000);
    _updaterBinary.setAddressAlignment(1);

    _updaterBinary.load(ui->updater_lineEdit->text());

    _updateValues();
}

void MainWindow::_loadKernel()
{

    _kernelBinary.clear();
    _kernelBinary.setAddressGapSize(0x1000);
    _kernelBinary.setAddressAlignment(1);

    _kernelBinary.load(ui->kernel_lineEdit->text());

    _updateValues();
}
