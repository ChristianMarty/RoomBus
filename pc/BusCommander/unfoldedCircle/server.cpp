#include "server.h"
#include "remote.h"

#include "unfoldedCircle/entity/light.h"
#include "unfoldedCircle/entity/switch.h"

#include "roomBus/roomBus.h"

#include "entity/entity.h"

using namespace UnfoldedCircle;

Server::Server(RoomBusInterface *roomBusInterface, QObject *parent)
    : QObject{parent}
    ,_roomBusInterface{roomBusInterface}
{
    addEntity(new Light{"Licht 1", QSet<Light::Feature>{Light::Feature::Toggle},
        Light::RoomBusChannel{
            .onTrigger = 4,
            .offTrigger = 5,
            .toggleTrigger = 6,
            .stateChannel= 2
        }}
    );

    addEntity(new Light{"Licht 2", QSet<Light::Feature>{Light::Feature::Toggle},
        Light::RoomBusChannel{
            .onTrigger = 7,
            .offTrigger = 8,
            .toggleTrigger = 9,
            .stateChannel= 3
        }}
    );

    addEntity(new Light{"Licht 35", QSet<Light::Feature>{Light::Feature::Dim, Light::Feature::ColorTemperature},
        Light::RoomBusChannel{
            .onTrigger = 0,
            .offTrigger = 0,
            .toggleTrigger = 0,
            .stateChannel= 0,
            .brightnessChannel = 0x10,
            .temperatureChannel = 0x11
        }}
    );


    connect(&_server, &QWebSocketServer::newConnection, this, &Server::on_newConnection);
    _server.listen(QHostAddress::Any, _port);

    qDebug(("Server listening on port "+ QString::number(_port)).toLocal8Bit());

    if(_roomBusInterface){
        connect(_roomBusInterface->busConnection(), &RoomBusAccess::newData, this, &Server::on_newRoomBusData);
    }
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
    if(socket == nullptr) return;

    Remote *remote = new Remote(*this, _roomBusInterface, socket);

    connect(remote, &Remote::disconnected, this, &Server::on_remoteDisconnected);
    _remotes.insert(remote);

    qDebug("New connection "+QString::number(*((uint64_t*)remote), 16).toLocal8Bit());
}

void Server::on_remoteDisconnected(Remote *remote)
{
    if(!_remotes.contains(remote)) return;

    qDebug("Close connection "+QString::number(*((uint64_t*)remote), 16).toLocal8Bit());

    _remotes.remove(remote);
    remote->deleteLater();
}

RoomBusInterface *Server::roomBusInterface()
{
    return _roomBusInterface;
}

void Server::sendEvent(Remote::Event event, QJsonObject data)
{
    for(Remote *remote: _remotes){
        remote->sendEvent(event, data);
    }
}

QMap<int, Entity *> Server::entities() const
{
    return _entities;
}

Entity *Server::entity(int entityId)
{
    return _entities.value(entityId);
}

void Server::on_newRoomBusData()
{
    while(_roomBusInterface->busConnection()->rxMsgBuffer.size())
    {
        RoomBus::Message temp = _roomBusInterface->busConnection()->rxMsgBuffer.first();
        _roomBusInterface->busConnection()->rxMsgBuffer.removeFirst();

        for(Remote *remote: _remotes){
            remote->pushRoomBusMessage(temp);
        }
    }
}

void Server::addEntity(Entity *entity)
{
    static uint32_t entityId = 0;
    entity->setServer(this, entityId);
    _entities.insert(entityId, entity);
    entityId++;
}

