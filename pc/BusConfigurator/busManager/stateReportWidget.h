#ifndef STATEREPORTWIDGET_H
#define STATEREPORTWIDGET_H

#include <QDialog>
#include "busDevice.h"
#include "protocol/stateReportProtocol.h"
#include "stateReportLine.h"

namespace Ui {
class stateReportWidget;
}

class stateReportWidget : public QDialog
{
    Q_OBJECT

public:
    explicit stateReportWidget(busDevice *busDevice, QWidget *parent = nullptr);
    ~stateReportWidget();

private slots:
    void on_updateListButton_clicked();

    void on_nameChange(void);

    void on_stateChange(void);

    void on_readButton_clicked();

    void on_clearButton_clicked();

private:

    void updateStateChannelList(QMap<uint8_t, QString> stateChannelList);

    Ui::stateReportWidget *ui;
    busDevice *_busDevice;

    QList<stateReportLine*> _stateReportLines;

    stateReportProtocol _stateReportProtocol;


};

#endif // STATEREPORTWIDGET_H
