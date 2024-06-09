#ifndef BUSDEVICEWIDGET_H
#define BUSDEVICEWIDGET_H

#include <QWidget>

#include "busDevice.h"

namespace Ui {
class busDeviceWidget;
}

class busDeviceWidget : public QWidget
{
    Q_OBJECT

public:
    explicit busDeviceWidget(busDevice *device, QWidget *parent = nullptr);
    ~busDeviceWidget();

    void updateData(void);

signals:
    void busDeviceShow(busDevice *device);

private slots:
    void on_identifyButton_clicked();
    void on_showButton_clicked();

private:
    Ui::busDeviceWidget *ui;
    busDevice *_device;


};

#endif // BUSDEVICEWIDGET_H
