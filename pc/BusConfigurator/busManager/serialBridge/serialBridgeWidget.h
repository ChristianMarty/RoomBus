#ifndef SERIALBRIDGEWIDGET_H
#define SERIALBRIDGEWIDGET_H

#include <QWidget>
#include "busDevice.h"
#include "serialBridge.h"

namespace Ui {
class SerialBridgeWidget;
}

class SerialBridgeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SerialBridgeWidget(RoomBusDevice *busDevice, QWidget *parent = nullptr);
    ~SerialBridgeWidget();

private slots:
    void on_tcpOpenButton_clicked();
    void on_tcpCloseButton_clicked();

    void on_tcpBridgeStatus(QString message, bool open, bool error);
    void on_busMessage(QString message);

private:
    Ui::SerialBridgeWidget *ui;
    SerialBridge *_tinyLoader = nullptr;
};

#endif // SERIALBRIDGEWIDGET_H
