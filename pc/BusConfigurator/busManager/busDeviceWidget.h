#ifndef BUSDEVICEWIDGET_H
#define BUSDEVICEWIDGET_H

#include <QWidget>

#include "busDevice.h"

namespace Ui {
class BusDeviceWidget;
}

class BusDeviceWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BusDeviceWidget(RoomBusDevice *device, QWidget *parent = nullptr);
    ~BusDeviceWidget();

    void updateData(void);

signals:
    void busDeviceShow(RoomBusDevice *device);

private slots:
    void on_identifyButton_clicked();
    void on_showButton_clicked();

private:
    Ui::BusDeviceWidget *ui;
    RoomBusDevice *_device;
};

#endif // BUSDEVICEWIDGET_H
