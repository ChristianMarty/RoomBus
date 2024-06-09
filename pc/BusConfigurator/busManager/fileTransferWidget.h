#ifndef FILETRANSFERWIDGET_H
#define FILETRANSFERWIDGET_H

#include <QWidget>
#include "busDevice.h"
#include "protocol/fileTransferProtocol.h"

#include <QStandardItemModel>

namespace Ui {
class fileTransferWidget;
}

class fileTransferWidget : public QWidget
{
    Q_OBJECT

public:
    explicit fileTransferWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~fileTransferWidget();

private slots:

    void on_transfereStatus_change(fileTransferProtocol::transfereStatus_t status, uint8_t progress);

    void on_fileList_change(QMap<QString, fileTransferProtocol::file_t> files);

    void on_makeFileButton_clicked();

    void on_readButton_clicked();

    void on_writeButton_clicked();

    void on_deleteButton_clicked();

    void on_reloadButton_clicked();

    void on_localPathToolButton_clicked();

    void on_treeView_clicked(const QModelIndex &index);

private:
    Ui::fileTransferWidget *ui;
    fileTransferProtocol _fileTransferProtocol;

    QStandardItemModel fileTree;
    QList<QList<QStandardItem *>> fileItems;

    QString _selectedRemotePath;

};

#endif // FILETRANSFERWIDGET_H
