#ifndef BUSDEVICEWINDOW_H
#define BUSDEVICEWINDOW_H

#include <QDockWidget>
#include "busDevice.h"

#include "settingsWidget.h"
#include "triggerWidget.h"
#include "stateReportWidget.h"
#include "valueReportWidget.h"
#include "fileTransferWidget.h"
#include "echoTestWidget.h"

#include "tinyLoaderWidget.h"

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
    explicit busDeviceWindow(busDevice *device, QWidget *parent = nullptr);
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

    void on_busTransmitMessage(busMessage msg);

    void on_appConfi_destroyed(QObject *obj = nullptr);


    void on_tinyLoaderButton_clicked();

private:
    Ui::busDeviceWindow *ui;
    busDevice *_device;

    settingsWidget *_settingsWidget = nullptr;
    echoTestWidget *_echoWidget = nullptr;
    triggerWidget *_triggerWidget = nullptr;
    stateReportWidget *_stateReportWidget = nullptr;
    valueReportWidget *_valueReportWidget = nullptr;
    fileTransferWidget *_fileTransferWidget = nullptr;

    tinyLoaderWidget *_tinyLoaderWidget = nullptr;

    QMdiSubWindow *_settingsWindow = nullptr;
    QMdiSubWindow *_echoWindow = nullptr;
    QMdiSubWindow *_triggerWindow = nullptr;
    QMdiSubWindow *_stateReportWindow = nullptr;
    QMdiSubWindow *_valueReportWindow = nullptr;
    QMdiSubWindow *_fileTransferWindow = nullptr;
    QMdiSubWindow *_tinyLoaderWindow = nullptr;

    appPluginInterface *_appConfigInterface = nullptr;
    QWidget *_appConfigWidget = nullptr;
    QPluginLoader* _plugin = nullptr;
    QMdiSubWindow *_pluginWindow = nullptr;

    void _loadPlugin(QString path);
    void _unloadPlugin();
};

#endif // BUSDEVICEWINDOW_H
