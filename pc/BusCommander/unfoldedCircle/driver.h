#ifndef DRIVER_H
#define DRIVER_H

#include <QObject>
#include <QDate>

namespace UnfoldedCircle
{

struct Version {
    uint8_t major = 0;
    uint8_t minor = 0;
    uint8_t patch = 1;
};

class Driver : public QObject
{
    Q_OBJECT
public:
    explicit Driver(QString ip, uint32_t port);

    void sendDriverRegistration(QString remoteIp, uint32_t pin);

    Version version() const;

private slots:

private:
    QString _driverId = "RoomBus";
    QString _name = "RoomBus";
    QString _description = "UnfoldedCircle RoomBus Driver";
    QString _driverUrl;
    Version _version = {
        .major = 0,
        .minor = 1,
        .patch = 0,
    };

    bool _deviceDiscovery = false;
    QDate _releaseDate = QDate::currentDate();
};
}
#endif // DRIVER_H
