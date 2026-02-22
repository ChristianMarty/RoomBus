#ifndef ECHOWINDOW_H
#define ECHOWINDOW_H

#include <QWidget>
#include "busDevice.h"
#include <QTimer>
#include <QCloseEvent>

namespace Ui {
class echoWindow;
}

class EchoTestWidget: public QWidget
{
    Q_OBJECT

public:
    explicit EchoTestWidget(RoomBusDevice *busDevice, QWidget *parent = nullptr);
    ~EchoTestWidget();

    void closeEvent(QCloseEvent *event);

private slots:
    void on_sendButton_clicked();
    void on_echoReceive(QByteArray rxData);

    void on_autoEchoBox_stateChanged(int arg1);

    void on_sendTimer(void);
    void on_timeout(void);

    void on_clearButton_clicked();

    void on_spinBox_interval_valueChanged(int arg1);

    void on_comboBox_priority_currentIndexChanged(int index);

private:
    Ui::echoWindow *ui;

    RoomBusDevice *_busDevice = nullptr;

    QTimer _autoSendTimer;
    QTimer _timeoutTimer;
    QByteArray _lastSend;

    uint8_t _sendIndex = 0;
    uint64_t _sendTimestamp = 0;

    bool _waitForRx = false;

    uint32_t _sendCounter = 0;
    uint32_t _receiveCounter = 0;
    uint32_t _failCounter = 0;
    uint32_t _passCounter = 0;
    uint32_t _lostCounter = 0;

    MiniBus::Priority _priority = MiniBus::Priority::Low;
};

#endif // ECHOWINDOW_H
