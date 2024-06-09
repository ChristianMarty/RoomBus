#ifndef ECHOWINDOW_H
#define ECHOWINDOW_H

#include <QWidget>
#include "busDevice.h"
#include <QTimer>
#include <QCloseEvent>

namespace Ui {
class echoWindow;
}

class echoTestWidget: public QWidget
{
    Q_OBJECT

public:
    explicit echoTestWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~echoTestWidget();

    void closeEvent(QCloseEvent *event);

private slots:
    void on_sendButton_clicked();
    void on_echoReceive(QByteArray rxData);

    void on_autoEchoBox_stateChanged(int arg1);

    void on_sendTimer(void);
    void on_timeout(void);

    void on_spinBox_valueChanged(int arg1);

    void on_clearButton_clicked();

    void on_timeoutBox_valueChanged(int arg1);

private:
    Ui::echoWindow *ui;

    busDevice *_busDevice = nullptr;

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

};

#endif // ECHOWINDOW_H
