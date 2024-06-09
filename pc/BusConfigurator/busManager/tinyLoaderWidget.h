#ifndef TINYLOADERWIDGET_H
#define TINYLOADERWIDGET_H

#include <QWidget>
#include "busDevice.h"
#include "tinyLoader.h"

namespace Ui {
class tinyLoaderWidget;
}

class tinyLoaderWidget : public QWidget
{
    Q_OBJECT

public:
    explicit tinyLoaderWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~tinyLoaderWidget();

private slots:
    void on_uploadButton_clicked();

    void on_toolButton_clicked();

    void on_progressChange(uint8_t progress);
    void on_infoChange(void);
    void on_hexInfoChange(void);

    void on_busScanButton_clicked();
    void on_busScanMessage(QString message);

    void on_addressBox_valueChanged(int arg1);

    void on_getAppCrcButton_clicked();

    void on_clearButton_clicked();

    void on_pushButton_clicked();

    void on_abortBusScanButton_clicked();

    void on_updateAddressButton_clicked();

    void on_rebootButton_clicked();

    void on_tcpOpenButton_clicked();

    void on_tcpCloseButton_clicked();

    void on_tcpBridgeStatus(QString message, bool open, bool error);

    void on_button_appStart_clicked();

    void on_button_appStop_clicked();

    void on_button_getDeviceState_clicked();

    void on_button_getDeviceInfo_clicked();

    void on_toolButton_triggered(QAction *arg1);

private:
    Ui::tinyLoaderWidget *ui;

    tinyLoader *_tinyLoader = nullptr;
};

#endif // TINYLOADERWIDGET_H
