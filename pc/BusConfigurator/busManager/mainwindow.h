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

#include "connection/tcpConnectionWidget.h"
#include "connection/connectionWidget.h"

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
    QList<BusDeviceWidget*> _busDeviceWidgetList;
    QMap<RoomBusDevice*, BusDeviceWindow*> _busDeviceWindowMap;

    void addConnection(void);

    RoomBusAccess _busConnection;
    ConnectionWidget *_busConnectionWidget = new ConnectionWidget(_busConnection);

    busMonitor *_monitorWindow = nullptr;

    qualityOfServiceWindow *_qosWindow = nullptr;

    Settings _settings;

};

#endif // MAINWINDOW_H
