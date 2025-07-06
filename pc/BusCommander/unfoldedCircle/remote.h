#ifndef REMOTE_H
#define REMOTE_H

#include <QObject>

#include "entity/entity.h"
#include "driver.h"

#include <QWebSocketServer>
#include <QWebSocket>

class RoomBusInterface;
namespace UnfoldedCircle
{
class Server;
class Remote : public QObject
{
    Q_OBJECT
public:
    explicit Remote(Server &server, RoomBusInterface *roomBusInterface, QWebSocket *socket);
    ~Remote(void);

    enum MessageKind {
        EventKind,
        ReqestKind,
        ResponseKind
    };

    enum Requests{
        unknownRequests,
        getDriverVersion,
        getDeviceState,
        getAvailableEntities,
        subscribeEvents,
        getEntityStates,
        entityCommand
    };

    enum Response{
        driverVersion,
        deviceState,
        availableEntities,
        result,
        entityStates
    };
    enum Event{
        entityChangeEvent,
        deviceStateEvent
    };

    void sendEntityChange(QJsonObject data);

    void sendEvent(Event event, QJsonObject data);

    friend Entity;

    RoomBusInterface *roomBusInterface();
    void pushRoomBusMessage(const RoomBus::Message &message);

signals:
    void disconnected(Remote* remote);

private slots:
    void on_socketConnected(void);
    void on_socketDisconnected(void);

    void on_textMessage(QString message);
    void on_binaryMessage(QByteArray message);

private:
    QWebSocket *_socket;
    Server &_server;
    RoomBusInterface *_roomBusInterface = nullptr;

    void _getDriverVersionHandler(uint32_t id);
    void _getAvailableEntitiesHandler(uint32_t id);
    void _entityCommandHandler(uint32_t id, QJsonObject data);

    void _sendResponse(uint32_t id, Response response, QJsonObject data);

};

}
#endif // REMOTE_H
