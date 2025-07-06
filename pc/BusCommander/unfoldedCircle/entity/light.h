#ifndef LIGHT_H
#define LIGHT_H

#include <QObject>
#include "entity.h"

namespace UnfoldedCircle
{

class Light : public Entity
{
    Q_OBJECT
public:

    struct RoomBusChannel{
        uint16_t onTrigger;
        uint16_t offTrigger;
        uint16_t toggleTrigger;

        uint16_t stateChannel;

        uint16_t brightnessChannel;
        uint8_t temperatureChannel;
    };

    enum class Feature {
        OnOff,
        Toggle,
        Dim,
        Color,
        ColorTemperature
    };
    enum class State {
        Unavailable,
        Unknown,
        On,
        Off
    };
    enum class Attribute {
        State,
        Hue,
        Saturation,
        Brightness,
        ColorTemperature
    };
    enum class Command {
        On,
        Off,
        Toggle
    };

    explicit Light(QString name, QSet<Feature> features = QSet<Feature>{}, RoomBusChannel roomBusChannel = RoomBusChannel{});

    void stateSystemHandler(uint16_t channel, uint8_t state) override;

    void commandHandler(QJsonObject data) override;

    QJsonObject json(void) const override;

    void sendEntityChange(void);

signals:

private:
    RoomBusChannel _roomBusChannel;
    QSet<Feature> _features;
    State _state = State::Unknown;
};

}

#endif // LIGHT_H
