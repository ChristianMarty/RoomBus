#ifndef STATEREPORTLINE_H
#define STATEREPORTLINE_H

#include <QWidget>


namespace Ui {
class stateReportLine;
}

class stateReportLine : public QWidget
{
    Q_OBJECT

public:
    explicit stateReportLine(QString name, uint8_t channel, QWidget *parent = nullptr);
    ~stateReportLine();

    uint8_t channel() const;
    void setName(QString name);
    void setState(QString state);

private:

    Ui::stateReportLine *ui;

    uint8_t _channel;
};

#endif // STATEREPORTLINE_H
