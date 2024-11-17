#ifndef APPPLUGININTERFACE_H
#define APPPLUGININTERFACE_H

#include <QWidget>
#include <QString>

#include "busMessage.h"
#include "busDevice.h"

class appPluginInterface
{
public:
    virtual ~appPluginInterface() = default;

    virtual QWidget *init(busDevice *device, QWidget *parent) = 0;
    virtual QString getName(void) = 0;
    virtual void receiveData(BusMessage msg) = 0;

};

Q_DECLARE_INTERFACE(appPluginInterface, "org.busManager.appPluginInterface")

#endif // APPPLUGININTERFACE_H
