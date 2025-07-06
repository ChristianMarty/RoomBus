#ifndef SWITCH_H
#define SWITCH_H

#include <QObject>
#include "entity.h"
namespace UnfoldedCircle
{


class Switch : public Entity
{
    Q_OBJECT
public:

    enum class Feature {
        OnOff,
        Toggle
    };
    enum class Attribute {
        State
    };
    enum class Command {
        On,
        Off,
        Toggle
    };
    enum class Device {
        Outlet,
        Switch
    };
    enum class Option {
        Readable
    };

    explicit Switch(QString name);
};

}
#endif // SWITCH_H
