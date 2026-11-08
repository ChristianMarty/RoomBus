#ifndef BUS_MONITOR_MODEL_H
#define BUS_MONITOR_MODEL_H

#include <QAbstractTableModel>
#include "miniBusMessage.h"

class BusMonitorModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit BusMonitorModel( QObject *parent = nullptr);

    // Header:
    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    // Basic functionality:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    void clear(void);

    void append(const QList<MiniBus::Message> &messages);

private:
    QList<MiniBus::Message> _data;
};

#endif // BUS_MONITOR_MODEL_H
