#ifndef ROOMBUS_H
#define ROOMBUS_H

#include <QObject>

class RoomBus : public QObject
{
    Q_OBJECT
public:
    explicit RoomBus(QObject *parent = nullptr);

signals:
};

#endif // ROOMBUS_H
