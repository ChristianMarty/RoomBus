#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QWidget>
#include "busDevice.h"
#include <QFileSystemWatcher>

namespace Ui {
class settingsWidget;
}

class settingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit settingsWidget(RoomBusDevice *busDevice, QWidget *parent = nullptr);
    ~settingsWidget();

    void updateData(void);

private slots:
    void on_setNameButton_clicked();

    void on_setAddressButton_clicked();

    void on_ledOnBox_stateChanged(int arg1);

    void on_appRunBox_stateChanged(int arg1);

    void on_autostartAppBox_stateChanged(int arg1);

    void on_setIntervalButton_clicked();

    void on_firmwarePathButton_clicked();

    void on_firmwareUploadButton_clicked();

    void on_bootloadStatusUpdate(uint8_t progress, bool error, QString message);

    void on_statusUpdate(void);

    void on_bootloadFileChanged(const QString &path);

    void on_appRunBox_clicked(bool checked);

    void on_autostartAppBox_clicked(bool checked);

    void on_ledOnBox_clicked(bool checked);

    void on_clearLog_button_clicked();

private:
    Ui::settingsWidget *ui;

    RoomBusDevice *_busDevice;

    QFileSystemWatcher _bootloadFileWatcher;
};

#endif // SETTINGSWIDGET_H
