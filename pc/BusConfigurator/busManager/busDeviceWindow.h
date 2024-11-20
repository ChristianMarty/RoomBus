#ifndef BUSDEVICEWINDOW_H
#define BUSDEVICEWINDOW_H

#include <QDockWidget>
#include "busDevice.h"

#include "settingsWidget.h"
#include "trigger/triggerWidget.h"
#include "event/eventWidget.h"
#include "state/stateReportWidget.h"
#include "value/valueWidget.h"
#include "fileTransferWidget.h"
#include "echoTestWidget.h"

#include "serialBridge/serialBridgeWidget.h"

#include <appPluginInterface.h>
#include <QPluginLoader>
#include <QMdiSubWindow>

namespace Ui {
class busDeviceWindow;
}

class busDeviceWindow : public QDockWidget
{
    Q_OBJECT

public:
    explicit busDeviceWindow(RoomBusDevice *device, QWidget *parent = nullptr);
    ~busDeviceWindow();

    void updateData(void);

    void updateStatus(void);

private slots:
    void on_triggerButton_clicked();

    void on_settingsButton_clicked();

    void on_stateButton_clicked();

    void on_echoButton_clicked();

    void on_valueButton_clicked();

    void on_fileButton_clicked();

    void on_rebootButton_clicked();

    void on_appConfig_clicked();

    void on_busTransmitMessage(RoomBus::Message msg);

    void on_appConfi_destroyed(QObject *obj = nullptr);

    void on_pushButton_serialBridge_clicked();

    void on_pushButton_event_clicked();

private:
    Ui::busDeviceWindow *ui;
    RoomBusDevice *_device;

    settingsWidget *_settingsWidget = nullptr;
    echoTestWidget *_echoWidget = nullptr;
    TriggerWidget *_triggerWidget = nullptr;
    EventWidget *_eventWidget = nullptr;
    StateReportWidget *_stateReportWidget = nullptr;
    ValueWidget *_valueReportWidget = nullptr;
    fileTransferWidget *_fileTransferWidget = nullptr;

    SerialBridgeWidget *_tinyLoaderWidget = nullptr;

    QMdiSubWindow *_settingsWindow = nullptr;
    QMdiSubWindow *_echoWindow = nullptr;
    QMdiSubWindow *_triggerWindow = nullptr;
    QMdiSubWindow *_eventWindow = nullptr;
    QMdiSubWindow *_stateReportWindow = nullptr;
    QMdiSubWindow *_valueReportWindow = nullptr;
    QMdiSubWindow *_fileTransferWindow = nullptr;
    QMdiSubWindow *_serialBridgeWindow = nullptr;

    appPluginInterface *_appConfigInterface = nullptr;
    QWidget *_appConfigWidget = nullptr;
    QPluginLoader* _plugin = nullptr;
    QMdiSubWindow *_pluginWindow = nullptr;

    void _loadPlugin(QString path);
    void _unloadPlugin();
};

#endif // BUSDEVICEWINDOW_H
