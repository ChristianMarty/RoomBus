#ifndef EVENTSIGNALLINE_H
#define EVENTSIGNALLINE_H

#include <QWidget>

#include "protocol/eventProtocol.h"

namespace Ui {
class eventSignalLine;
}

class eventSignalLine : public QWidget
{
    Q_OBJECT

public:
    explicit eventSignalLine(eventSlot eventSlot, QWidget *parent = nullptr);
    ~eventSignalLine();

    void update(eventSlot eventSlot);

    uint8_t channel() const;

signals:
    void eventActive(uint8_t eventChannel, bool active);

private slots:
    void on_checkBox_stateChanged(int arg1);

private:
    Ui::eventSignalLine *ui;

    uint8_t _channel;
};

#endif // EVENTSIGNALLINE_H
