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

    explicit Light(QString name, int entityId, QSet<Feature> features = QSet<Feature>{});

    void commandHandler(QJsonObject data) override;

    QJsonObject json(void) const override;

    void sendEntityChange(void);

signals:

private:
    uint16_t _onTriggerChannel;
    uint16_t _offTriggerChannel;
    uint16_t _toggleTriggerChannel;
    uint16_t _stateChannel;

    QSet<Feature> _features;

    State _state;

};

}

#endif // LIGHT_H
