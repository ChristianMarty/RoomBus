#include "light.h"
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonValue>

using namespace UnfoldedCircle;

UnfoldedCircle::Light::Light(QString name, int entityId, QSet<Feature> features)
    :Entity(name, entityId, Type::lightEntity)
    ,_features{features}
{

}

void Light::commandHandler(QJsonObject data)
{
    QString commandId = data["cmd_id"].toString();

    Command command;
    if(commandId == "on") command = Command::On;
    else if(commandId == "off") command = Command::Off;
    else if(commandId == "toggle") command = Command::Toggle;

    if(command == Command::On) _state = State::On;
    if(command == Command::Off) _state = State::Off;
    if(command == Command::Toggle){
        if(_state == State::On)_state = State::Off;
        else if(_state == State::Off)_state = State::On;
        else _state = State::On;
    }

    sendEntityChange();

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


