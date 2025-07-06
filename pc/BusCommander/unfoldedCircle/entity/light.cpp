#include "light.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

#include "roomBus/roomBus.h"
#include "unfoldedCircle/remote.h"
#include "unfoldedCircle/server.h"

using namespace UnfoldedCircle;

UnfoldedCircle::Light::Light(QString name, QSet<Feature> features, RoomBusChannel roomBusChannel)
    :Entity(name, Type::lightEntity)
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

    QJsonObject parameter = data["params"].toObject();


    switch(command){
        case Command::On:
            _server->roomBusInterface()->sendTrigger(_roomBusChannel.onTrigger);

            if(_features.contains(Feature::Dim) && !parameter.isEmpty()){
                if(parameter.contains("brightness")){
                    uint32_t brightness = parameter["brightness"].toInt();
                    brightness = brightness*0xFF;
                    if(brightness>0xffff)brightness = 0xffff;
                    _server->roomBusInterface()->sendValue(_roomBusChannel.brightnessChannel, brightness);
                }
            }

            if(_features.contains(Feature::ColorTemperature)&& !parameter.isEmpty()){
                if(parameter.contains("color_temperature")){
                    uint32_t temperature = parameter["color_temperature"].toInt();
                    temperature = (uint32_t)((float)temperature*(float)2.55);
                    if(temperature>255)temperature = 255;
                    _server->roomBusInterface()->sendValue(_roomBusChannel.temperatureChannel, temperature);
                }

            }
            break;

        case Command::Off:
            _server->roomBusInterface()->sendTrigger(_roomBusChannel.offTrigger);
            break;

        case Command::Toggle:
            _server->roomBusInterface()->sendTrigger(_roomBusChannel.toggleTrigger);
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


