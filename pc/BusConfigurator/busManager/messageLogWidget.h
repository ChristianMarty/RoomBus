#ifndef MESSAGELOGWIDGET_H
#define MESSAGELOGWIDGET_H

#include <QWidget>
#include "busDevice.h"
#include "protocol/messageLogProtocol.h"

namespace Ui {
class messageLogWidget;
}

class messageLogWidget : public QWidget
{
    Q_OBJECT

public:
    explicit messageLogWidget(QWidget *parent = nullptr);
    ~messageLogWidget();

    void assignDevice(busDevice *busDevice);
private slots:

    void on_newMessage(LogMessage msg);

    void on_sysMsgBox_stateChanged(int arg1);
    void on_sysWarnBox_stateChanged(int arg1);
    void on_sysErrBox_stateChanged(int arg1);
    void on_appMsgBox_stateChanged(int arg1);
    void on_appWarnBox_stateChanged(int arg1);
    void on_appErrBox_stateChanged(int arg1);

    void on_clearButton_clicked();

private:
    Ui::messageLogWidget *ui;

    void addMessage(LogMessage msg);

    void updateLog(void);

    MessageLogProtocol *_logProtocol = nullptr;

    uint8_t _filter;
    QString _log;
};

#endif // MESSAGELOGWIDGET_H
