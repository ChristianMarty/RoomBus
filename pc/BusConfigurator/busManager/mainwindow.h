#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "busAccess.h"
#include "busMonitor/busMonitor.h"
#include "busDevice.h"
#include "busDeviceWidget.h"
#include "busDeviceWindow.h"

#include "qualityOfServiceWindow.h"
#include <QList>

#include "tcpConnectionWidget.h"
#include "busConnectionWidget.h"

#include "settings.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_newData(void);
    void on_deviceTx(RoomBus::Message msg);

    void on_scanButton_clicked();
    void on_monitorButton_clicked();
    void on_qosButton_clicked();
    void on_busDeviceWindowShow(RoomBusDevice *device);

private:
    RoomBusDevice *addDevice(uint8_t address);
    RoomBusDevice *getDevice(uint8_t address);
    void updateDevices(void);

    Ui::MainWindow *ui;

    QList<RoomBusDevice*> _busDeviceList;
    QList<busDeviceWidget*> _busDeviceWidgetLsit;
    QMap<RoomBusDevice*,busDeviceWindow*> _busDeviceWindowMap;

    void addConnection(void);

    RoomBusAccess _busConnection;

    QList<busConnectionWidget*> _busConnectionWidget;

    busMonitor *_monitorWindow = nullptr;

    qualityOfServiceWindow *_qosWindow = nullptr;

    settings settings;

};

#endif // MAINWINDOW_H
