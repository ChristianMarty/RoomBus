#include "light.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "roomBus/roomBus.h"
#include "unfoldedCircle/unfoldedCircleRemote.h"

using namespace UnfoldedCircle;

UnfoldedCircle::Light::Light(QString name, int entityId, QSet<Feature> features, RoomBusChannel roomBusChannel)
    :Entity(name, entityId, Type::lightEntity)
    ,_roomBusChannel{roomBusChannel}
    ,_features{features}   
{
    sendEntityChange();
}

void Light::stateSystemHandler(uint16_t channel, uint8_t state)
{
    if(channel != _roomBusChannel.stateChannel) return;

    if(state == 0)_state = State::Off;
    else if(state == 1)_state = State::On;
    else _state = State::Unknown;

    sendEntityChange();
}

void Light::commandHandler(QJsonObject data)
{
    QString commandId = data["cmd_id"].toString();

    Command command;
    if(commandId == "on") command = Command::On;
    else if(commandId == "off") command = Command::Off;
    else if(commandId == "toggle") command = Command::Toggle;
    else return; // todo: error handler

    switch(command){
        case Command::On:
            _remote->roomBusInterface()->sendTrigger(_roomBusChannel.onTrigger);
            break;

        case Command::Off:
            _remote->roomBusInterface()->sendTrigger(_roomBusChannel.offTrigger);
            break;

        case Command::Toggle:
            _remote->roomBusInterface()->sendTrigger(_roomBusChannel.toggleTrigger);
            break;
    }
}

QJsonObject Light::json() const
{
    QJsonArray features;
    for(Feature feature: _features){
        switch(feature){
            case Feature::OnOff: features.append(QJsonValue{"on_off"}); break;
            case Feature::Toggle: features.append(QJsonValue{"toggle"}); break;
            case Feature::Dim: features.append(QJsonValue{"dim"}); break;
            case Feature::Color: features.append(QJsonValue{"color"}); break;
            case Feature::ColorTemperature: features.append(QJsonValue{"color_temperature"}); break;
        }
    }
    return _json(features);
}

void Light::sendEntityChange()
{
    QString state;
    switch(_state){
        case State::Unavailable: state = "UNAVAILABLE"; break;
        case State::Unknown: state = "UNKNOWN"; break;
        case State::On: state = "ON"; break;
        case State::Off: state = "OFF"; break;
    }

    QJsonObject attributes{
        {"state", state},
    };

    _sendEntityChange(attributes);
}


