#ifndef TRIGGERLINE_H
#define TRIGGERLINE_H

#include <QWidget>

namespace Ui {
class triggerLine;
}

class triggerLine : public QWidget
{
    Q_OBJECT

public:
    explicit triggerLine(QString name, uint8_t channel, QWidget *parent = nullptr);
    ~triggerLine();

    uint8_t channel() const;
    void setName(QString name);

signals:
    void trigger(uint8_t triggerChannel);

private slots:
    void on_triggerButton_clicked();

private:
    Ui::triggerLine *ui;

    uint8_t _channel;
};

#endif // TRIGGERLINE_H
