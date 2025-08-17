#ifndef BUSDEVICEWINDOW_H
#define BUSDEVICEWINDOW_H

#include <QDockWidget>
#include "busDevice.h"

namespace Ui {
class BusDeviceWindow;
}

class BusDeviceWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit BusDeviceWindow(RoomBusDevice *device, QWidget *parent = nullptr);
    ~BusDeviceWindow();

    void updateData(void);

    void updateStatus(void);

private slots:
    void on_statusUpdate(void);

    void on_triggerButton_clicked();

    void on_settingsButton_clicked();

    void on_stateButton_clicked();

    void on_echoButton_clicked();

    void on_valueButton_clicked();

    void on_fileButton_clicked();

    void on_rebootButton_clicked();

    void on_pushButton_serialBridge_clicked();

    void on_pushButton_event_clicked();

    void on_pushButton_identify_clicked();

    void on_pushButton_eeprom_clicked();

private:
    Ui::BusDeviceWindow *ui;
    RoomBusDevice *_device;

    void _loadPlugin(QString path);
    void _unloadPlugin();
};

#endif // BUSDEVICEWINDOW_H
