#ifndef UNFOLDEDCIRCLEREMOTE_H
#define UNFOLDEDCIRCLEREMOTE_H

#include <QObject>
#include <QWebSocketServer>
#include <QWebSocket>

#include "entity/entity.h"
#include "driver.h"

namespace UnfoldedCircle
{

class Remote : public QObject
{
    Q_OBJECT
public:
    explicit Remote(QWebSocket *socket);

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

    void addEntity(Entity *entity);
    void sendEntityChange(QJsonObject data);

    friend Entity;

private slots:
    void on_textMessage(QString message);
    void on_binaryMessage(QByteArray message);
    void on_socketDisconnected(void);

private:
    QWebSocket *_socket;
    QMap<int, Entity *> _entities;

    void _getDriverVersionHandler(uint32_t id);
    void _getAvailableEntitiesHandler(uint32_t id);
    void _entityCommandHandler(uint32_t id, QJsonObject data);

    void _sendResponse(uint32_t id, Response response, QJsonObject data);
    void _sendEvent(Event event, QJsonObject data);
};


class Server : public QObject
{
    Q_OBJECT
public:
    explicit Server(QObject *parent = nullptr);
    ~Server();

    void registerDriver(QString remoteIp, uint32_t pin);

signals:

private slots:
    void on_newConnection(void);

private:
    QWebSocketServer _server{QStringLiteral("UnfoldedCircleRemote2Integration"),QWebSocketServer::NonSecureMode, this};

    uint32_t _port = 9999;
    QSet<Remote*> _remotes;
    Driver _driver{"192.168.1.138", _port};
};

}
#endif // UNFOLDEDCIRCLEREMOTE_H