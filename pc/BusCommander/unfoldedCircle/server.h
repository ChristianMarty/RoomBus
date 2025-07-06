#ifndef SERVER_H
#define SERVER_H

#include <QObject>
#include <QWebSocket>

#include "entity/entity.h"
#include "driver.h"
#include "remote.h"

class RoomBusInterface;
namespace UnfoldedCircle
{
class Event;
class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(RoomBusInterface *roomBusInterface = nullptr, QObject *parent = nullptr);
    ~Server();

    void registerDriver(QString remoteIp, uint32_t pin);

    void addEntity(Entity *entity);

    Entity *entity(int entityId);
    QMap<int, Entity *> entities() const;

    RoomBusInterface *roomBusInterface();

    void sendEvent(Remote::Event event, QJsonObject data);

private slots:
    void on_newConnection(void);
    void on_newRoomBusData(void);

    void on_remoteDisconnected(Remote* remote);

private:
    RoomBusInterface *_roomBusInterface = nullptr;
    QWebSocketServer _server{QStringLiteral("UnfoldedCircleRemote2Integration"),QWebSocketServer::NonSecureMode, this};

    QMap<int, Entity *> _entities;

    uint32_t _port = 9999;
    QSet<Remote*> _remotes;
    Driver _driver{"192.168.1.138", _port};
};

}
#endif // SERVER_H
