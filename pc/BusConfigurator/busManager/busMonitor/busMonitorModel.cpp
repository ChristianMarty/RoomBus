#include "busMonitorModel.h"
#include <QDateTime>
#include "protocol/protocolBase.h"

BusMonitorModel::BusMonitorModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

QVariant BusMonitorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role != Qt::DisplayRole) return QVariant();
    if (orientation != Qt::Horizontal) return QVariant();

    switch(section) {
        case 0:
            return QString("Timestamp");
        case 1:
            return QString("Source");
        case 2:
            return QString("Destination");
        case 3:
            return QString("Priority");
        case 4:
            return QString("Protocol");
        case 5:
            return QString("Command");
        case 6:
            return QString("Data");
        default:
            return QVariant();
    }
}

int BusMonitorModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }

    return _data.count();
}

int BusMonitorModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()){
        return 0;
    }

    return 7;
}

QVariant BusMonitorModel::data(const QModelIndex &index, int role) const
{
    if(!index.isValid()) return QVariant();
    if((index.column()  < 4) && role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter;
    }

    if (role != Qt::DisplayRole) return QVariant();

    int rowIndex = index.row();
    int columnIndex = index.column();

    if(rowIndex >= _data.count()) return QVariant();
    const MiniBus::Message &item = _data.at(rowIndex);

    switch(columnIndex){
        case 0: return QVariant(QDateTime::currentDateTime().fromMSecsSinceEpoch(item.timestamp).toString("hh:mm:ss.zzz"));
        case 1: return QVariant(item.sourceAddress);
        case 2: return QVariant(item.destinationAddress);
        case 3: return QVariant(ProtocolBase::priorityName(item.priority));
        case 4: return QVariant(ProtocolBase::protocolName(item.protocol));
        case 5: return QVariant(ProtocolBase::commandName(item.protocol, item.command));
        case 6: return QVariant((QString)item.data.toHex(' ').toUpper()+"\n"+ProtocolBase::dataDecoder(item.protocol, item.command, item.data));
    }

    return QVariant();
}

void BusMonitorModel::clear()
{
    int first = 0;
    int last = _data.count()-1;
    if(last<=first)return;

    beginRemoveRows(QModelIndex(), first, last);
    _data.clear();
    endRemoveRows();
}

void BusMonitorModel::append(const QList<MiniBus::Message> &messages)
{
    int first = _data.count();
    int last = first+messages.count()-1;
    if(last<first)return;

    beginInsertRows(QModelIndex(), first, last);
    _data.append(messages);
    endInsertRows();
}
