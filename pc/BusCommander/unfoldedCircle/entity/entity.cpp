#include "entity.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include "unfoldedCircle/unfoldedCircleRemote.h"

using namespace UnfoldedCircle;

Entity::Entity(QString name, int entityId, Type type)
    :_name{name},
    _entityId{entityId},
    _type{type}
{
}
QJsonObject Entity::json(void) const
{
    return _json();
}

QJsonObject Entity::_json(QJsonArray features) const
{

    QJsonObject name {{"en", _name}};
    QJsonObject json {
                     {"entity_id", QString::number(_entityId)},
        {"entity_type", _typeToString(_type)},
        {"name", name},
        {"features", features}
    };

    return json;
}

void Entity::_sendEntityChange(QJsonObject attributes)
{
    if(_remote == nullptr) return;

    QJsonObject data{
        {"entity_type", _typeToString(_type)},
        {"entity_id", QString::number(_entityId)},
        {"attributes", attributes}
    };

    _remote->_sendEvent(Remote::Event::entityChangeEvent, data);
}

QString Entity::_typeToString(Type type) const
{
    QString typeString;

    switch(type){
        case Type::buttonEntity: typeString = "button"; break;
        case Type::climateEntity: typeString = "climate"; break;
        case Type::coverEntity: typeString = "cover"; break;
        case Type::lightEntity: typeString = "light"; break;
        case Type::mediaPlayerEntity: typeString = "media_player"; break;
        case Type::sensorEntity: typeString = "sensor"; break;
        case Type::switchEntity: typeString = "switch"; break;
        case Type::activityEntity: typeString = "activity"; break;
        case Type::macroEntity: typeString = "macro"; break;
        case Type::remoteEntity: typeString = "remote"; break;
        case Type::irEmitterEntity: typeString = "ir_emitter"; break;
    }

    return typeString;
}

void Entity::setRemote(Remote *newRemote)
{
    _remote = newRemote;
}

int Entity::entityId() const
{
    return _entityId;
}