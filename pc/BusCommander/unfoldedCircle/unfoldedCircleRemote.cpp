#include "unfoldedCircleRemote.h"
#include "unfoldedCircle/entity/light.h"
#include "unfoldedCircle/entity/switch.h"

using namespace UnfoldedCircle;

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

Server::Server(QObject *parent)
    : QObject{parent}
{

    connect(&_server, &QWebSocketServer::newConnection, this, &Server::on_newConnection);
    _server.listen(QHostAddress::Any, _port);

    qDebug(("Server listening on port "+ QString::number(_port)).toLocal8Bit());
}

Server::~Server()
{
    qDeleteAll(_remotes);
}

void Server::registerDriver(QString remoteIp, uint32_t pin)
{
    _driver.sendDriverRegistration(remoteIp, pin);
}


void Server::on_newConnection()
{
    QWebSocket *socket = _server.nextPendingConnection();
    _remotes.insert(new Remote(socket));
}

Remote::Remote(QWebSocket *socket):
    _socket{socket}
{
    connect(_socket, &QWebSocket::textMessageReceived, this, &Remote::on_textMessage);
    connect(_socket, &QWebSocket::binaryMessageReceived, this, &Remote::on_binaryMessage);
    connect(_socket, &QWebSocket::disconnected, this, &Remote::on_socketDisconnected);

    addEntity(new Light{"Licht 1",1, QSet<Light::Feature>{Light::Feature::Toggle}});

    QString msg = socket->peerAddress().toString();
    qDebug(msg.toLocal8Bit());
}

void Remote::addEntity(Entity *entity)
{
    entity->setRemote(this);
    _entities.insert(entity->entityId(), entity);
}

void Remote::on_textMessage(QString message)
{
    QString debug = "Text :"+QString(message);
    qDebug(debug.toLocal8Bit());

    QJsonDocument jsonResponse = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject jsonObject = jsonResponse.object();

    QString kind = jsonObject["kind"].toString();
    uint32_t id = jsonObject["id"].toInt();
    QJsonObject data = jsonObject["msg_data"].toObject();

    if(kind == "req"){
        QString msg = jsonObject["msg"].toString();
        Requests requests = unknownRequests;
        if(msg == "get_driver_version") requests = getDriverVersion;
        else if(msg == "get_device_state") requests = getDeviceState;
        else if(msg == "get_available_entities") requests = getAvailableEntities;
        else if(msg == "subscribe_events") requests = subscribeEvents;
        else if(msg == "get_entity_states") requests = getEntityStates;
        else if(msg == "entity_command") requests = entityCommand;

        switch(requests){
            case getDriverVersion :_getDriverVersionHandler(id); break;
            case getAvailableEntities :_getAvailableEntitiesHandler(id); break;
            case entityCommand: _entityCommandHandler(id, data); break;
        }
    }
}

void Remote::on_binaryMessage(QByteArray message)
{
    QString msg = "WebSocket Binary Message :"+QString(message);
    qWarning(msg.toLocal8Bit());
}

void Remote::on_socketDisconnected()
{

}

void Remote::_getDriverVersionHandler(uint32_t id)
{
    QJsonObject version {
       // {"api", "0.10.0"},
        {"driver", "0.0.1"}
    };
    QJsonObject data{
        {"name", "RoomBus"},
        {"version", version}
    };
    _sendResponse(id, driverVersion, data);
}

void Remote::_getAvailableEntitiesHandler(uint32_t id)
{
    QJsonArray entities;

    for(const Entity *entity: _entities){
        entities.append(entity->json());
    }

    QJsonObject data{
        {"available_entities", entities}
    };
    _sendResponse(id, availableEntities, data);
}

void Remote::_entityCommandHandler(uint32_t id, QJsonObject data)
{
    uint32_t entityId = data["entity_id"].toString().toInt();
    if(!_entities.contains(entityId)){
        // todo: send error response
        return;
    }

    _sendResponse(id, Response::result, QJsonObject{});

    _entities[entityId]->commandHandler(data);
}

void Remote::_sendResponse(uint32_t id, Response response, QJsonObject data)
{
    QString msg;
    switch(response){
        case driverVersion: msg = "driver_version"; break;
        case deviceState: msg = "device_state"; break;
        case availableEntities: msg = "available_entities"; break;
        case result: msg = "result"; break;
        case entityStates: msg = "entity_states"; break;
        default:
            return;
    }

    QJsonObject json {
        {"kind", "resp"},
        {"req_id", QJsonValue((int )id)},
        {"msg", msg},
        {"code", 200}, // todo: ahhh
        {"msg_data", data}
    };

    QJsonDocument frame;
    frame.setObject(json);

    _socket->sendTextMessage(frame.toJson());

    QString debug = "Send "+msg+": "+frame.toJson();
    qDebug(debug.toLocal8Bit());
}

void Remote::_sendEvent(Event event, QJsonObject data)
{
    QString msg;
    switch(event){
        case entityChangeEvent: msg = "entity_change"; break;
        case deviceStateEvent: msg = "device_state"; break;
    default:
        return;
    }

    QJsonObject json {
        {"kind", "event"},
        {"msg", msg},
        {"cat", "ENTITY"},
        {"msg_data", data}
    };

    QJsonDocument frame;
    frame.setObject(json);

    _socket->sendTextMessage(frame.toJson());

    QString debug = "Send "+msg+": "+frame.toJson();
    qDebug(debug.toLocal8Bit());
}

